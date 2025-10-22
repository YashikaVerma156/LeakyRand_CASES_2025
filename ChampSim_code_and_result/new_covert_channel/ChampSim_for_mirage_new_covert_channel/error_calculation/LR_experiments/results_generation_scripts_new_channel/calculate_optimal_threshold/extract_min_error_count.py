import re
import os
import sys

#1. List relevant output files.
#sender_arr_size=[3276, 6554, 9830, 13108, 16384]
sender_arr_size=[3276, 6554, 9830]
src_dir='extracted_data'
for size in sender_arr_size:
    min_err_count=10000000000
    Threshold=10000000000
    # Filter files with a specific string
    specific_string = 'error_count_sender_arr_size_'+str(size)  # Replace with your specific string
    file_list = [f for f in os.listdir(src_dir) if specific_string in f]
    #print(size,' ' ,len(file_list))
#2. Traverse all files and extract the threshold with the least error count.
    for fname in file_list:
        file_path = os.path.join(src_dir, fname)
        pattern="Total Error: "

        with open(file_path, "r") as file:
            for line_number, line in enumerate(file, start=1):
                if re.search(pattern, line):
                    line = line.strip('\n')
                    err_count = int(line.split(' ')[2])
                    if(err_count < min_err_count):
                        min_err_count = err_count
                        Threshold = int(fname.split('_')[10].split('.')[0])

#3. Counting the range of optimal threshold.
    th_range=[]
    for fname in file_list:
        file_path = os.path.join(src_dir, fname)
        pattern="Total Error: "

        with open(file_path, "r") as file:
            for line_number, line in enumerate(file, start=1):
                if re.search(pattern, line):
                    line = line.strip('\n')
                    err_count = int(line.split(' ')[2])
                    if(err_count == min_err_count):
                        th_range.append(int(fname.split('_')[10].split('.')[0]))
    
#4. Print them.
    th_range.sort()
    print(th_range)
    print("size: ", size, ' min_err_count: ', min_err_count, ' Threshold: ', Threshold)
