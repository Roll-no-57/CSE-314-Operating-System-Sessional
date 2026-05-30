#!/bin/bash
# A shell script to read from a file

# Specify the file to read
file="example.txt"

# Read the file line by line
while IFS= read -r line
do
    echo "Line: $line"
done < "$file"
