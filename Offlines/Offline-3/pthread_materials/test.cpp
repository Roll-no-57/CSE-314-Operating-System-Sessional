#include<stdio.h>
#include<fstream>
#include<iostream>
#include<vector>
#include <random>
#include<unistd.h>
#include<chrono>


#define MAX_NUMBER_OF_TYPEWRITER_MACHINE 4

int Number_of_operative;
int Unit_size;
int Document_recreation_time;
int Logbook_entry;


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
        this->station_number = id % MAX_NUMBER_OF_TYPEWRITER_MACHINE + 1;
        this->state = WAITING_FOR_RECREATION;
        this->unit_number = (id - 1) / Unit_size + 1;


    }
};

std::vector<Operative> Operatives; 

class Unit {
public:
    int ID; // ID of the unit, can be derived from the index in the Units vector
    int completed_count;
    pthread_mutex_t mutex;
    int leader_id; // ID of the leader operative in this unit

    Unit() : completed_count(0) {
        pthread_mutex_init(&mutex, NULL);
        leader_id = ID * Unit_size;
    }
};


std::vector<Unit> Units;


// This lock is for the output machine . because multiple station(TS1,TS2...) will try to access the output console . So a lock is needed here.
pthread_mutex_t mutex_output;

// An array of station lock for each of the station .because at any time several operatives can try to access on station . So for each of the station we need a separate lock.
pthread_mutex_t station_locks[MAX_NUMBER_OF_TYPEWRITER_MACHINE];
// An array of condition variable for each of the station . If a station is busy then the operatives will wait on that condition variable until the station is free.
// Lastly we will broadcast on that condition variable to wake up all the operatives waiting on that station.
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
  // Creates a random device for non-deterministic random number generation
  std::random_device rd;
  // Initializes a random number generator using the random device
  std::mt19937 generator(rd());

  // Lambda value for the Poisson distribution
  double lambda = 10000.234;

  // Defines a Poisson distribution with the given lambda
  std::poisson_distribution<int> poissonDist(lambda);

  // Generates and returns a random number based on the Poisson distribution
  return poissonDist(generator);
}



void write_output(std::string output){
    pthread_mutex_lock(&mutex_output);
    std::cout<< output;
    pthread_mutex_unlock(&mutex_output);
}




void Initialize(){

    // Initialize the Operatives instances
    for(int i=0;i<Number_of_operative;i++){
        Operatives.emplace_back(Operative(i+1));
    }

    int num_units = (Number_of_operative + Unit_size - 1) / Unit_size;
    for (int i = 0; i < num_units; ++i) {
        Units.emplace_back();
    }

    // Initialize locks for each of the station
    for(int i=0;i<MAX_NUMBER_OF_TYPEWRITER_MACHINE;i++){
        pthread_mutex_init(&station_locks[i],NULL);
        pthread_cond_init(&station_conds[i], NULL); // Add this line
    }

    // Initialize the output lock
    pthread_mutex_init(&mutex_output,NULL);

    write_output("Successfully initialized Operatives.\n");
}







void * Operative_activities(void * arg){

    Operative * operative = (Operative * )arg;

    int random_delay = get_random_number() % (Document_recreation_time * Number_of_operative)+1;

    // use random delay for each operative
    usleep(random_delay * 1000);

    int station_idx = operative->station_number - 1;

    // Now lock operative assigned station 
    pthread_mutex_lock(&station_locks[station_idx]);

    while (station_busy[station_idx]) {
        write_output("Operative " + std::to_string(operative->ID) + " is waiting at station " + std::to_string(operative->station_number) + " at time " + std::to_string(get_time()) + "\n");
        pthread_cond_wait(&station_conds[station_idx], &station_locks[station_idx]);
    }

    // Occupy the station
    station_busy[station_idx] = true;
    write_output("Operative " + std::to_string(operative->ID) + " has arrived at typewriting station " +std::to_string(operative->station_number)+ " at time " + std::to_string(get_time()) + "\n");

    pthread_mutex_unlock(&station_locks[station_idx]);


    // Assign some delay to simulate document creation process 
    usleep(Document_recreation_time * 1000);


    pthread_mutex_lock(&station_locks[station_idx]);
    write_output("Operative " + std::to_string(operative->ID) + " has completed document recreation at time " + std::to_string(get_time())  + " at station " + std::to_string(operative->station_number) + "\n");
    station_busy[station_idx] = false;
    pthread_cond_broadcast(&station_conds[station_idx]); // Notify all waiting operatives
    pthread_mutex_unlock(&station_locks[station_idx]);



    int unit_idx = operative->unit_number - 1;
    pthread_mutex_lock(&Units[unit_idx].mutex);
    Units[unit_idx].completed_count++;
    if (Units[unit_idx].completed_count == Unit_size) {
        write_output("Unit " + std::to_string(operative->unit_number) + " has completed document recreation phase at time " + std::to_string(get_time()) + "\n");
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
    std::streambuf *cinBuffer = std::cin.rdbuf(); // Save original std::cin buffer
    std::cin.rdbuf(inputFile.rdbuf()); // Redirect std::cin to input file

    std::ofstream outputFile(argv[2]);
    std::streambuf *coutBuffer = std::cout.rdbuf(); // Save original cout buffer
    std::cout.rdbuf(outputFile.rdbuf()); // Redirect cout to output file


    // Taking input from input file
    std::cin >> Number_of_operative;
    std::cin >> Unit_size;
    std::cin >> Document_recreation_time;
    std::cin >> Logbook_entry;


    pthread_t Operatives_threads[Number_of_operative];

    // Initialize operatives
    Initialize();

    // Start all the operative but they will start with a poisson distributed delay
    for(int i=0;i<Number_of_operative;i++){
        pthread_create(&Operatives_threads[i],NULL,Operative_activities,&Operatives[i]);
    }


    // wait for all the operative threads to finish
    for(int i=0;i<Number_of_operative;i++){
        pthread_join(Operatives_threads[i],NULL);
    }



    return 0;
}





