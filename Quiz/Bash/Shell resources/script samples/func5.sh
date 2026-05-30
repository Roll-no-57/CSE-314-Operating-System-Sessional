#!/bin/bash
# A shell script with a function accepting variable number of arguments

# Define a function that handles multiple arguments
concat() {
    local result=""
    for arg in "$@"
    do
        result+="$arg "
    done
    echo "Concatenated result: $result"
}

# Call the function with multiple arguments
concat "This" "is" "a" "test."
