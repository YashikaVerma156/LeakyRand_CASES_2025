import os
import re
import subprocess
import sys


def extract_random_seed():

    pattern1="seed_number is: 2249"
    with open(file_path, "r") as file:
        for line_number, line in enumerate(file, start=1):
            if re.search(pattern1, line):
                line = line.strip('\n')
                line = line.split(" ")
                receiver_misses.append(int(line[7]))
                sender_misses.append(int(line[8]))

def process_files_in_directory(directory, benchmark_file, threshold):
    total_err_cnt=0
    thresholds=[]
    thresholds.append(int(threshold))
    for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        if os.path.isfile(filepath):
            # Process the file here (e.g., read, modify, analyze)
            #print(f"Processing file: {filepath}")
            # Extract seed number from the file.
            pattern1="seed_number is:"
            with open(filepath, "r") as file:
                for line_number, line in enumerate(file, start=1):
                    if re.search(pattern1, line):
                        line = line.strip('\n')
                        line = line.split(" ")
                        seed = str(line[2])
                        #print(filepath)
                        filename = filepath.split("/")[1]
                        msg_size = str(filename.split("_")[5])
                        msg_string = str(filename.split("_")[11])
                        sender_arr_size = str(filename.split("_")[15])
                        data_extract_script ='data_extraction_seed.py'
                        subprocess.run(["python3", data_extract_script, filepath, sender_arr_size, msg_size, msg_string, seed])
                        continue
  
                        delta_calculation_script ='delta_calculation_seed.py'
                        extracted_data_filename="output_" + str(sender_arr_size) +"_"+str(msg_string)+"_"+str(seed)+"_"+str(msg_size)+".txt"
                        extracted_data_dir="extracted_data"
                        extracted_file_path = os.path.join(extracted_data_dir, extracted_data_filename)
                        subprocess.run(["python3", delta_calculation_script, extracted_file_path, sender_arr_size, msg_size, msg_string, seed, benchmark_file])
       ######   Error calculation   ######

                        #thresholds=[-3259]    #[-5500]
                        for th in thresholds:
                            extracted_data_dir="extracted_data"

                            err_cnt_file_name="error_count_sender_arr_size_"+str(sender_arr_size)+"_msg_size_"+str(msg_size)+"_th_"+str(th)+".txt"
                            err_cnt_file_path = os.path.join(extracted_data_dir, err_cnt_file_name)
                            if os.path.exists(err_cnt_file_path):
                                os.remove(err_cnt_file_path)
                                print(f"{err_cnt_file_path} deleted.")
                            else:
                                print(f"{err_cnt_file_path} does not exist.")

                            err_diff_file_name="error_diff_0to1_sender_arr_size_"+str(sender_arr_size)+"_msg_size_"+str(msg_size)+"_th_"+str(th)+".txt"
                            err_diff_file_path = os.path.join(extracted_data_dir, err_diff_file_name)
                            if os.path.exists(err_diff_file_path):
                                os.remove(err_diff_file_path)
                                print(f"{err_diff_file_path} deleted.")
                            else:
                                print(f"{err_diff_file_path} does not exist.")

                            err_diff_file_name="error_diff_1to0_sender_arr_size_"+str(sender_arr_size)+"_msg_size_"+str(msg_size)+"_th_"+str(th)+".txt"
                            err_diff_file_path = os.path.join(extracted_data_dir, err_diff_file_name)
                            if os.path.exists(err_diff_file_path):
                                os.remove(err_diff_file_path)
                                print(f"{err_diff_file_path} deleted.")
                            else:
                                print(f"{err_diff_file_path} does not exist.")



                            error_calculation_script ='error_calculation_seed.py'
                            extracted_data_filename="output_" + str(sender_arr_size) +"_"+str(msg_string)+"_"+str(seed)+"_"+str(msg_size)+".txt"
                            extracted_file_path = os.path.join(extracted_data_dir, extracted_data_filename)
                            subprocess.run(["python3", error_calculation_script, extracted_file_path, sender_arr_size, msg_size, msg_string, seed, str(th), benchmark_file])

                        min_max_script ='min_max_latency_diff.py'
                        extracted_data_filename="output_" + str(sender_arr_size) +"_"+str(msg_string)+"_"+str(seed)+"_"+str(msg_size)+".txt"
                        extracted_data_dir="extracted_data"
                        extracted_file_path = os.path.join(extracted_data_dir, extracted_data_filename)
                        subprocess.run(["python3", min_max_script, extracted_file_path, sender_arr_size, msg_size, msg_string, seed, benchmark_file])

######   Add errors for whole directory   ######
    #thresholds=[-3259]    #[-5500]
    #for th in thresholds:
    #    total_err_cnt=0
    #    zero_to_one=0
    #    one_to_zero=0
        # File path
    #    err_file_dir="extracted_data"
    #    err_result_file = "error_count_sender_arr_size_"+str(sender_arr_size)+"_msg_size"+"_"+str(msg_size)+"_th_"+str(th)+".txt"
        # Create the file path
    #    err_file_path = os.path.join(err_file_dir, err_result_file)
    #    str_to_match="Error is:"
    #    with open(err_file_path, 'r') as file:
    #        for line_number, line in enumerate(file, start=1):
    #            if re.search(str_to_match, line):
    #                err=int(line.split(" ")[2])
    #                err01=int(line.split(" ")[4])
    #                err10=int(line.split(" ")[6])
    #                total_err_cnt+= err
    #                zero_to_one+= err01
    #                one_to_zero+= err10
    #    new_data='Total Error: '+str(total_err_cnt)+' Total zero_to_one: '+str(zero_to_one)+' Total one_to_zero: '+str(one_to_zero)
        # Open the file in append mode and write new data
        #XXX below lines are commented intentionally.
    #    with open(err_file_path, 'a') as file:
    #        file.write(new_data+ '\n')  # Add a newline after the new data

# Get the current working directory
cwd = os.getcwd()

# Define the directory name
directory_name = "extracted_data"

# Join the cwd and directory name to get the full path of the directory
directory_path = os.path.join(cwd, directory_name)

# Create the directory
try:
    os.mkdir(directory_path)
    print("Directory created successfully.")
except OSError as error:
    print(f"Failed to create directory: {error}")

# 1. List of directories to process
#directories_to_process = ['sender_arr_size_19660']
#directories_to_process = ['sender_arr_size_4915']

directories_to_process = ['sender_arr_size_19660', 'sender_arr_size_4915','sender_arr_size_9830']

benchmarks="benchmark_test.txt"
benchmark_dir="benchmark"
benchmark_file = os.path.join(benchmark_dir, benchmarks)

# 2. Process files in each directory
for directory in directories_to_process:
    if os.path.exists(directory):

      ##### Create a file for later use.
        # File path
        sender_array_size=directory.split('_')[3]
        msg_size=512
        file_dir="extracted_data"
        result_file = "min_max_latency_difference_sender_arr_size_"+str(sender_array_size)+"_msg_size"+"_"+str(msg_size)+".txt"
        # Create the file path
        file_path = os.path.join(file_dir, result_file)
        # Open the file in append mode and write new data
        with open(file_path, 'w') as f:
            f.write('min_bit0,max_bit0,min_bit1,max_bit1,msg_string,seed,bit_num_min0,bit_num_max0,bit_num_min1,bit_num_max1'+'\n')  # Add a newline after the new data
        #threshold_file_path="../spp_results_analysis_scripts_train/calculate_optimal_threshold/temp_optimal_threshold_"+str(sender_array_size)+".txt"
        #with open(threshold_file_path, "r") as th_fl:
        #    for line in th_fl:
        #        line=line.strip()
        #        line=line.split(" ")
        #        threshold=int(line[5])
                #sanity_check
        #        if int(line[1]) != int(sender_array_size):
        #            print("Correct threshold file is not found.")
        #            exit(1)
        #print(threshold)
        #exit(1)
        #print(f"Processing files in directory: {directory}")
        process_files_in_directory(directory, benchmark_file, 0)

    else:
        print(f"Directory '{directory}' does not exist.")
#seed_number is: 2249

