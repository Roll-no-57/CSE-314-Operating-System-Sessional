#!/bin/bash
# A shell script to read an array from a file

# Create a file with array elements
echo -e "Apple\nBanana\nCherry" > fruits.txt

# Read the file into an array
mapfile -t fruits < fruits.txt

# Print the array elements
echo "Fruits read from file:"
for fruit in "${fruits[@]}"
do
    echo "$fruit"
done
