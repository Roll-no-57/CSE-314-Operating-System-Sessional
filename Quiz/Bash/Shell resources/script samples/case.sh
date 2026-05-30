#!/bin/bash
# A shell script using case

echo "Enter a letter (a, b, or c):"
read letter

case $letter in
    "a")
        echo "You entered 'a'."
        ;;
    "b")
        echo "You entered 'b'."
        ;;
    "c")
        echo "You entered 'c'."
        ;;
    *)
        echo "Invalid input. Please enter a, b, or c."
        ;;
esac
