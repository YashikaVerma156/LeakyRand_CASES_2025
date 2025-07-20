#line_num=5
#l=`sed -n "${line_num}p" seeds_test.txt`
#IFS=',' read -ra columns <<< "$l"
#echo "${columns[32]}"
STR_NUM=98
if [ $STR_NUM -lt 100 ]
then
    echo "yes"
fi

exit

search_string="champsim.trace_sender_512_${STR_NUM}_"
sender_traces_directory="sender_test_benchmark_traces"
# Use a loop to iterate through each file in the directory
for filename in $(ls "$sender_traces_directory"); do

  # Check if the filename contains the search string
  if [[ $filename == *"$search_string"* ]]; then
    echo "Found match: $filename"

    # Split the filename using both '_' and '.' as delimiters
    IFS='_.' read -ra array <<< "$filename"

    # Extract the desired part (index 5 in this case)
    extracted_part="${array[5]}"

    # Print the extracted value
    echo "$extracted_part"

  fi

done
