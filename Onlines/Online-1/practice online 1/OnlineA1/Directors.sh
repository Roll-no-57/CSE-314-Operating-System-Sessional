#!/bin/bash

# Steps :
#   for each files --done
#       extract its movie name and director name    --done
#       create separate directory in the name of the director if not exists   --done
#       move  this file to that directory           --done solved
#   done


#   <Movie name>
#
#   <Story line>
#
#   <Directors name>
#   <Years active>

#   
for text_file in "movie_data"/*.txt;do
    # echo "$(basename "$text_file")"
    # grep_output=$(grep -cv Avatar "$text_file")
    # echo "$filename"
    # echo "$grep_output"

    #Now in this file we need director name which is located in above of the last line
    director="$(tail -n 2 "$text_file" | head -n 1)"

    echo "$director"

    # create a directory named after the director if not exists
    if [ ! -d "$director" ];then
        mkdir -p "$director"
    fi

    mv "$text_file" "$director"
done


# Recovery script
# for text_file in */*.txt;do
#     director="$(tail -n 2 "$text_file" | head -n 1)"
#     echo "$text_file"
#     mv "$text_file" "movie_data/"
#     rmdir "$director"       
# done