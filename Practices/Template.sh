#!/bin/bash
filename="NewYork Part 03 Horror.dat"

#${var%pattern}--> remove the smallest match from the end 
#${var%%pattern}--> remove the longest match from the end 
#${var#pattern}--> remove the smallest match from the start 
#${var##pattern}--> remove the longest match from the start


# Remove file extension
name="${filename%.dat}"

# Extract city
city="${name% Part*}"

# Extract part number
part="${name#*Part }"
part="${part%% *}"

# Extract category
category="${name##* }"

echo "City: $city"
echo "Part: $part"
echo "Category: $category"
#output:
#City: NewYork
#Part: 03
#Category: Horror
