#!/bin/bash

# media_dir=$1
# # catelog_name=$2

# declare -A catalog

# find "$media_dir" -type f | while read -r media_file;do
#     # echo "$media_file"
#     base_name="$(basename "$media_file")"

#     mp3_file="${base_name%%*.mp3}"
#     mp4_file="${base_name%%*.mp4}"
#     mmkv_file="${base_name%%*.mkv}"
#     flac_file="${base_name%%*.flac}"

#     actual_name="${base_name%%.*}"

#     # echo "$actual_name"

#     artist=""
#     title=""

#     # pattern match artist-title
#     if [[ "$actual_name" =~ ^[a-zA-Z0-9_\ ]+"-"[a-zA-Z0-9_\ ]+$ ]];then
#         artist="${actual_name%%-*}"
#         title="${actual_name##*-}"
#         echo "$artist"
#         echo "$title"
#     # title(year)-artist
#     elif [[ "$actual_name" =~ ^[a-zA-Z0-9_\ ]+"("[0-9]{4}")"[\ ]*"-"[a-zA-Z0-9_\ ]+$ ]];then
#         title="${actual_name%%\(*}"
#         artist="${actual_name##*-}"
#         echo "$artist"
#         echo "$title"
#     else
#         title="Unknown"
#         artist="Unknown"
#     fi 

#     artist="$(echo "$artist"|xargs)"
#     title="$(echo "$title"|xargs)"


#     if [[ -n "${catalog[$artist]}" ]];then
#         catalog["$artist"]="${catalog[$artist]} $title"
#     else
#         catalog["$artist"]="$title"
#     fi

    

# done


# :> cata.txt


# for artist in $(echo "${!catalog[@]}"| tr ' ' '\n' | sort);do
#     echo "$artist" >> cata.txt

#     echo ${catalog["$artist"]}  | tr ' ' '\n' |sort  >> cata.txt


# done



#!/bin/bash

media_dir=$1
declare -A catalog

while IFS= read -r media_file; do
    base_name="$(basename "$media_file")"
    actual_name="${base_name%.*}"  # strip extension

    artist=""
    title=""

    # Pattern: Artist - Title
    if [[ "$actual_name" =~ ^(.*[^[:space:]])[[:space:]]*-[[:space:]]*(.+)$ ]]; then
        artist="${BASH_REMATCH[1]}"
        title="${BASH_REMATCH[2]}"
    # Pattern: Title (Year) - Artist
    elif [[ "$actual_name" =~ ^(.*)[[:space:]]*\([0-9]{4}\)[[:space:]]*-[[:space:]]*(.*)$ ]]; then
        title="${BASH_REMATCH[1]}"
        artist="${BASH_REMATCH[2]}"
    else
        artist="Unknown"
        title="Unknown"
    fi

    # # Trim whitespace
    # artist="$(echo "$artist" | xargs)"
    # title="$(echo "$title" | xargs)"

    if [[ -n "${catalog[$artist]}" ]]; then
        catalog["$artist"]="${catalog[$artist]}|||$title"
    else
        catalog["$artist"]="$title"
    fi

done < <(find "$media_dir" -type f)

# Output file
:> cata.txt

# Sort artists
mapfile -t sorted_artists < <(printf "%s\n" "${!catalog[@]}" | sort)

for artist in "${sorted_artists[@]}"; do
    echo "$artist" >> cata.txt
    titles="${catalog[$artist]}"
    # Split on custom delimiter to preserve spaces in titles
    IFS='|||' read -ra title_array <<< "$titles"
    mapfile -t sorted_titles < <(printf "%s\n" "${title_array[@]}" | sort)
    for title in "${sorted_titles[@]}"; do
        echo "    $title" >> cata.txt
    done
done
