#!/bin/bash

unroll_fact=$1
algo_trigger_point=$2
err_corr_iterations=$3
other_blocks=$4

# Define the file name
file_name="output.txt"

# Define the pattern to search for
pattern="${unroll_fact}_${algo_trigger_point}_${err_corr_iterations}_${other_blocks}"

# Define the string to add to the file
new_string="{unroll_fact}_{algo_trigger_point}_{error_corr_iterations}_{other_blocks_previous}   {arr_size}"

# Check if the file exists
if [ ! -f "$file_name" ]; then
    # If the file doesn't exist, create it and add the string to the first line
    echo "$new_string" > "$file_name"
fi

# Check if the file exists
if [ -f "$file_name" ]; then
    # Use grep to search for the pattern in the file
    match=$(grep -o "$pattern\s[0-9]*" "$file_name")
    
    # Check if a match is found
    if [ -n "$match" ]; then
        # Extract the value of arr_size
        arr_size=$(echo "$match" | awk '{print $2}')
        echo "arr_size is found"
    else
        echo "arr_size is not found"
    fi
else
    echo "File $file_name does not exist."
    exit
fi
