#!/bin/bash
# A shell script to split a string based on a custom delimiter

# Define a string with a custom delimiter
string="apple;banana;cherry"

# Set IFS to the custom delimiter
IFS=';'

# Read the string into an array
read -ra fruits <<< "$string"

# Print each element of the array
for fruit in "${fruits[@]}"
do
    echo "$fruit"
done
