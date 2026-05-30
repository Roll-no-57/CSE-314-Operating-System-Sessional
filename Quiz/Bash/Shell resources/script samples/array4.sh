#!/bin/bash
# A shell script with associative arrays (requires Bash 4.0+)

# Declare an associative array
declare -A capitals

# Assign values to the array
capitals["France"]="Paris"
capitals["Germany"]="Berlin"
capitals["Japan"]="Tokyo"

# Access elements
echo "Capital of France: ${capitals["France"]}"
echo "Capital of Germany: ${capitals["Germany"]}"
echo "Capital of Japan: ${capitals["Japan"]}"

# Loop through the associative array
for country in "${!capitals[@]}"
do
    echo "The capital of $country is ${capitals[$country]}"
done
