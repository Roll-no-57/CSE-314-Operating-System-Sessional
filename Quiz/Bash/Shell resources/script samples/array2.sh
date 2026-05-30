#!/bin/bash
# A shell script that loops through an array

# Define an array
colors=("Red" "Green" "Blue" "Yellow")

# Loop through the array
for color in "${colors[@]}"
do
    echo "Color: $color"
done
