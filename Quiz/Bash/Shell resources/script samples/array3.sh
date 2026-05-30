#!/bin/bash
# A shell script demonstrating array initialization and manipulation

# Initialize an array
numbers=(1 2 3 4 5)

# Add an element to the array
numbers+=("6")

# Remove an element (by index)
unset numbers[2]

# Print the updated array
echo "Updated numbers: ${numbers[@]}"

# Print array indices
echo "Array indices: ${!numbers[@]}"
