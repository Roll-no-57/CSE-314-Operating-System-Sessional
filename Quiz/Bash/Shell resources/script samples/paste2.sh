#!/bin/bash
# A shell script to combine two files with a custom delimiter

# Specify the files to combine
file1="file1.txt"
file2="file2.txt"

# Combine the files column-wise with a custom delimiter
paste -d ':' "$file1" "$file2" > combined_delimited.txt

echo "Files combined with ':' delimiter into combined_delimited.txt"
