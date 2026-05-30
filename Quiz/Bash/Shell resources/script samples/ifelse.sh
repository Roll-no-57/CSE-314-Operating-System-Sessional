#!/bin/bash
# A shell script using if-elif-else

echo "Enter a number:"
read num

if [ $num -gt 10 ]; then
    echo "The number is greater than 10."
elif [ $num -eq 10 ]; then
    echo "The number is equal to 10."
else
    echo "The number is less than 10."
fi
