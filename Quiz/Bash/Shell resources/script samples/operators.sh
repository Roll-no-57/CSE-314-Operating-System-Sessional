#!/bin/bash
# A shell script for using operators

num1=10
num2=20

# Comparison Operators
if [ $num1 -gt $num2 ]; then
    echo "$num1 is greater than $num2"
else
    echo "$num1 is less than or equal to $num2"
fi

# Logical Operators
if [ $num1 -lt 15 ] && [ $num2 -lt 25 ]; then
    echo "Both conditions are true"
else
    echo "One or both conditions are false"
fi
