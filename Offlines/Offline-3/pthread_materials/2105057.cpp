#include<stdio.h>
#include<fstream>
#include<iostream>
#include<vector>
#include <random>
#include<unistd.h>
#include<chrono>
#include<pthread.h>

#define MAX_NUMBER_OF_TYPEWRITER_MACHINE 4

int Number_of_operative;
int Unit_size;
int Document_recreation_time;
int Logbook_entry;

// Reader-writer synchronization variables for logbook
pthread_mutex_t logbook_mutex;
pthread_cond_t readers_cond, writers_cond;
int active_readers = 0;
int active_writers = 0;
int waiting_writers = 0;
// Shared variable to track completed operations
int completed_operations = 0; 
bool program_finished = false;

enum Operative_state {
    WAITING_FOR_RECREATION,
    RECREATING_DOCUMENT
};

class Operative {
    public:
    int ID;                 // ID of the operative
    int unit_number;        // Unit number of that operative
    Operative_state state;
    int station_number;

    // All properties are 1 index based
    Operative(int id){
        this->ID = id;
        this->station_number = (id - 1) % MAX_NUMBER_OF_TYPEWRITER_MACHINE + 1;
        this->state = WAITING_FOR_RECREATION;
        this->unit_number = (id - 1) / Unit_size + 1;
    }
};

std::vector<Operative> Operatives; 

class Unit {
public:
    int ID; // ID of the unit
    int completed_count;
    pthread_mutex_t mutex;
    pthread_cond_t completion_cond;
    int leader_id; // ID of the leader operative in this unit
    bool document_phase_complete;
    bool logbook_entry_complete;

    Unit(int unit_id) : completed_count(0), document_phase_complete(false), logbook_entry_complete(false) {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&completion_cond, NULL);
        ID = unit_id;
        leader_id = unit_id * Unit_size; // Leader is the highest ID in the unit
    }
};

std::vector<Unit> Units;

// Output lock
pthread_mutex_t mutex_output;

// Station locks and condition variables
pthread_mutex_t station_locks[MAX_NUMBER_OF_TYPEWRITER_MACHINE];
pthread_cond_t station_conds[MAX_NUMBER_OF_TYPEWRITER_MACHINE];

static bool station_busy[MAX_NUMBER_OF_TYPEWRITER_MACHINE] = {false};

auto start_time = std::chrono::high_resolution_clock::now();

long long get_time() {
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);
  long long elapsed_time_ms = duration.count();
  return elapsed_time_ms;
}

// Function to generate a Poisson-distributed random number
int get_random_number() {
  std::random_device rd;
  std::mt19937 generator(rd());
  double lambda = 2.0; 
  std::poisson_distribution<int> poissonDist(lambda);
  return poissonDist(generator);
}

void write_output(std::string output){
    pthread_mutex_lock(&mutex_output);
    std::cout << output << std::flush;
    pthread_mutex_unlock(&mutex_output);
}

// Reader-writer functions for logbook access
void start_read() {
    pthread_mutex_lock(&logbook_mutex);
    // writers are given priority over readers
    while (active_writers > 0 || waiting_writers > 0) {
        pthread_cond_wait(&readers_cond, &logbook_mutex);
    }
    active_readers++;
    pthread_mutex_unlock(&logbook_mutex);
}

void end_read() {
    pthread_mutex_lock(&logbook_mutex);
    active_readers--;
    if (active_readers == 0) {
        pthread_cond_signal(&writers_cond);
    }
    pthread_mutex_unlock(&logbook_mutex);
}

void start_write() {
    pthread_mutex_lock(&logbook_mutex);
    waiting_writers++;
    while (active_readers > 0 || active_writers > 0) {
        pthread_cond_wait(&writers_cond, &logbook_mutex);
    }
    waiting_writers--;
    active_writers++;
    pthread_mutex_unlock(&logbook_mutex);
}

void end_write() {
    pthread_mutex_lock(&logbook_mutex);
    active_writers--;
    pthread_cond_broadcast(&writers_cond);
    pthread_cond_broadcast(&readers_cond);
    pthread_mutex_unlock(&logbook_mutex);
}

void Initialize(){
    // Initialize the Operatives instances
    for(int i=0;i<Number_of_operative;i++){
        Operatives.emplace_back(Operative(i+1));
    }

    int num_units = (Number_of_operative + Unit_size - 1) / Unit_size;
    for (int i = 0; i < num_units; ++i) {
        Units.emplace_back(Unit(i+1));
    }

    // Initialize locks for each station
    for(int i=0;i<MAX_NUMBER_OF_TYPEWRITER_MACHINE;i++){
        pthread_mutex_init(&station_locks[i],NULL);
        pthread_cond_init(&station_conds[i], NULL);
    }

    // Initialize the output lock
    pthread_mutex_init(&mutex_output,NULL);

    // Initialize reader-writer synchronization primitives
    pthread_mutex_init(&logbook_mutex, NULL);
    pthread_cond_init(&readers_cond, NULL);
    pthread_cond_init(&writers_cond, NULL);

    write_output("Successfully initialized Operatives.\n");
}

// Intelligence Staff thread function
void* intelligence_staff_activities(void* arg) {
    int staff_id = *((int*)arg);
    
    // Start immediately with small initial delays
    int initial_delay = (staff_id == 1) ? 50 : 100;
    usleep(initial_delay * 1000);
    
    while (!program_finished) {
        start_read();
        
        // Read the logbook and report
        write_output("Intelligence Staff " + std::to_string(staff_id) + 
                    " began reviewing logbook at time " + std::to_string(get_time()) + 
                    ". Operations completed = " + std::to_string(completed_operations) + "\n");
        
        end_read();
        
        // Different delay patterns for each staff member - much shorter delays
        int delay_ms;
        if (staff_id == 1) {
            delay_ms = 200 + (rand() % 300);
        } else {
            delay_ms = 300 + (rand() % 400); 
        }
        // int delay_ms = 150;
        
        usleep(delay_ms * 1000);
    }
    
    return NULL;
}

// Leader thread function for logbook entry
void* leader_activities(void* arg) {
    int unit_id = *((int*)arg);
    int unit_idx = unit_id - 1;
    
    // Wait for all unit members to complete document recreation
    pthread_mutex_lock(&Units[unit_idx].mutex);
    while (!Units[unit_idx].document_phase_complete) {
        pthread_cond_wait(&Units[unit_idx].completion_cond, &Units[unit_idx].mutex);
    }
    pthread_mutex_unlock(&Units[unit_idx].mutex);
    
    
    usleep((200 + rand() % 500) * 1000);
    
    // Leader proceeds to logbook entry
    start_write();
    
    write_output("Unit " + std::to_string(unit_id) + " leader began logbook entry at time " + std::to_string(get_time()) + "\n");
    
    // Simulate logbook entry time
    usleep(Logbook_entry * 1000);
    
    // Update completed operations
    // This increment is thread-safe since it is inside write start_write and end_write
    // because it when the leader is writing to the logbook, no other thread can write or read
    // when it ends write operation, it will signal all waiting threads
    completed_operations++;
    
    pthread_mutex_lock(&Units[unit_idx].mutex);
    Units[unit_idx].logbook_entry_complete = true;
    pthread_mutex_unlock(&Units[unit_idx].mutex);
    
    write_output("Unit " + std::to_string(unit_id) + " has completed intelligence distribution at time " + std::to_string(get_time()) + "\n");
    
    end_write();
    
    return NULL;
}

void * Operative_activities(void * arg){
    Operative * operative = (Operative * )arg;

    // Generate random delay using Poisson distribution
    int random_delay = get_random_number() * 100; 

    // Use random delay for each operative
    usleep(random_delay * 1000);

    int station_idx = operative->station_number - 1;

    // Lock operative assigned station 
    pthread_mutex_lock(&station_locks[station_idx]);

    // Wait if station is busy
    while (station_busy[station_idx]) {
        write_output("Operative " + std::to_string(operative->ID) + " is waiting at station " + std::to_string(operative->station_number) + " at time " + std::to_string(get_time()) + "\n");
        pthread_cond_wait(&station_conds[station_idx], &station_locks[station_idx]);
    }

    // Occupy the station
    station_busy[station_idx] = true;
    write_output("Operative " + std::to_string(operative->ID) + " has arrived at typewriting station " + std::to_string(operative->station_number) + " at time " + std::to_string(get_time()) + "\n");

    pthread_mutex_unlock(&station_locks[station_idx]);

    // Simulate document recreation process 
    usleep(Document_recreation_time * 1000);

    // Complete document recreation and free the station
    pthread_mutex_lock(&station_locks[station_idx]);
    write_output("Operative " + std::to_string(operative->ID) + " has completed document recreation at time " + std::to_string(get_time()) + " at station " + std::to_string(operative->station_number) + "\n");
    station_busy[station_idx] = false;
    pthread_cond_broadcast(&station_conds[station_idx]); // Notify all waiting operatives
    pthread_mutex_unlock(&station_locks[station_idx]);

    // Update unit completion status
    int unit_idx = operative->unit_number - 1;
    pthread_mutex_lock(&Units[unit_idx].mutex);
    Units[unit_idx].completed_count++;
    if (Units[unit_idx].completed_count == Unit_size) {
        write_output("Unit " + std::to_string(operative->unit_number) + " has completed document recreation phase at time " + std::to_string(get_time()) + "\n");
        Units[unit_idx].document_phase_complete = true;
        pthread_cond_signal(&Units[unit_idx].completion_cond);
    }
    pthread_mutex_unlock(&Units[unit_idx].mutex);

    return NULL;
}

int main(int argc, char *argv[]){
    if(argc != 3){
        std::cout<<"Usage ./a.out <input file> <output file>"<<std::endl;
        return 0;
    }

    // File handling for input and output redirection
    std::ifstream inputFile(argv[1]);
    std::streambuf *cinBuffer = std::cin.rdbuf();
    std::cin.rdbuf(inputFile.rdbuf());

    std::ofstream outputFile(argv[2]);
    std::streambuf *coutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(outputFile.rdbuf());

    // Taking input from input file
    std::cin >> Number_of_operative;
    std::cin >> Unit_size;
    std::cin >> Document_recreation_time;
    std::cin >> Logbook_entry;

    pthread_t Operatives_threads[Number_of_operative];
    
    // Create arrays for intelligence staff and unit leaders
    int num_units = (Number_of_operative + Unit_size - 1) / Unit_size;
    pthread_t intelligence_staff_threads[2];
    pthread_t leader_threads[num_units];
    int staff_ids[2] = {1, 2};
    int* unit_ids = new int[num_units];
    
    for (int i = 0; i < num_units; i++) {
        unit_ids[i] = i + 1;
    }

    // Initialize operatives and synchronization primitives
    Initialize();

    // Start intelligence staff threads
    write_output("Starting Intelligence Staff monitoring...\n");
    for (int i = 0; i < 2; i++) {
        if (pthread_create(&intelligence_staff_threads[i], NULL, intelligence_staff_activities, &staff_ids[i]) != 0) {
            write_output("Error creating intelligence staff thread " + std::to_string(i+1) + "\n");
        }
    }

    // Start leader threads
    for (int i = 0; i < num_units; i++) {
        pthread_create(&leader_threads[i], NULL, leader_activities, &unit_ids[i]);
    }

    // Start all operative threads
    for(int i=0;i<Number_of_operative;i++){
        pthread_create(&Operatives_threads[i],NULL,Operative_activities,&Operatives[i]);
    }

    // Wait for all operative threads to finish
    for(int i=0;i<Number_of_operative;i++){
        pthread_join(Operatives_threads[i],NULL);
    }

    // Wait for leader threads to finish
    for (int i = 0; i < num_units; i++) {
        pthread_join(leader_threads[i], NULL);
    }


    usleep(2000000); 
    
    // Signal program completion
    program_finished = true;
    
    
    usleep(1000000); 

    // Cancel intelligence staff threads 
    for (int i = 0; i < 2; i++) {
        pthread_cancel(intelligence_staff_threads[i]);
    }

    // Clean up allocated memory
    delete[] unit_ids;

    // Restore original streams
    std::cin.rdbuf(cinBuffer);
    std::cout.rdbuf(coutBuffer);

    return 0;
}