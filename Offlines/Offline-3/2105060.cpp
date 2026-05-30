#include <iostream>
#include <chrono>
#include <fstream>
#include <pthread.h>
#include <semaphore.h>
#include <random>
#include <unistd.h>
#include <vector>

using namespace std;

// Color codes for terminal and file output
#define BLACK_COLOR string("\033[30m")
#define RED_COLOR string("\033[31m")
#define GREEN_COLOR string("\033[32m")
#define YELLOW_COLOR string("\033[33m")
#define BLUE_COLOR string("\033[34m")
#define MAGENTA_COLOR string("\033[35m")
#define CYAN_COLOR string("\033[36m")
#define WHITE_COLOR string("\033[37m")
#define RESET_COLOR string("\033[0m")

// Constants
#define MIN_ARRIVAL_DELAY 2     // Minimum arrival delay for operatives
#define MAX_ARRIVAL_DELAY 10    // Maximum arrival delay for operatives
#define NUMBER_OF_STATIONS 4    // Total number of stations
#define NUMBER_OF_STAFFS 2      // Total number of intelligence staffs
#define STAFF_MIN_DELAY 5       // Minimum delay between reads for staff
#define STAFF_MAX_DELAY 10      // Maximum delay between reads for staff

int N, M, x, y; /* N = number of operatives, M = unit size, x = document recreation time, y = logbook write time */

pthread_mutex_t output_lock;
pthread_mutex_t station_mutexes[NUMBER_OF_STATIONS];

vector<sem_t> group_leader_semaphores;

sem_t read_semaphore;
sem_t write_semaphore;
sem_t read_try_semaphore;
sem_t logbook_semaphore;

int concurrent_readers = 0;
int writers_waiting = 0;
int operations_completed = 0;

auto start_time = chrono::high_resolution_clock::now();

/**
 * Get the elapsed time in milliseconds since the start of the simulation.
 * @return The elapsed time in milliseconds.
 */
long long get_time() {
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
    long long elapsed_time_ms = duration.count();
    return elapsed_time_ms;
}

// uses mutex lock to write output to avoid interleaving
void write_output(const string& output) {
    pthread_mutex_lock(&output_lock);
    cout << output;
    cout.flush();
    pthread_mutex_unlock(&output_lock);
}

// Function to generate a Poisson-distributed random number
int get_random_number() {
    random_device rd;
    mt19937 generator(rd());

    // Lambda value for the Poisson distribution
    double lambda = 10000.234;
    poisson_distribution<int> poissonDist(lambda);
    return poissonDist(generator);
}

int get_random_number(int min, int max) {
    return min + (get_random_number() % (max - min + 1));
}

/**
 * Class representing an operative in the simulation.
 */
class Operative {
public:
    int id;                         // Unique ID for each operative (1-based)
    int station_id;                 // Station ID assigned to current operative
    int arrival_delay;              // Delay before operative arrives
    int group_id;                   // Group ID for the operative (1-based)
    bool is_group_leader = false;   // Flag to indicate if the operative is a group leader

    /**
     * Constructor to initialize an operative with a unique ID and random writing
     * time.
     * @param id Operative's ID. [1-based]
     * @param group_id Group ID for the operative. [1-based]
     * @param is_group_leader Flag to indicate if the operative is a group leader.
     */
    Operative(int id, int group_id, bool is_group_leader) : id(id), group_id(group_id), is_group_leader(is_group_leader) {
        station_id = (id % 4) + 1;
        arrival_delay = get_random_number(MIN_ARRIVAL_DELAY, MAX_ARRIVAL_DELAY);
    }

    Operative(int id) : id(id) {
        station_id = (id % 4) + 1;
        arrival_delay = get_random_number(MIN_ARRIVAL_DELAY, MAX_ARRIVAL_DELAY);
    }
};

vector<Operative> operatives;

void group_leader_logbook_entry(Operative *operative) {
    sem_wait(&write_semaphore);             // reserve entry section for writers - avoids race conditions
    writers_waiting++;                      // report yourself as a writer entering
    if (writers_waiting == 1) {             // checks if you're first writer
        sem_wait(&read_try_semaphore);      // if you're first, then you must lock the readers out. Prevent them from trying to enter CS
    }
    sem_post(&write_semaphore);             // release entry section

    // < CRITICAL SECTION START >
    sem_wait(&logbook_semaphore);           // reserve the resource for yourself - prevents other writers from simultaneously editing the shared resource

    write_output(RED_COLOR + "Unit " + to_string(operative->group_id) + " leader (Operative " + to_string(operative->id) + ") started writing to logbook at time " + to_string((int) (get_time() / 1000)) + "\n" + RESET_COLOR);
    sleep(y);
    operations_completed++;
    write_output(GREEN_COLOR + "Unit " + to_string(operative->group_id) + " leader (Operative " + to_string(operative->id) + ") has completed intelligence distribution at time " + to_string((int) (get_time() / 1000)) + "\n" + RESET_COLOR);

    sem_post(&logbook_semaphore);
    // < CRITICAL SECTION END >

    sem_wait(&write_semaphore);             // reserve exit section
    writers_waiting--;                      // indicate you're leaving
    if (writers_waiting == 0) {             // checks if you're the last writer
        sem_post(&read_try_semaphore);      // if you're last writer, you must unlock the readers. Allows them to try enter CS for reading
    }
    sem_post(&write_semaphore);             // release exit section
}

/**
 * Thread function for operative activities.
 * Simulates the operative's report writing and reaching the print station.
 * @param arg Pointer to an Operative object.
 */
void *operative_activities(void *arg) {
    Operative *operative = (Operative *)arg;

    // Simulate the arrival delay for the operative
    sleep(operative->arrival_delay);
    write_output(CYAN_COLOR + "Operative " + to_string(operative->id) + " has arrived at typewriting station " + to_string(operative->station_id) + " at time " + to_string((int) (get_time() / 1000)) + "\n" + RESET_COLOR);

    pthread_mutex_lock(&station_mutexes[operative->station_id - 1]);
    /* RECREATE DOCUMENT */
    sleep(x);   // Simulate time taken to recreate the document
    write_output(BLUE_COLOR + "Operative " + to_string(operative->id) + " has completed document recreation at typewriting station " + to_string(operative->station_id) + " at time " + to_string((int) (get_time() / 1000)) + "\n" + RESET_COLOR);
    sem_post(&group_leader_semaphores[operative->group_id - 1]);        // Notify group leader that this operative has completed document recreation
    pthread_mutex_unlock(&station_mutexes[operative->station_id - 1]);

    // If current operative is a group leader, wait for all group members to finish
    if (operative->is_group_leader) {
        for (int i = 0; i < M; i++) {
            sem_wait(&group_leader_semaphores[operative->group_id - 1]); // Wait for group members to finish (including itself)
        }
        write_output(YELLOW_COLOR + "Unit " + to_string(operative->group_id) + " leader (Operative " + to_string(operative->id) + ") has completed document recreation phase at time " + to_string((int) (get_time() / 1000)) + "\n" + RESET_COLOR);
        group_leader_logbook_entry(operative);
    }

    return NULL;
}

void *staff_activities(void *arg) {
    int staff_id = *(int *)arg;
    delete (int *)arg;

    while (true) {
        int delay = get_random_number(STAFF_MIN_DELAY, STAFF_MAX_DELAY); // Generate a random delay
        sleep(delay);

        sem_wait(&read_try_semaphore);          // Indicate a reader is trying to enter
        sem_wait(&read_semaphore);              // lock entry section to avoid race condition with other readers
        concurrent_readers++;                   // report yourself as a reader
        if (concurrent_readers == 1) {          // checks if you are first reader
            sem_wait(&logbook_semaphore);       // if you are first reader, lock  the resource
        }
        sem_post(&read_semaphore);              // release entry section for other readers
        sem_post(&read_try_semaphore);          // indicate you are done trying to access the resource

        // < CRITICAL SECTION START >

        write_output(MAGENTA_COLOR + "Intelligence Staff " + to_string(staff_id) + " began reviewing logbook at time " + to_string((int) (get_time() / 1000)) + ". Operations completed = " + to_string(operations_completed) + "\n" + RESET_COLOR);

        // < CRITICAL SECTION END >

        sem_wait(&read_semaphore);              // reserve exit section - avoids race condition with readers
        concurrent_readers--;                   // indicate you're leaving
        if (concurrent_readers == 0) {          // checks if you are last reader leaving
            sem_post(&logbook_semaphore);       // if last, you must release the locked resource
        }
        sem_post(&read_semaphore);              // release exit section for other readers
    }

    return NULL;
}

int main(int argc, char const *argv[]) {
    string inputFilename, outputFilename;
    if (argc != 3) {
        cout << "Usage: ./a.out <input_file> <output_file>" << endl;
        inputFilename = "input.txt";
        outputFilename = "output.txt";
        // return 0;
    }

    // File handling for input and output redirection
    ifstream inputFile(inputFilename);
    if (!inputFile.is_open()) {
        cerr << "Error opening input file: " << inputFilename << endl;
        return 1;
    }
    streambuf *cinBuffer = cin.rdbuf(); // Save original cin buffer
    cin.rdbuf(inputFile.rdbuf()); // Redirect cin to input file

    ofstream outputFile(outputFilename);
    if (!outputFile.is_open()) {
        cerr << "Error opening output file: " << outputFilename << endl;
        return 1;
    }
    streambuf *coutBuffer = cout.rdbuf(); // Save original cout buffer
    cout.rdbuf(outputFile.rdbuf()); // Redirect cout to output file


    cin >> N >> M >> x >> y;

    // Initialize mutex locks
    pthread_mutex_init(&output_lock, NULL);

    sem_init(&read_semaphore, 0, 1);
    sem_init(&write_semaphore, 0, 1);
    sem_init(&read_try_semaphore, 0, 1);
    sem_init(&logbook_semaphore, 0, 1);

    group_leader_semaphores.resize(N / M); // Group Count, C = Total Operatives (N) / Unit Size (M)
    for (int i = 0; i < group_leader_semaphores.size(); i++) {
        sem_init(&group_leader_semaphores[i], 0, 0);
    }

    // Initialize operatives vector
    for (int i = 0; i < N; i++) {
        operatives.emplace_back(i + 1, floor(i/M) + 1, ((i + 1) % M == 0));
    }

    start_time = std::chrono::high_resolution_clock::now(); // Reset start time

    pthread_t operative_threads[N];
    for (int i = 0; i < N; i++) {
        pthread_create(&operative_threads[i], NULL, operative_activities, (void *) &operatives[i]);
    }

    pthread_t intelligence_staff_threads[NUMBER_OF_STAFFS];
    for (int i = 0; i < NUMBER_OF_STAFFS; i++) {
        int *staff_id = new int(i + 1);
        pthread_create(&intelligence_staff_threads[i], NULL, staff_activities, (void *) staff_id);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(operative_threads[i], NULL);               // Wait for operative threads to finish
    }

    for (int i = 0; i < NUMBER_OF_STAFFS; i++) {
        pthread_cancel(intelligence_staff_threads[i]);              // Cancel staff threads
        pthread_join(intelligence_staff_threads[i], NULL);      // Wait for staff threads to finish
    }

    // Destroy semaphores and mutexes
    pthread_mutex_destroy(&output_lock);

    for (int i = 0; i < NUMBER_OF_STATIONS; i++) {
        pthread_mutex_destroy(&station_mutexes[i]);
    }

    for (int i = 0; i < group_leader_semaphores.size(); i++) {
        sem_destroy(&group_leader_semaphores[i]);
    }

    sem_destroy(&read_semaphore);
    sem_destroy(&write_semaphore);
    sem_destroy(&read_try_semaphore);
    sem_destroy(&logbook_semaphore);

    // Restore std::cin and cout to their original states (console)
    std::cin.rdbuf(cinBuffer);
    std::cout.rdbuf(coutBuffer);

    return 0;
}
