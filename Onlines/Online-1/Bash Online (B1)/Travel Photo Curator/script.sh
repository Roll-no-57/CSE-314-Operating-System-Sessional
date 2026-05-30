#!/bin/bash

dir_name="tourPhotos"
subdir_morn="Morning"
subdir_even="evening"
subdir_noon="Afternoon"

mkdir -p "$dir_name"
mkdir -p "$dir_name/$subdir_even"
mkdir -p "$dir_name/$subdir_noon"
mkdir -p "$dir_name/$subdir_morn"




for photo_path in "files/photos_input"/*.jpg;do
    # echo "$photo_path"
    base_name="$(basename "$photo_path")"
    photo_name="${base_name%.jpg}"

    time="${photo_name##*_}"

    # echo "$time"

    hour="${time:0:2}"

    echo "$hour"

    if ((10#$hour >= 0 && 10#$hour <=11));then
        new_name="morning_${base_name}"
        cp "$photo_path" "$dir_name/$subdir_morn/$new_name"
    fi

    if ((10#$hour >= 12 && 10#$hour <=17));then
        new_name="afternoon_${base_name}"
        cp "$photo_path" "$dir_name/$subdir_noon/$new_name"
    fi
    if ((10#$hour >= 18 && 10#$hour <=23));then
        new_name="evening_${base_name}"
        cp "$photo_path" "$dir_name/$subdir_even/$new_name"
    fi

done



: > count.txt

# morning_count="$(find "tourPhotos/Morning/" -type f | wc -l)"
# echo "Morning :${morning_count}" >> count.txt
# evening_count="$(find "tourPhotos/evening/" -type f | wc -l)"
# echo "Evening :${evening_count}" >> count.txt
# Afternoon_count="$(find "tourPhotos/Afternoon/" -type f | wc -l)"
# echo "Afternoon :${Afternoon_count}" >> count.txt





: > count.txt  # Clear the file first

while read -r directory; do
    # Skip the base directory
    if [[ "$directory" != "tourPhotos" ]]; then
        count=$(find "$directory" -type f | wc -l)
        echo "${directory#*/} : $count" >> count.txt
    fi
done < <(find "tourPhotos" -type d)
#



#==========================================another solution =================================#

# dir_name="tourPhotos"
# subdir_morn="Morning"
# subdir_noon="Afternoon"
# subdir_even="evening"

# mkdir -p "$dir_name/$subdir_morn"
# mkdir -p "$dir_name/$subdir_noon"
# mkdir -p "$dir_name/$subdir_even"

# for photo_path in files/photos_input/*.jpg; do
#     base_name="$(basename "$photo_path")"
#     photo_name="${base_name%.jpg}"
#     time="${photo_name##*_}"
#     hour="${time:0:2}"

#     # Remove leading zero using 10# to avoid octal interpretation
#     hour=$((10#$hour))

#     case $hour in
#         [0-9]|1[01])
#             new_name="morning_${base_name}"
#             cp "$photo_path" "$dir_name/$subdir_morn/$new_name"
#             ;;
#         1[2-7])
#             new_name="afternoon_${base_name}"
#             cp "$photo_path" "$dir_name/$subdir_noon/$new_name"
#             ;;
#         1[89]|2[0-3])
#             new_name="evening_${base_name}"
#             cp "$photo_path" "$dir_name/$subdir_even/$new_name"
#             ;;
#         *)
#             echo "Invalid hour in file: $base_name"
#             ;;
#     esac
# done
