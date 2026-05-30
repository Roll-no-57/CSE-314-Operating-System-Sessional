#!/bin/bash

dir_name="blueprints"
mkdir -p "$dir_name"

declare -A inventory

while read -r data_file; do
    city_name="$(head -n 1 "$data_file" | awk '{print $1}')"
    base_name="$(basename "$data_file")"
    file_name="${base_name%.dat}"

    part_category="${file_name#*_}"                # Part_##_<category>
    part_number="${part_category%_*}"              # Part_##
    part_number="${part_number#Part_}"             # ##
    category="${file_name##*_}"                    # <category>

    new_name="${city_name}_Part_${part_number}_${category}.dat"
    cp "$data_file" "$dir_name/$new_name"

    if [[ "$part_number" =~ ^[0-9]+$ ]] && ((10#$part_number % 2 == 0)); then
        if [[ -n "${inventory[$category]}" ]]; then
            ((inventory[$category]++))
        else
            inventory[$category]=1
        fi
    fi
done < <(find heist -type f)

# Save inventory
{
    for category in "${!inventory[@]}"; do
        echo "$category: ${inventory[$category]}"
    done | sort
} > inventory_summary.txt
