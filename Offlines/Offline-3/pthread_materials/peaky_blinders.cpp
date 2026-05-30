#include <iostream>
#include <fstream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <random>
#include <cmath>

using namespace std;

// Global variables
int N, M, x, y; // N = operatives, M = unit size, x = document recreation time, y = logbook entry time
int c; // number of units (N/M)
int completed_operations = 0; // shared variable for logbook

// Semaphores and mutexes
sem_t typewriting_stations[4]; // 4 typewriting stations (TS1, TS2, TS3, TS4)
pthread_mutex_t station_mutex[4]; // mutex for each station's waiting queue
vector<pthread_cond_t> station_conditions[4]; // condition variables for waiting operatives

// Reader-Writer problem for logbook
sem_t resource_access; // controls access to the logbook
sem_t read_count_access; // controls access to read_count
int read_count = 0; // number of readers currently reading

// Unit synchronization
vector<sem_t> unit_completion; // semaphore for each unit to wait for all members
vector<int> unit_member_count; // count of completed members in each unit
vector<pthread_mutex_t> unit_mutex; // mutex for each unit's member count

// Timing
auto start_time = chrono::high_resolution_clock::now();
pthread_mutex_t output_mutex; // for synchronized output

// Function to get current time in milliseconds
long long get_current_time() {
    auto current_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(current_time - start_time);
    return duration.count();
}

// Function to generate Poisson distributed random number
int generate_poisson(double lambda) {
    random_device rd;
    mt19937 gen(rd());
    poisson_distribution<int> d(lambda);
    return d(gen);
}

// Thread-safe output function
void print_output(const string& message) {
    pthread_mutex_lock(&output_mutex);
    cout << message << endl;
    pthread_mutex_unlock(&output_mutex);
}

// Operative structure
struct Operative {
    int id;
    int unit_id;
    int station_id;
    bool is_leader;
};

// Function for operatives to recreate documents
void* operative_function(void* arg) {
    Operative* op = (Operative*)arg;
    
    // Random delay before starting (Poisson distribution)
    int delay = generate_poisson(5.0); // lambda = 5 for reasonable delays
    usleep(delay * 1000); // convert to milliseconds
    
    // Arrive at typewriting station
    print_output("Operative " + to_string(op->id) + " has arrived at typewriting station at time " + to_string(get_current_time()));
    
    // Wait for typewriting station (station assignment: operative ID mod 4 + 1)
    // But we use 0-based indexing, so it's (operative ID - 1) mod 4
    int station_idx = (op->id - 1) % 4;
    
    // Wait for the station to be available
    sem_wait(&typewriting_stations[station_idx]);
    
    // Document recreation phase
    print_output("Operative " + to_string(op->id) + " started document recreation at time " + to_string(get_current_time()));
    usleep(x * 1000); // x time units for document recreation
    print_output("Operative " + to_string(op->id) + " has completed document recreation at time " + to_string(get_current_time()));
    
    // Signal station availability
    sem_post(&typewriting_stations[station_idx]);
    
    // Update unit completion count
    pthread_mutex_lock(&unit_mutex[op->unit_id]);
    unit_member_count[op->unit_id]++;
    
    // Check if all unit members have completed document recreation
    if (unit_member_count[op->unit_id] == M) {
        print_output("Unit " + to_string(op->unit_id + 1) + " has completed document recreation phase at time " + to_string(get_current_time()));
        
        // If this operative is the leader, proceed to logbook entry
        if (op->is_leader) {
            pthread_mutex_unlock(&unit_mutex[op->unit_id]);
            
            // Wait for write access to logbook (writer priority)
            sem_wait(&resource_access);
            
            // Logbook entry phase
            print_output("Unit " + to_string(op->unit_id + 1) + " leader (Operative " + to_string(op->id) + ") started logbook entry at time " + to_string(get_current_time()));
            usleep(y * 1000); // y time units for logbook entry
            
            // Update completed operations count
            completed_operations++;
            print_output("Unit " + to_string(op->unit_id + 1) + " has completed intelligence distribution at time " + to_string(get_current_time()));
            
            // Release write access
            sem_post(&resource_access);
        } else {
            pthread_mutex_unlock(&unit_mutex[op->unit_id]);
        }
    } else {
        pthread_mutex_unlock(&unit_mutex[op->unit_id]);
    }
    
    return NULL;
}

// Intelligence staff reader function
void* intelligence_staff_function(void* arg) {
    int staff_id = *(int*)arg;
    
    while (true) {
        // Random delay between readings (Poisson distribution)
        int delay = generate_poisson(15.0); // Different lambda for staff
        usleep(delay * 1000);
        
        // Reader wants to read
        sem_wait(&read_count_access);
        read_count++;
        if (read_count == 1) {
            sem_wait(&resource_access); // First reader blocks writers
        }
        sem_post(&read_count_access);
        
        // Reading the logbook
        print_output("Intelligence Staff " + to_string(staff_id) + " began reviewing logbook at time " + to_string(get_current_time()) + ". Operations completed = " + to_string(completed_operations));
        
        // Simulate reading time
        usleep(2000); // 2 seconds reading time
        
        // Reader finished reading
        sem_wait(&read_count_access);
        read_count--;
        if (read_count == 0) {
            sem_post(&resource_access); // Last reader allows writers
        }
        sem_post(&read_count_access);
        
        // Break condition (optional - for finite execution)
        if (completed_operations >= c) {
            break;
        }
    }
    
    return NULL;
}

int main(int argc, char* argv[]) {
    // File I/O setup
    if (argc != 3) {
        cout << "Usage: ./program <input_file> <output_file>" << endl;
        return 1;
    }
    
    ifstream input_file(argv[1]);
    if (!input_file.is_open()) {
        cout << "Error opening input file" << endl;
        return 1;
    }
    
    ofstream output_file(argv[2]);
    if (!output_file.is_open()) {
        cout << "Error opening output file" << endl;
        return 1;
    }
    
    // Redirect cout to output file
    streambuf* orig_cout = cout.rdbuf();
    cout.rdbuf(output_file.rdbuf());
    
    // Read input
    input_file >> N >> M >> x >> y;
    input_file.close();
    
    c = N / M; // number of units
    
    // Initialize synchronization primitives
    for (int i = 0; i < 4; i++) {
        sem_init(&typewriting_stations[i], 0, 1); // Each station can handle 1 operative
        pthread_mutex_init(&station_mutex[i], NULL);
    }
    
    // Initialize reader-writer semaphores
    sem_init(&resource_access, 0, 1);
    sem_init(&read_count_access, 0, 1);
    
    // Initialize unit synchronization
    unit_completion.resize(c);
    unit_member_count.resize(c, 0);
    unit_mutex.resize(c);
    
    for (int i = 0; i < c; i++) {
        sem_init(&unit_completion[i], 0, 0);
        pthread_mutex_init(&unit_mutex[i], NULL);
    }
    
    pthread_mutex_init(&output_mutex, NULL);
    
    // Create operatives
    vector<Operative> operatives(N);
    vector<pthread_t> operative_threads(N);
    
    for (int i = 0; i < N; i++) {
        operatives[i].id = i + 1;
        operatives[i].unit_id = i / M;
        operatives[i].station_id = i % 4;
        operatives[i].is_leader = ((i + 1) % M == 0); // Highest ID in unit is leader
    }
    
    // Create intelligence staff threads
    pthread_t staff_threads[2];
    int staff_ids[2] = {1, 2};
    
    start_time = chrono::high_resolution_clock::now();
    
    // Start intelligence staff threads
    for (int i = 0; i < 2; i++) {
        pthread_create(&staff_threads[i], NULL, intelligence_staff_function, &staff_ids[i]);
    }
    
    // Create operative threads with randomized arrival
    vector<bool> created(N, false);
    int remaining = N;
    
    while (remaining > 0) {
        int random_idx = generate_poisson(N/4.0) % N;
        if (!created[random_idx]) {
            pthread_create(&operative_threads[random_idx], NULL, operative_function, &operatives[random_idx]);
            created[random_idx] = true;
            remaining--;
            usleep(100000); // Small delay between thread creation (100ms)
        }
    }
    
    // Wait for all operative threads to complete
    for (int i = 0; i < N; i++) {
        pthread_join(operative_threads[i], NULL);
    }
    
    // Clean up intelligence staff threads (they run indefinitely, so we cancel them)
    for (int i = 0; i < 2; i++) {
        pthread_cancel(staff_threads[i]);
    }
    
    // Clean up
    for (int i = 0; i < 4; i++) {
        sem_destroy(&typewriting_stations[i]);
        pthread_mutex_destroy(&station_mutex[i]);
    }
    
    sem_destroy(&resource_access);
    sem_destroy(&read_count_access);
    
    for (int i = 0; i < c; i++) {
        sem_destroy(&unit_completion[i]);
        pthread_mutex_destroy(&unit_mutex[i]);
    }
    
    pthread_mutex_destroy(&output_mutex);
    
    // Restore cout
    cout.rdbuf(orig_cout);
    output_file.close();
    
    return 0;
}