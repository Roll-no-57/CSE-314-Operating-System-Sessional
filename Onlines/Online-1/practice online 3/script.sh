#!/bin/bash

# #In the online, you have to recursively search all pdf files in a directory and its subdirectories. An integer command line argument will be provided to your script. You have to find all pdfs with page numbers higher than that and copy those into a new directory. You also have to rename the pdfs in the order of their file sizes. That is, the pdf that has the smallest file size (with at least a certain number of pages) will be renamed to 1.pdf.



# Create output directory
output_dir="filtered_pdfs"
mkdir -p "$output_dir" || { echo "Error: Could not create $output_dir"; exit 1; }

min_pages=$1

# Temporary file to store PDF paths and sizes
temp_file=$(mktemp)

# Find all PDFs and process them
find . -type f -iname "*.pdf" | while read pdf; do
    # Get page count using pdfinfo
    pages=$(pdfinfo "$pdf" 2>/dev/null | grep '^Pages:' | awk '{print $2}')

    # echo "$pages"
    # echo "$pdf"
    
    # # Check if pages is a number and greater than min_pages
    if [[ "$pages" =~ ^[0-9]+$ && "$pages" -gt "$min_pages" ]]; then

        file_size=$(pdfinfo "$pdf" | grep '^File size:' | awk '{print $3}')
        # alternatively you can do it with $(stat -c%s "$pdf")
        # echo "$file_size"
        echo "$file_size|$pdf" >> "$temp_file"
    fi
done 


count=1
sort -n "$temp_file" | while IFS="|" read -r size path;do 
    echo "$size and $path"
    cp "$path" "$output_dir/$count.pdf"
    ((count++))
    echo "$count"
done
echo "$count"



# run command ./script.sh 8