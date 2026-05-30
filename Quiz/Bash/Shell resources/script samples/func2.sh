#!/bin/bash
# A shell script with a function that returns a value

# Define a function that returns a value
add() {
    local sum=$(($1 + $2))
    echo $sum
}

# Call the function and capture the return value
result=$(add 5 3)
echo "The sum is: $result"
