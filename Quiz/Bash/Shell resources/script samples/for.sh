#!/bin/bash
# A shell script using a for loop

for i in {1..5}
do
    echo "Iteration $i"
done

for ((i=1; i<=5; i++))
do
    echo "Iteration $i"
done

# For loop iterating over a list
for name in Alice Bob Charlie
do
    echo "Hello, $name!"
done
