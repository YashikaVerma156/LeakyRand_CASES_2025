#!/bin/bash

# Usage: ./script.sh prefix

prefix="$1"

# Output file will be named as the prefix
output_file="${prefix}_output.txt"
> "$output_file"  # Empty the file if it exists

# Loop over directories matching the prefix
for dir in "${prefix}"*/; do
    # Check if it's a directory and contains the desired file
    file="${dir}board.pc.com_1.device"
    if [[ -d "$dir" && -f "$file" ]]; then
        # Extract second last line and append to output
        second_last_line=$(tail -n 2 "$file" | head -n 1)
        echo "$second_last_line" >> "$output_file"
    fi
done

echo "Output written to $output_file"

