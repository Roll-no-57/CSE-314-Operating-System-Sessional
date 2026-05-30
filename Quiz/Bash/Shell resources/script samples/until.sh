#!/bin/bash
# A shell script using an until loop

counter=1

until [ $counter -gt 5 ]
do
    echo "Counter: $counter"
    ((counter++))  # Increment the counter
done
