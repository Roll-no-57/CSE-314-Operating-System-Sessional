#!/bin/bash

# Loop through all text files in current directory and subdirectories
find . -type f -name "*.txt" | while read file; do
    # Make sure file exists and has content
    if [ -f "$file" ] && [ -s "$file" ]; then
        # Extract player name, country, and role from the file
        player_name=$(head -n 1 "$file")
        country=$(head -n 2 "$file" | tail -n 1)
        role=$(head -n 4 "$file" | tail -n 1)
        
        # Skip if we couldn't extract the required information
        if [ -z "$player_name" ] || [ -z "$country" ] || [ -z "$role" ]; then
            echo "Skipping $file: Missing required information"
            continue
        fi
        
        # Create the target directory structure
        target_dir="$country/$role"
        mkdir -p "$target_dir"
        
        # Move and rename the file
        cp "$file" "$target_dir/$player_name.txt"
        rm "$file"
        
        echo "Moved $file to $target_dir/$player_name.txt"
    fi

done

# Remove empty directories in a bottom-up fashion
# Find all directories, sort by depth (deepest first), and remove if empty
find . -type d | sort -r | while read dir; do
    # Skip the current directory
    if [ "$dir" = "." ]; then
        continue
    fi
    
    # Check if directory is empty and remove if it is
    if [ -z "$(ls -A "$dir")" ]; then
        rmdir "$dir"
        echo "Removed empty directory: $dir"
    fi
done

# echo "Organization complete!"