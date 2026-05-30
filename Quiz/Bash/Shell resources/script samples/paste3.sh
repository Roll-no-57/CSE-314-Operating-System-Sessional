#!/bin/bash
# A shell script to create a table from multiple files

# Specify the files containing columns
col1="col1.txt"
col2="col2.txt"
col3="col3.txt"

# Combine the columns into a table
paste "$col1" "$col2" "$col3" > table.txt

echo "Columns combined into a table in table.txt"
