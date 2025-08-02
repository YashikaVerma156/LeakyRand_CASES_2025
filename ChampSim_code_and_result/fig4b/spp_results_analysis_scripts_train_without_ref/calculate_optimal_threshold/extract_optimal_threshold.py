import os
import re
import subprocess
import sys
import glob

def process_files_in_directory(directory, threshold_start, threshold_end, benchmark_file, processed_sender_arr_size):
    total_err_cnt=0
    for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        file_dir='extracted_data'
        if os.path.isfile(filepath) and "output_" in filename:
            sender_arr_size=filename.split('_')[1]
            msg_string=filename.split('_')[2]
            seed=filename.split('_')[3]
            msg_size=filename.split('_')[4].split('.')[0]
            #print(msg_size)
            if int(sender_arr_size) == int(processed_sender_arr_size) and str(filename.split('_')[0]) == "output":
                #print("Hi",msg_size)
                ######   Error calculation   ######
                for th in range(threshold_start, threshold_end+1, 1):
                    #print(msg_size)
                    error_calculation_script ='error_calculation_seed.py'
                    extracted_file_path = os.path.join(directory, filename)
                    #print(extracted_file_path)
                    subprocess.run(["python3", error_calculation_script, extracted_file_path, sender_arr_size, msg_size, msg_string, seed, str(th), file_dir, benchmark_file])

    #print("Hi")
    ######   Add errors for whole directory   ######
    for th in range(threshold_start, threshold_end+1, 1):
        total_err_cnt=0
        zero_to_one=0
        one_to_zero=0
        err_result_file = "error_count_sender_arr_size_"+str(processed_sender_arr_size)+"_msg_size"+"_"+str(msg_size)+"_th_"+str(th)+".txt"
        ######## Create the file path
        err_file_path = os.path.join(file_dir, err_result_file)
        str_to_match="Error is:"
        with open(err_file_path, 'r') as file:
            for line_number, line in enumerate(file, start=1):
                if re.search(str_to_match, line):
                    err=int(line.split(" ")[2])
                    err01=int(line.split(" ")[4])
                    err10=int(line.split(" ")[6])
                    total_err_cnt+= err
                    zero_to_one+= err01
                    one_to_zero+= err10
        new_data='Total Error: '+str(total_err_cnt)+' Total zero_to_one: '+str(zero_to_one)+' Total one_to_zero: '+str(one_to_zero)
        # Open the file in append mode and write new data
        with open(err_file_path, 'a') as file:
            file.write(new_data+ '\n')  # Add a newline after the new data

# List of directories to process
# Use only the below three directories as in the remaining sender disturbance i.e. 40% and 50%, no overlap is observed.

directories_to_train = ['sender_arr_size_3276', 'sender_arr_size_6554','sender_arr_size_9830','sender_arr_size_13108','sender_arr_size_16380']

#Do it for benchmark_train.txt, as the threshold with the least errors will be applied as threshold for test_benchmark_suite results ?
benchmarks="benchmark_train.txt"
benchmark_dir="benchmark"
benchmark_file = os.path.join(benchmark_dir, benchmarks)

file_dir='extracted_data'
# Get the current working directory
cwd = os.getcwd()
# Join the cwd and directory name to get the full path of the directory
directory_path = os.path.join(cwd, file_dir)
# Create the directory 
try:
    os.mkdir(directory_path)
    print("Directory created successfully.")
except OSError as error:
    print(f"Failed to create directory: {error}")

# Process files in each directory
for directory in directories_to_train:
    result_dir = "../extracted_data"
    processed_sender_arr_size = 0 
    if os.path.exists(result_dir):
        processed_sender_arr_size=int(directory.split('_')[3])

        # Calculate the threshold_start threshold_end range.
        min_max_range_fl="../temp_min_max_range_"+str(processed_sender_arr_size)+".txt"
        with open(min_max_range_fl,"r") as m_fl:
            for line in m_fl:
                line=line.strip()
                line=line.split(" ")
                min_bit1=int(line[7])
                max_bit1=int(line[9])
                min_bit0=int(line[3])
                max_bit0=int(line[5])
        #No overlap between ranges.
        if max_bit0 < min_bit1:
            threshold_start=int((min_bit1 - max_bit0)/2 ) + max_bit0
            threshold_end=threshold_start+1

        # Overlap between ranges.
        if min_bit1 < max_bit0:
            threshold_start = min_bit1 - 1
            threshold_end = max_bit0 + 1


        # Delete files to not be overwritten. Build the pattern
        pattern = f"error_count_sender_arr_size_{processed_sender_arr_size}_msg_size*"
        # Get all matching files
        files_to_delete = glob.glob(os.path.join(directory_path, pattern))
        # Delete each file
        for file_path in files_to_delete:
            try:
                os.remove(file_path)
                print(f"Deleted: {file_path}")
            except Exception as e:
                print(f"Failed to delete {file_path}: {e}")

        pattern = f"error_diff_1to0_sender_arr_size_{processed_sender_arr_size}_msg_size*"
        # Get all matching files
        files_to_delete = glob.glob(os.path.join(directory_path, pattern))
        # Delete each file
        for file_path in files_to_delete:
            try:
                os.remove(file_path)
                print(f"Deleted: {file_path}")
            except Exception as e:
                print(f"Failed to delete {file_path}: {e}")

        pattern = f"error_diff_0to1_sender_arr_size_{processed_sender_arr_size}_msg_size*"
        # Get all matching files
        files_to_delete = glob.glob(os.path.join(directory_path, pattern))
        # Delete each file
        for file_path in files_to_delete:
            try:
                os.remove(file_path)
                print(f"Deleted: {file_path}")
            except Exception as e:
                print(f"Failed to delete {file_path}: {e}")


        print("sender_arr_size:",processed_sender_arr_size,"threshold_start:",threshold_start,"threshold_end:", threshold_end)
        process_files_in_directory(result_dir, threshold_start, threshold_end, benchmark_file, processed_sender_arr_size)

    else:
        print(f"Directory '{result_dir}' does not exist.")

