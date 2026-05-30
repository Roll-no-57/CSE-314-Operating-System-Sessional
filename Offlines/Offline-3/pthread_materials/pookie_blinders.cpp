/*
 * CSE 314: Operating System Sessional - Assignment 3: IPC
 * The Shadows of Small Heath - Peaky Blinders Synchronization
 * 
 * This program simulates the Peaky Blinders intelligence operation with:
 * Task 1: Station assignment protocol with proper synchronization
 * Task 2: Reader-writer problem for logbook access
 * 
 * Compilation: g++ -pthread -o peaky_blinders main.cpp
 * Usage: ./peaky_blinders input.txt output.txt
 */

#include <iostream>
#include <fstream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <random>
#include <chrono>
#include <vector>
#include <algorithm>

using namespace std;

// Global variables
int N, M, x, y; // N=operatives, M=unit size, x=doc time, y=logbook time
int completed_operations = 0; // Shared counter for completed operations
auto start_time = chrono::high_resolution_clock::now();

// Synchronization primitives for Task 1 (Station Assignment)
sem_t stations[4]; // Semaphores for 4 typewriting stations
pthread_mutex_t station_mutex[4]; // Mutex for each station
pthread_cond_t station_cond[4]; // Condition variables for waiting operatives

// Synchronization primitives for Task 2 (Reader-Writer Problem)
pthread_mutex_t logbook_mutex; // Mutex for logbook access
pthread_mutex_t reader_count_mutex; // Mutex for reader count
pthread_cond_t writers_cond; // Condition variable for writers
int active_readers = 0; // Number of active readers
bool writer_active = false; // Flag to indicate if writer is active

// Output synchronization
pthread_mutex_t output_mutex;
ofstream output_file;

// Function to get current time in milliseconds since start
long long get_current_time() {
    auto current = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(current - start_time);
    return duration.count();
}

// Function to generate Poisson distributed random number
int get_poisson_random(double lambda = 10.0) {
    static random_device rd;
    static mt19937 gen(rd());
    poisson_distribution<int> dist(lambda);
    return dist(gen) + 1;
}

// Thread-safe output function
void safe_print(const string& message) {
    pthread_mutex_lock(&output_mutex);
    output_file << message << endl;
    output_file.flush();
    pthread_mutex_unlock(&output_mutex);
}

// Operative structure
struct Operative {
    int id;
    int group_id;
    int station_id;
    bool is_leader;
    
    Operative(int _id) : id(_id) {
        group_id = (id - 1) / M + 1;
        station_id = (id - 1) % 4;
        is_leader = (id % M == 0) || (id == N && N % M != 0);
    }
};

// Group structure to track completion
struct Group {
    int id;
    int completed_members;
    pthread_mutex_t mutex;
    
    Group(int _id) : id(_id), completed_members(0) {
        pthread_mutex_init(&mutex, NULL);
    }
};

vector<Group> groups;

// Task 1: Document Recreation Phase
void* document_recreation_phase(void* arg) {
    Operative* op = (Operative*)arg;
    
    // Random arrival delay
    int arrival_delay = get_poisson_random(3.0);
    usleep(arrival_delay * 100000); // Convert to microseconds
    
    safe_print("Operative " + to_string(op->id) + " has arrived at typewriting station at time " + to_string(get_current_time()));
    
    // Wait for station availability (Task 1 implementation)
    pthread_mutex_lock(&station_mutex[op->station_id]);
    while (sem_trywait(&stations[op->station_id]) != 0) {
        // Station is occupied, wait for notification
        pthread_cond_wait(&station_cond[op->station_id], &station_mutex[op->station_id]);
    }
    pthread_mutex_unlock(&station_mutex[op->station_id]);
    
    // Document recreation work
    usleep(x * 100000); // x time units
    
    safe_print("Operative " + to_string(op->id) + " has completed document recreation at time " + to_string(get_current_time()));
    
    // Release station and notify waiting operatives
    sem_post(&stations[op->station_id]);
    pthread_mutex_lock(&station_mutex[op->station_id]);
    pthread_cond_broadcast(&station_cond[op->station_id]); // Notify all waiting operatives
    pthread_mutex_unlock(&station_mutex[op->station_id]);
    
    // Update group completion status
    pthread_mutex_lock(&groups[op->group_id - 1].mutex);
    groups[op->group_id - 1].completed_members++;
    if (groups[op->group_id - 1].completed_members == M || 
        (op->group_id == (N - 1) / M + 1 && groups[op->group_id - 1].completed_members == N % M)) {
        safe_print("Unit " + to_string(op->group_id) + " has completed document recreation phase at time " + to_string(get_current_time()));
    }
    pthread_mutex_unlock(&groups[op->group_id - 1].mutex);
    
    return NULL;
}

// Task 2: Logbook Entry Phase (only for leaders)
void* logbook_entry_phase(void* arg) {
    Operative* op = (Operative*)arg;
    
    if (!op->is_leader) return NULL;
    
    // Wait for all group members to complete document recreation
    bool group_ready = false;
    while (!group_ready) {
        pthread_mutex_lock(&groups[op->group_id - 1].mutex);
        int expected_members = (op->group_id == (N - 1) / M + 1 && N % M != 0) ? N % M : M;
        group_ready = (groups[op->group_id - 1].completed_members == expected_members);
        pthread_mutex_unlock(&groups[op->group_id - 1].mutex);
        
        if (!group_ready) {
            usleep(100000); // Wait 100ms before checking again
        }
    }
    
    // Writer access to logbook (Task 2 implementation)
    pthread_mutex_lock(&logbook_mutex);
    
    // Wait until no readers are active
    pthread_mutex_lock(&reader_count_mutex);
    while (active_readers > 0 || writer_active) {
        pthread_cond_wait(&writers_cond, &reader_count_mutex);
    }
    writer_active = true;
    pthread_mutex_unlock(&reader_count_mutex);
    
    // Critical section: Write to logbook
    usleep(y * 100000); // y time units for logbook entry
    completed_operations++;
    
    safe_print("Unit " + to_string(op->group_id) + " has completed intelligence distribution at time " + to_string(get_current_time()));
    
    // Release writer access
    pthread_mutex_lock(&reader_count_mutex);
    writer_active = false;
    pthread_cond_broadcast(&writers_cond);
    pthread_mutex_unlock(&reader_count_mutex);
    
    pthread_mutex_unlock(&logbook_mutex);
    
    return NULL;
}

// Intelligence Staff Reader threads
void* intelligence_staff_reader(void* arg) {
    int staff_id = *(int*)arg;
    
    while (true) {
        // Random reading intervals
        int read_interval = get_poisson_random(5.0);
        usleep(read_interval * 100000);
        
        // Reader access to logbook
        pthread_mutex_lock(&reader_count_mutex);
        
        // Wait if writer is active
        while (writer_active) {
            pthread_cond_wait(&writers_cond, &reader_count_mutex);
        }
        
        active_readers++;
        pthread_mutex_unlock(&reader_count_mutex);
        
        // Critical section: Read logbook
        safe_print("Intelligence Staff " + to_string(staff_id) + " began reviewing logbook at time " + 
                  to_string(get_current_time()) + ". Operations completed = " + to_string(completed_operations));
        
        usleep(200000); // Reading time
        
        // Release reader access
        pthread_mutex_lock(&reader_count_mutex);
        active_readers--;
        if (active_readers == 0) {
            pthread_cond_signal(&writers_cond); // Wake up waiting writers
        }
        pthread_mutex_unlock(&reader_count_mutex);
        
        // Exit condition: all operations completed
        if (completed_operations >= (N + M - 1) / M) {
            break;
        }
    }
    
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << endl;
        return 1;
    }
    
    // Read input
    ifstream input_file(argv[1]);
    if (!input_file.is_open()) {
        cerr << "Error opening input file" << endl;
        return 1;
    }
    
    input_file >> N >> M >> x >> y;
    input_file.close();
    
    // Open output file
    output_file.open(argv[2]);
    if (!output_file.is_open()) {
        cerr << "Error opening output file" << endl;
        return 1;
    }
    
    // Initialize synchronization primitives
    for (int i = 0; i < 4; i++) {
        sem_init(&stations[i], 0, 1); // Each station can serve 1 operative
        pthread_mutex_init(&station_mutex[i], NULL);
        pthread_cond_init(&station_cond[i], NULL);
    }
    
    pthread_mutex_init(&logbook_mutex, NULL);
    pthread_mutex_init(&reader_count_mutex, NULL);
    pthread_mutex_init(&output_mutex, NULL);
    pthread_cond_init(&writers_cond, NULL);
    
    // Initialize groups
    int num_groups = (N + M - 1) / M;
    for (int i = 0; i < num_groups; i++) {
        groups.emplace_back(i + 1);
    }
    
    // Create operative objects
    vector<Operative> operatives;
    for (int i = 1; i <= N; i++) {
        operatives.emplace_back(i);
    }
    
    // Start intelligence staff readers
    pthread_t staff_threads[2];
    int staff_ids[2] = {1, 2};
    for (int i = 0; i < 2; i++) {
        pthread_create(&staff_threads[i], NULL, intelligence_staff_reader, &staff_ids[i]);
    }
    
    // Create operative threads for document recreation
    vector<pthread_t> doc_threads(N);
    for (int i = 0; i < N; i++) {
        pthread_create(&doc_threads[i], NULL, document_recreation_phase, &operatives[i]);
    }
    
    // Create leader threads for logbook entry
    vector<pthread_t> logbook_threads;
    for (int i = 0; i < N; i++) {
        if (operatives[i].is_leader) {
            pthread_t thread;
            pthread_create(&thread, NULL, logbook_entry_phase, &operatives[i]);
            logbook_threads.push_back(thread);
        }
    }
    
    // Wait for all operative threads to complete
    for (int i = 0; i < N; i++) {
        pthread_join(doc_threads[i], NULL);
    }
    
    for (pthread_t& thread : logbook_threads) {
        pthread_join(thread, NULL);
    }
    
    // Wait a bit more for final staff readings
    usleep(2000000);
    
    // Cancel staff threads (they run indefinitely)
    for (int i = 0; i < 2; i++) {
        pthread_cancel(staff_threads[i]);
    }
    
    // Cleanup
    for (int i = 0; i < 4; i++) {
        sem_destroy(&stations[i]);
        pthread_mutex_destroy(&station_mutex[i]);
        pthread_cond_destroy(&station_cond[i]);
    }
    
    pthread_mutex_destroy(&logbook_mutex);
    pthread_mutex_destroy(&reader_count_mutex);
    pthread_mutex_destroy(&output_mutex);
    pthread_cond_destroy(&writers_cond);
    
    output_file.close();
    
    return 0;
}