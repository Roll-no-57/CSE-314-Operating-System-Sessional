#!/bin/bash
# A shell script with a function for error handling

# Define a function that handles errors
check_file() {
    local file=$1
    if [ -f "$file" ]; then
        echo "$file exists."
    else
        echo "$file does not exist."
        return 1  # Return an error code
    fi
}

# Call the function and check the return code
check_file "example.txt"
if [ $? -ne 0 ]; then
    echo "There was an error checking the file."
fi
