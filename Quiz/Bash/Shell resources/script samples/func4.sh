#!/bin/bash
# A shell script with a function using local variables

# Define a function with local variables
calculate() {
    local num1=$1
    local num2=$2
    local result=$(($num1 * $num2))
    echo "The product of $num1 and $num2 is: $result"
}

# Call the function
calculate 4 5
