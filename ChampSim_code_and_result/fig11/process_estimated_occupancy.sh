file_name="program_output_for_1_percent_unoccupied_llc.txt"

first_line=$(grep -ni "Solution: Remaining budget 168930" ${file_name} | awk -F ':' '{ print $1 }')

sed -n '45,55p' ${file_name} | grep rem_budget | awk '{print $10}'  > estimated_occ.txt
