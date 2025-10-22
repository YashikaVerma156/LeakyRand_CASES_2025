#!/bin/bash

unroll_fact=$1
algo_trigger_point=$2
err_corr_iterations=$3
other_blocks=$4
arr_size=$5

# Define the file name
file_name="output.txt"

# Check if the file already exists
if [ -f "$file_name" ]; then
    echo "File $file_name already exists. Appending to it."
else
    echo "Creating new file $file_name."
    echo "{unroll_fact}_{algo_trigger_point}_{err_corr_iterations}_{other_blocks}  {arr_size}" >> "$file_name"
fi

# Define the information to write
new_info="${unroll_fact}_${algo_trigger_point}_${err_corr_iterations}_${other_blocks} ${arr_size}"

# Write the information to the file on a new line
echo "$new_info" >> "$file_name"

echo "Information written to $file_name."
