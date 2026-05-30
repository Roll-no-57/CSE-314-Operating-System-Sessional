#!/bin/bash
# A shell script using a while loop

counter=1

while [ $counter -le 5 ]
do
    echo "Counter: $counter"
    ((counter++))  # Increment the counter
done
