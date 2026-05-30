#!/bin/bash
# A shell script to handle command-line arguments

# Check if at least one argument is passed
if [ $# -eq 0 ]; then
    echo "No arguments provided."
    exit 1
fi

echo "The script name is: $0"
echo "The first argument is: $1"
echo "The second argument is: $2"
echo "The total number of arguments passed is: $#"

# Loop through all the arguments
echo "All arguments:"
for arg in "$@"; do
    echo "$arg"
done
