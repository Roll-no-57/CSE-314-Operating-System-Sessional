#!/bin/bash
# A shell script with basic array operations

# Define an array
fruits=("Apple" "Banana" "Cherry")

# Access individual elements
echo "First fruit: ${fruits[0]}"
echo "Second fruit: ${fruits[1]}"
echo "Third fruit: ${fruits[2]}"

# Print all elements of the array
echo "All fruits: ${fruits[@]}"

# Get the length of the array
echo "Number of fruits: ${#fruits[@]}"
