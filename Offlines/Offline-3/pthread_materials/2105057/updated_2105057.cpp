#include<stdio.h>
#include<fstream>
#include<iostream>
#include<vector>
#include <random>
#include<unistd.h>
#include<chrono>
#include<pthread.h>
#include<semaphore.h>

#define MAX_NUMBER_OF_TYPEWRITER_MACHINE 4

int Number_of_operative;
int Unit_size;
int Document_recreation_time;
int Logbook_entry;

// Semaphore-based lock for logbook 
sem_t read_semaphore;
sem_t write_semaphore;
sem_t read_try_semaphore;
sem_t logbook_semaphore;

int concurrent_readers = 0;
int writers_waiting = 0;
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
        this->station_number = (id % MAX_NUMBER_OF_TYPEWRITER_MACHINE) + 1;
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
    int leader_id; // ID of the leader operative in this unit
    bool document_phase_complete;
    bool logbook_entry_complete;

    Unit(int unit_id) : completed_count(0), document_phase_complete(false), logbook_entry_complete(false) {
        pthread_mutex_init(&mutex, NULL);
        ID = unit_id;
        leader_id = unit_id * Unit_size; // Leader is the highest ID in the unit
    }
};

std::vector<Unit> Units;

// Group leader semaphores for each unit 
// Used for checking if all operatives in a unit have completed their document recreation
std::vector<sem_t> group_leader_semaphores;

// Output lock
pthread_mutex_t mutex_output;

// Station locks 
pthread_mutex_t station_mutexes[MAX_NUMBER_OF_TYPEWRITER_MACHINE];

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
  double lambda = 10000.0; 
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
    sem_wait(&read_try_semaphore);      // Used to ensure that no writer is currently writing
    sem_wait(&read_semaphore);     
    concurrent_readers++;                   
    if (concurrent_readers == 1) {          // Locks the logbook when a reader first starts reading
        sem_wait(&logbook_semaphore);       
    }
    sem_post(&read_semaphore);            // Allows concurrent readers to enter  
    sem_post(&read_try_semaphore);          
}

void end_read() {
    sem_wait(&read_semaphore);              
    concurrent_readers--;                   
    if (concurrent_readers == 0) {          
        sem_post(&logbook_semaphore);       // Unlocks the logbook when all the last concurrent reader finishes --> ensures readers priority
    }
    sem_post(&read_semaphore);              
}

void start_write() {
    sem_wait(&write_semaphore);            
    writers_waiting++;                      
    if (writers_waiting == 1) {             
        sem_wait(&read_try_semaphore);      
    }
    sem_post(&write_semaphore);             
    sem_wait(&logbook_semaphore);           // a writer Locks the logbook so that no other writer or reader can access it
}

void end_write() {
    sem_post(&logbook_semaphore);           
    sem_wait(&write_semaphore);             
    writers_waiting--;                      
    if (writers_waiting == 0) {            
        sem_post(&read_try_semaphore);     
    }
    sem_post(&write_semaphore);            
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
        pthread_mutex_init(&station_mutexes[i],NULL);
    }

    // Initialize the output lock
    pthread_mutex_init(&mutex_output,NULL);

    
    sem_init(&read_semaphore, 0, 1);
    sem_init(&write_semaphore, 0, 1);
    sem_init(&read_try_semaphore, 0, 1);
    sem_init(&logbook_semaphore, 0, 1);

    // Initialize group leader semaphores 
    group_leader_semaphores.resize(num_units);
    for (int i = 0; i < group_leader_semaphores.size(); i++) {
        sem_init(&group_leader_semaphores[i], 0, 0);
    }

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
        
        sleep(3);
        
        end_read();
        
        // Different delay patterns for each staff member
        int delay_ms;
        if (staff_id == 1) {
            delay_ms = 200 + (rand() % 300);
        } else {
            delay_ms = 300 + (rand() % 400); 
        }
        
        usleep(delay_ms * 1000);
    }
    
    return NULL;
}

// Leader thread function for logbook entry 
void* leader_activities(void* arg) {
    int unit_id = *((int*)arg);
    int unit_idx = unit_id - 1;
    
    // Wait for all unit members to complete document recreation using semaphores 
    for (int i = 0; i < Unit_size; i++) {
        sem_wait(&group_leader_semaphores[unit_idx]);
    }
    
    write_output("Unit " + std::to_string(unit_id) + " has completed document recreation phase at time " + std::to_string(get_time()) + "\n");
    
    usleep((200 + rand() % 500) * 1000);
    
    // Leader proceeds to logbook entry
    start_write();
    
    write_output("Unit " + std::to_string(unit_id) + " leader began logbook entry at time " + std::to_string(get_time()) + "\n");
    
    // Simulate logbook entry time
    usleep(Logbook_entry * 1000);
    
    // Update completed operations (thread-safe within write critical section)
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

    // Lock station mutex 
    pthread_mutex_lock(&station_mutexes[station_idx]);

    write_output("Operative " + std::to_string(operative->ID) + " has arrived at typewriting station " + std::to_string(operative->station_number) + " at time " + std::to_string(get_time()) + "\n");

    // Simulate document recreation process 
    usleep(Document_recreation_time * 1000);

    write_output("Operative " + std::to_string(operative->ID) + " has completed document recreation at time " + std::to_string(get_time()) + " at station " + std::to_string(operative->station_number) + "\n");

    // Signal completion to group leader using semaphore 
    sem_post(&group_leader_semaphores[operative->unit_number - 1]);

    // Unlock station mutex - this automatically "notifies" all waiting operatives
    pthread_mutex_unlock(&station_mutexes[station_idx]);

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

    std::cin >> Number_of_operative >> Unit_size;
    std::cin >> Document_recreation_time >> Logbook_entry;

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

    // Destroy semaphores
    sem_destroy(&read_semaphore);
    sem_destroy(&write_semaphore);
    sem_destroy(&read_try_semaphore);
    sem_destroy(&logbook_semaphore);

    // Destroy group leader semaphores
    for (int i = 0; i < group_leader_semaphores.size(); i++) {
        sem_destroy(&group_leader_semaphores[i]);
    }

    // Destroy station mutexes
    for (int i = 0; i < MAX_NUMBER_OF_TYPEWRITER_MACHINE; i++) {
        pthread_mutex_destroy(&station_mutexes[i]);
    }

    // Clean up allocated memory
    delete[] unit_ids;

    // Restore original streams
    std::cin.rdbuf(cinBuffer);
    std::cout.rdbuf(coutBuffer);

    return 0;
}