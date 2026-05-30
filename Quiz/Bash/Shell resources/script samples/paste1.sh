#!/bin/bash
# A shell script to combine two files side by side

# Specify the files to combine
file1="file1.txt"
file2="file2.txt"

# Combine the files column-wise
paste "$file1" "$file2" > combined.txt

echo "Files combined into combined.txt"
