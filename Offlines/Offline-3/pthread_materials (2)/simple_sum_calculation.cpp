/*
  This program calculates the sum of all numbers from 1 to N using multiple
  threads. Each thread is responsible for summing a specific range of numbers,
  and the final sum is obtained by combining the partial sums from each thread.

  How it works:
    - N: The upper limit of the range (1 to N).
    - M: The number of threads to use for the calculation.
    - Each thread calculates the sum of a portion of the range, storing its
  result in a variable.
    - The main thread waits for all threads to finish, then combines their
  results to get the total sum.

  Compilation:
    g++ -pthread simple_sum_calculation.cpp -o a.out

  Usage:
    ./a.out

  Prepared by: Nafis Tahmid (1905002), Date: 10 November 2024
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Class to hold the data range and partial sum for each thread
class ThreadData {
public:
  long start;    // Start of the range for the thread
  long end;      // End of the range for the thread
  long long sum; // Sum computed by the thread in its range
};

// Function that each thread runs to compute the sum in its range
void *computeSum(void *arg) {
  ThreadData *data = (ThreadData *)arg;
  data->sum = 0; // Initialize the thread's sum to zero
  for (int i = data->start; i <= data->end; i++) {
    data->sum += i; // Add each number in the range to the thread's sum
  }
  return NULL;
}

int main(void) {
  long N = 2441139;  // The upper limit for summing numbers :-)
  long M = 10;       // Number of threads
  long long sum = 0; // Variable to store the final sum

  // Array of threads
  pthread_t threads[M];
  // Array to store data (range and partial sum) for each thread
  ThreadData data[M];

  // Create M threads to compute parts of the sum
  for (int i = 0; i < M; i++) {
    // Define the range for each thread based on thread index
    data[i].start = i * N / M + 1;
    data[i].end = (i + 1) * N / M;
    // Create the thread, passing the thread's data as an argument
    pthread_create(&threads[i], NULL, computeSum, (void *)&data[i]);
  }

  // Join M threads to ensure main thread waits for all threads to finish
  // and accumulate the partial sums, otherwise the main thread may finish
  // before the threads and the sum will be incorrect
  for (int i = 0; i < M; i++) {
    // if the following line is commented, the program may output diffferent sum
    // in each run with same N
    pthread_join(threads[i], NULL); // Wait for each thread to finish
    sum += data[i].sum;             // Accumulate the sum from each thread
  }

  // Print the final computed sum
  printf("The sum of numbers between 1 to %ld is %lld\n", N, sum);

  return 0;
}

/*
  Prepared by: Nafis Tahmid (1905002), Date: 10 November 2024
*/