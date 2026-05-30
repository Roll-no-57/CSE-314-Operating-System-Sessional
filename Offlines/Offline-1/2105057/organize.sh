#!/bin/bash

# check if minimum required are provided or not 
if [ $# -lt 4 ]; then
    echo "Usage: ./organize.sh <submissions> <targets> <tests> <answers> [-v] [-noexecute] [-nolc] [-nocc] [-nofc]"
    exit 1
fi


# Mandatory arguments from command line
SUBMISSIONS=$1
TARGETS=$2
TESTS=$3
ANSWERS=$4

# Initialize flags for optional arguments
VERBOSE=0
NOEXECUTE=0
NOLC=0
NOCC=0
NOFC=0


# Parse optional arguments
for arg in "${@:5}"; do
    case $arg in
        -v) VERBOSE=1 ;;
        -noexecute) NOEXECUTE=1 ;;
        -nolc) NOLC=1 ;;
        -nocc) NOCC=1 ;;
        -nofc) NOFC=1 ;;
        *) echo "Invalid option: $arg"; exit 1 ;;
    esac
done

# Create target directory
mkdir -p "$TARGETS"
mkdir -p "$TARGETS/C"
mkdir -p "$TARGETS/C++"
mkdir -p "$TARGETS/Java"
mkdir -p "$TARGETS/Python"


# Define CSV file and write dynamic header
csv_file="$TARGETS/result.csv"
header="student_id,student_name,language"
if [ $NOEXECUTE -eq 0 ]; then
    header="$header,matched,not_matched"
fi
if [ $NOLC -eq 0 ]; then
    header="$header,line_count"
fi
if [ $NOCC -eq 0 ]; then
    header="$header,comment_count"
fi
if [ $NOFC -eq 0 ]; then
    header="$header,function_count"
fi
echo "$header" > "$csv_file"



# we will only work with the zip files not other file formatts
for zipfile in "$SUBMISSIONS"/*.zip; do

#==================================Task A==================================#
#1. first we have to unzip the submission files and then organize them according to their file types and student ID 


    #Task-1: extract the studentID from file name
    # Now we need to parse the zipfile name and extract the studentID
    # $zipfile give the full path of the file relative to the workspace but we need actual file name
    filename=$(basename "$zipfile")
    studentIdZip="${filename##*_submission_}"               #this will return studentId.zip
    studentId="${studentIdZip%%.*}"                         #so remove the .zip extension

    # Extract the full name from the file name and format it
    full_name=$(echo "$filename" | sed 's/_[0-9]\{7\}_submission_.*//')
    student_name="${full_name//_/ }"

    #Task-2: unzip the zip file and store the unzipped file in a temporary directory because we need to process inside and only need the .c/.cpp/.java/.py
    tempDir="temp_$studentId"
    mkdir -p "$tempDir"
    unzip -qq "$zipfile" -d "$tempDir"


    #Task-3 : find the code file and rename them
    if [ -n "$(find "$tempDir" -type f -name "*.c")" ]; then
        codeFile="$(find "$tempDir" -type f -name "*.c")"
        language="C"
        targetSubDir="$TARGETS/C/$studentId"
        targetFile="$targetSubDir/main.c"
    elif [ -n "$(find "$tempDir" -type f -name "*.cpp")" ]; then
        codeFile="$(find "$tempDir" -type f -name "*.cpp")"
        language="C++"
        targetSubDir="$TARGETS/C++/$studentId"
        targetFile="$targetSubDir/main.cpp"
    elif [ -n "$(find "$tempDir" -type f -name "*.java")" ]; then
        codeFile="$(find "$tempDir" -type f -name "*.java")"
        language="Java"
        targetSubDir="$TARGETS/Java/$studentId"
        targetFile="$targetSubDir/Main.java"
    elif [ -n "$(find "$tempDir" -type f -name "*.py")" ]; then
        codeFile="$(find "$tempDir" -type f -name "*.py")"
        language="Python"
        targetSubDir="$TARGETS/Python/$studentId"
        targetFile="$targetSubDir/main.py"
    fi

    #task-4 :create subdirectories and copy the program and rename to main
    mkdir -p "$targetSubDir"
    cp "$codeFile" "$targetFile"

    # and lastly task-A done

#======================================Task-B================================#
# 1. now task B is to analyze the code metrics
# 2. calculate line count
# 3. calculate comment count
# 4. calculate function count
    lineCount=0
    commentCount=0
    functionCount=0

    if [ $NOLC -eq 0 ]; then
        lineCount=$(wc -l < "$targetFile")
    fi

    if [ $NOCC -eq 0 ]; then
        if [ "$language" = "C" ] || [ "$language" = "C++" ] || [ "$language" = "Java" ]; then
            commentCount=$(grep -c "//" "$targetFile")
        elif [ "$language" = "Python" ]; then
            commentCount=$(grep -c "#" "$targetFile")
        fi
    fi


    # function count 
    # return_type function_name(arguments){
    # 1. consider the leading white spaces (^\s*)
    # 2. consider a word as return type ([a-zA-Z0-9_]+)
    # 3. consider one or more white-space (\s+)
    # 4. consider function name as word ([a-zA-Z0-9_]+)
    # 5. consider again zero or more white-space(\s*)
    # 6. check the first left parenthesis (\()
    # 7. ignore everything except the closing bracket ([^)]*)
    # 8. check the right parenthesis(\))
    # 9. consider zero or more white-space(\s*)
    # 10. check the right curly braces(\{)

    if [ $NOFC -eq 0 ]; then
        if [ "$language" = "C" ] || [ "$language" = "C++" ]; then
            functionCount=$(grep -P -c "^\s*(?:[\w:<>\[\]\*&]+\s+)+[\w]+\s*\([^)]*\)\s*\{" "$targetFile")
        elif [ "$language" = "Java" ]; then
            #functionCount=$(grep -c "^\s*\(public\|private\|protected\)\s\+.*(.*).*{" "$targetFile")
            functionCount=$(grep -E '^[[:space:]]*(@\w+\s*)?(\w+\s+)*\w+\s*\(.*\)\s*\{' "$targetFile" | \
                grep -v -E '^[[:space:]]*(if|for|while|switch|else)' | \
                wc -l)
        elif [ "$language" = "Python" ]; then
            functionCount=$(grep -c -E "^\s*def\s+" "$targetFile")
        fi
    fi

    # Task-B done

#======================================Task-C================================#


    if [ $NOEXECUTE -eq 0 ]; then
        # Compile the code
        case "$language" in
            C) gcc "$targetFile" -o "$targetSubDir/main.out" ;;
            C++) g++ "$targetFile" -o "$targetSubDir/main.out" ;;
            Java) javac "$targetFile" ;;
        esac

        # Run the tests and compare outputs
        matched=0
        notMatched=0

        for testFile in "$TESTS"/test*.txt; do
            testNum=$(basename "$testFile" .txt | sed 's/test//')
            ansFile="$ANSWERS/ans$testNum.txt"
            outFile="$targetSubDir/out$testNum.txt"

            case "$language" in
                C|C++) "$targetSubDir/main.out" < "$testFile" > "$outFile" ;;
                Python) python3 "$targetSubDir/main.py" < "$testFile" > "$outFile" ;;
                Java) java -cp "$targetSubDir" Main < "$testFile" > "$outFile" ;;
            esac

            if diff -q "$outFile" "$ansFile" > /dev/null; then
                ((matched++))
            else
                ((notMatched++))
            fi
        done
    fi



    # Build and append CSV row
    studentData="$studentId,\"$student_name\",$language"
    if [ $NOEXECUTE -eq 0 ]; then
        studentData="$studentData,$matched,$notMatched"
    fi
    if [ $NOLC -eq 0 ]; then
        studentData="$studentData,$lineCount"
    fi
    if [ $NOCC -eq 0 ]; then
        studentData="$studentData,$commentCount"
    fi
    if [ $NOFC -eq 0 ]; then
        studentData="$studentData,$functionCount"
    fi
    echo "$studentData" >> "$csv_file"




    # clean up the temporary directory when done or any error occurs.
    rm -rf "$tempDir"

done



# Run this code : ./organize.sh submissions targets tests answers -v -noexecute -nolc -nocc -nofc