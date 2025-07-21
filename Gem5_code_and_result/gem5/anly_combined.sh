#!/bin/bash

# Usage: ./script.sh prefix

prefix="$1"

# Output file will be named as the prefix
output_file="${prefix}_output.txt"
> "$output_file"  # Empty the file if it exists

# Find matching directories and sort them
find . -maxdepth 1 -type d -name "${prefix}*" | sort | while read -r dir; do
    dir="${dir#./}"  # Remove leading './'
    file="${dir}/board.pc.com_1.device"

    echo "Checking directory: $dir"
    echo "Looking for file: $file"

    if [[ -f "$file" ]]; then
        echo "Found file: $file"
        second_last_line=$(tail -n 2 "$file" | head -n 1)
        echo "Second last line: $second_last_line"
        echo "$second_last_line" >> "$output_file"
    else
        echo "File not found: $file"
    fi
done

echo "Output written to $output_file"

