#!/bin/bash
# A shell script with a function that uses default arguments

# Define a function with default arguments
greet() {
    local name=${1:-"Guest"}
    echo "Hello, $name!"
}

# Call the function with and without an argument
greet "Alice"
greet
