#!/bin/bash
# A shell script to read a CSV file with IFS

# Specify the CSV file
file="data.csv"

# Open the file and read it line by line
while IFS=',' read -r name age city
do
    echo "Name: $name, Age: $age, City: $city"
done < "$file"
