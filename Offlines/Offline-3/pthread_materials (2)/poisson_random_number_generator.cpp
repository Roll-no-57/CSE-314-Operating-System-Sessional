/*
  This program generates random numbers based on the Poisson distribution.

  A Poisson distribution is a probability distribution often used to model the
  number of times an event occurs within a specified interval, like the number
  of arrivals at a service point in a given time frame. It has a parameter
  lambda (λ) which represents the average rate of occurrences in the interval.

  In this code:
  - We create a Poisson distribution with lambda = 10000.234.
  - Using a random number generator, we produce a random value that fits
    this Poisson distribution.
  - The get_random_number function returns a new random number each time it
    is called, based on the specified Poisson distribution.
  - We then call this function 10 times in a loop to display 10 random numbers.

  This example demonstrates how to use the C++ Standard Library to generate
  Poisson-distributed random numbers.

  Compilation:
    g++ poisson_random_number_generator.cpp -o a.out

  Usage:
    ./a.out

  Prepared by: Nafis Tahmid (1905002), Date: 10 November 2024
*/

#include <iostream>
#include <random>

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

int main() {
  // Loop to print 10 Poisson-distributed random numbers
  for (int i = 0; i < 10; i++) {
    std::cout << get_random_number()
              << std::endl; // Output each generated number
  }
  return 0;
}

/*
  Prepared by: Nafis Tahmid (1905002), Date: 10 November 2024
*/