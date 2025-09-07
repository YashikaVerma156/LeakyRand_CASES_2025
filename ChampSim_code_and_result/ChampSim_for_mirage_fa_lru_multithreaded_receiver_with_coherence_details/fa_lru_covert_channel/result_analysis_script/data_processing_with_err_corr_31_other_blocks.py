import os
import re
import sys
import pandas as pd
import math

def generate_total_error_count_skip_first_bit(data_file_path):
    errors = 0
    errors_0_to_1 = 0
    errors_1_to_0 = 0
    max_duration_bit_1 = 0
    min_duration_bit_1 = 0
    max_duration_bit_0 = 0
    min_duration_bit_0 = 0
    skip_first_bit_in_error = 0
    err=0
    #i=0
    match = re.search(r'per_string_data_strnum_(\d+)_', data_file_path)
    if match:
        str_num = match.group(1)
    else:
        print("Error: String number not found in file: ", data_file_path)
        exit(0)

    # Use regular expression to find algo trigger point.
    data_filename = os.path.basename(data_file_path)
    #print(filename)  # Output: abc.txt
    with open(data_file_path, "r") as file:
        for line_number, line in enumerate(file, start=1):
            #Corresponds to header in the text file.
            if(line_number == 1):
                continue
            line=line.strip()
            line=line.split(',')
            #print(line)
            original_bit = int(line[1])
            received_bit = int(line[2])
            bit_duration = int(line[3])

            #### Filter out max and min duration for marker bits and normal bits.

            # Calculate the errors for data bits only, after skipping the first bit.
            if skip_first_bit_in_error == 0:
                skip_first_bit_in_error += 1
                print("skipped first bit in error")
            else:
                if(original_bit != received_bit):
                    errors += 1
                    if(original_bit == 1):
                        errors_1_to_0 += 1
                    else:
                        errors_0_to_1 += 1

                 # line_number > 3 to avoid the initial one-time overhead. This overhead is due to the inefficient way of writing sender code due to time issue.
            if(original_bit == 1 and bit_duration > max_duration_bit_1 and line_number > 3):
                max_duration_bit_1 = bit_duration
                # First time both the variables are initialized by bit_duration.
                if(min_duration_bit_1 == 0):
                    min_duration_bit_1 = bit_duration
            elif(original_bit == 1 and bit_duration < min_duration_bit_1 and line_number > 3):
                min_duration_bit_1 = bit_duration
            elif(original_bit == 0 and bit_duration > max_duration_bit_0  and line_number > 3):
                max_duration_bit_0 = bit_duration
                # First time both the variables are initialized by bit_duration.
                if(min_duration_bit_0 == 0):
                    min_duration_bit_0 = bit_duration
            elif(original_bit == 0 and (bit_duration < min_duration_bit_0) and line_number > 3):
                min_duration_bit_0 = bit_duration
            #i+=1
            err+=1

    per_string_data[str_num] = [errors, errors_0_to_1, errors_1_to_0, max_duration_bit_1, min_duration_bit_1, max_duration_bit_0, min_duration_bit_0]


def generate_total_error_count(data_file_path):
    errors = 0
    errors_0_to_1 = 0
    errors_1_to_0 = 0
    max_duration_bit_1 = 0
    min_duration_bit_1 = 0
    max_duration_bit_0 = 0
    min_duration_bit_0 = 0
    err=0
    #i=0
    match = re.search(r'per_string_data_strnum_(\d+)_', data_file_path)
    if match:
        str_num = match.group(1)
    else:
        print("Error: String number not found in file: ", data_file_path)
        exit(0)

    # Use regular expression to find algo trigger point.
    data_filename = os.path.basename(data_file_path)
    #print(filename)  # Output: abc.txt
    with open(data_file_path, "r") as file:
        for line_number, line in enumerate(file, start=1):
            #Corresponds to header in the text file.
            if(line_number == 1):
                continue
            line=line.strip()
            line=line.split(',')
            #print(line)
            original_bit = int(line[1])
            received_bit = int(line[2])
            bit_duration = int(line[3])

            #### Filter out max and min duration for marker bits and normal bits.

            # Calculate the errors for data bits only.
            if(original_bit != received_bit):
                errors += 1
                if(original_bit == 1):
                    errors_1_to_0 += 1
                else:
                    errors_0_to_1 += 1

                 # line_number > 3 to avoid the initial one-time overhead. This overhead is due to the inefficient way of writing sender code due to time issue.
            if(original_bit == 1 and bit_duration > max_duration_bit_1 and line_number > 3):
                max_duration_bit_1 = bit_duration
                # First time both the variables are initialized by bit_duration.
                if(min_duration_bit_1 == 0):
                    min_duration_bit_1 = bit_duration
            elif(original_bit == 1 and bit_duration < min_duration_bit_1 and line_number > 3):
                min_duration_bit_1 = bit_duration
            elif(original_bit == 0 and bit_duration > max_duration_bit_0  and line_number > 3):
                max_duration_bit_0 = bit_duration
                # First time both the variables are initialized by bit_duration.
                if(min_duration_bit_0 == 0):
                    min_duration_bit_0 = bit_duration
            elif(original_bit == 0 and (bit_duration < min_duration_bit_0) and line_number > 3):
                min_duration_bit_0 = bit_duration
             
            #i+=1
            err+=1

    per_string_data[str_num] = [errors, errors_0_to_1, errors_1_to_0, max_duration_bit_1, min_duration_bit_1, max_duration_bit_0, min_duration_bit_0]

def read_file_line_by_line(file_path, string_number, string_len):
   ## Extract bit duration.
    pattern = r"cpu 0 is on wait,"
    cpus_wait_cycle_line_num=[]
    per_bit_data={}
    with open(file_path, "r") as file:
        for line_number, line in enumerate(file, start=1):
            if re.search(pattern, line):
                cpus_wait_cycle_line_num.append(line_number)

    ## Read the actual message.
    original_msg=[]   ## Original message after including the marker bits.
    if train_data == 1:
        with open('benchmark/benchmark_train.txt', "r") as file:
            for line_number, line in enumerate(file, start=1):
                if(line_number == string_number):
                    line = line.strip()
                    # Split the string into single characters and assign to an array
                    original_msg = [char for char in line]
    if train_data == 0:
        with open('benchmark/benchmark_test.txt', "r") as file:
            for line_number, line in enumerate(file, start=1):
                if(line_number == string_number):
                    line = line.strip()
                    # Split the string into single characters and assign to an array
                    original_msg = [char for char in line]
    err=0
    i=0

    ## Identify the received message and error.    
    received_msg=[]
    finish_time=[]
    sender_sent_msg=[]
    bit_interval=[]
    set_up_done=0
    turn_cpu1=0
    bit_duration_start = 0
    bit_duration_end = 0
    pattern=r'bit received is:'
    pattern1=r'SSSSSSSS cpu awakened from sleep is: 1'
    pattern2=r'clflush called on cpu: 1'
    pattern3=r'cpu 0 is on wait'
    pattern4=r'=========cache fill is done========'
    pattern5=r'Finished CPU 1 instructions'
    with open(file_path, "r") as file:
        for line_number, line in enumerate(file, start=1):
            line=line.strip()
            if re.search(pattern, line):
                match = re.search(r'bit received is: (\d+)', line)
                if match:
                    received_msg.append(match.group(1))
                else:
                    print(line)
                    print("Number not found in the string.", line_number)
                    exit(0)

            # Extract execution finish time.
            if re.search(pattern5, line):
                match = re.search(r'cycles: (\d+)', line)
                if match:
                    finish_time.append(match.group(1))
                else:
                    print(line)
                    print("Finish time not found in the string.", line_number)
                    exit(0)

            ## Identify if the sent mesage is indeed the one meant to send.
            if pattern4 in line:
                set_up_done=1
            if set_up_done == 1:
                if pattern1 in line:
                    turn_cpu1 = 1
                if turn_cpu1 == 1 and pattern2 in line:
                    turn_cpu1 += 1
                if pattern3 in line and turn_cpu1 > 0:
                    if turn_cpu1 == 1:
                        sender_sent_msg.append(0)
                    if turn_cpu1 == 2:
                        sender_sent_msg.append(1)
                    turn_cpu1 = 0

            ## Calculate bit interval.
            if pattern3 in line:
                match = re.search(r'current core cycle: (\d+) instr_', line)
                if match:
                    if bit_duration_start == 0:
                        bit_duration_start = match.group(1)
                    else:
                        bit_duration_end = match.group(1)
                        bit_interval.append(int(bit_duration_end)-int(bit_duration_start))
                        bit_duration_start = bit_duration_end
                else:
                    print(line)
                    print("Number not found in the string.", line_number)
                    exit(0)

    print(len(original_msg))
    print(len(sender_sent_msg))

    for i in range(0, 512):
        #print(i)
        if(int(original_msg[i]) != int(sender_sent_msg[i])):
            print(sender_sent_msg)
            print("i is: ",i," actual_msg_bit: ",original_msg[i]," sent_bit: ", sender_sent_msg[i])
            print("========== Something is not same between the sender_sent_msg and original_msg ============")
            exit(0)
    #print('counta: ',counta)

    #print(len(received_msg))
    #print(received_msg)


    print(len(bit_interval))
   #Calculate the errors in transmission.
    counta=0
    for i in range(1, 512):
        per_bit_data[int(i)] = [original_msg[i-1], received_msg[i-1], bit_interval[i-1]]
        if(int(original_msg[i-1]) != int(received_msg[i-1])):
            print("i is error position: ",i," original bit: ",int(original_msg[i-1])," received bit: ", int(received_msg[i-1]))
            if(i != 1):
                print("Something is wrong.")
                #exit(0)
            counta+=1
    #print('counta: ',counta)
    #exit(0)

    df = pd.DataFrame.from_dict(per_bit_data, orient='index', columns=['original_bit','received_bit','bit_interval'])
    # Rename the index column
    df = df.rename_axis('bit_position')
    df.to_csv(data_file_path, index=True)
    #print("written_to_file")

    # Assign the finish time. 
    per_string_fin_time[int(string_number)]=[finish_time[0]]

############### Programs starting point. ######################
# Check if no command-line arguments are provided
if len(sys.argv) != 2:
    print("One commandline argument is expected i.e 1 if the training_dataset results are processed or 0 if the testing_dataset results are processed. Exiting...")
    sys.exit(1)

if int(sys.argv[1]) > 1 or int(sys.argv[1]) < 0:
    print("The value should be either 0 or 1.")
    sys.exit(1)

# Access command-line argument.
train_data=int(sys.argv[1])


# Get the current working directory
cwd = os.getcwd()

# Define the directory name
directory_name = "extracted_results"

# Join the cwd and directory name to get the full path of the directory
directory_path = os.path.join(cwd, directory_name)

# Create the directory
try:
    os.mkdir(directory_path)
    print("Directory created successfully.")
except OSError as error:
    print(f"Failed to create directory: {error}")


#UFs=[32, 64, 128, 256]
string_len=512
string_number=0
res_dir_path='extracted_results'
directory_path='../results_with_helper_core'
#directory_path='../results'
file_list = os.listdir(directory_path)
            
#string='result_lru_3_champsim.trace_receiver_fa_lru_channel'
string='result_lru_3_champsim.trace_receiver_fa_lru'
string1='_514_with_both_algo_1_access_'
string2='access_train'
string3='access_test'

#Declaring empty dictionary to store data per message strings that are communicated.
per_string_data={}
per_string_fin_time={}
if train_data == 1:
    tot_err_cnt_file='total_error_count_train.csv'
    tot_exc_cyc_cnt_file='total_execution_cycle_count_train.csv'
    # Filter files that contain the specified string
    filtered_files = [file for file in file_list if string in file and string1 in file and string2 in file]
else:
    tot_err_cnt_file='total_error_count_test.csv'
    tot_exc_cyc_cnt_file='total_execution_cycle_count_test.csv'
		# Filter files that contain the specified string
    filtered_files = [file for file in file_list if string in file and string1 in file and string3 in file]
    count=0
print(len(filtered_files))
# Read the content of each file
for file_name in filtered_files:	 
    file_path = os.path.join(directory_path, file_name)
		# Use regular expression to find the string number.
    match = re.search(r'_\d+_(\d+)_\d+_\d+', file_name)
    if match:
        string_number = int(match.group(1))
        print("string_number ",string_number)
    else:
        print("Number not found in the string.")
        exit(0)

    if train_data == 1:
        data_file='per_string_data_strnum_'+str(string_number)+'_train_'+'.csv'
    else:
        data_file='per_string_data_strnum_'+str(string_number)+'_test_'+'.csv'
    data_file_path = os.path.join(res_dir_path, data_file)
    print(file_path,"count: ",count)
    read_file_line_by_line(file_path, string_number, string_len)
    # This script generate total errors and min-max bit duration.
    #generate_total_error_count(data_file_path)
    generate_total_error_count_skip_first_bit(data_file_path)
    #exit(0)
    count+=1

# Sort the dictionary.
sorted_items = sorted(per_string_data.items())
# Convert the sorted items back to a dictionary
per_string_data = dict(sorted_items)
# Create a DataFrame
df = pd.DataFrame.from_dict(per_string_data, orient='index', columns=['Total_error','errors_0_to_1','errors_1_to_0','Maximum_duration_1','Minimum_duration_1','Maximum_duration_0','Minimum_duration_0'])
# Rename the index column
df = df.rename_axis('Msg_string_num')
# Write to csv file.
tot_err_cnt_file_path = os.path.join(res_dir_path, tot_err_cnt_file)
df.to_csv(tot_err_cnt_file_path, index=True)

# Sort the dictionary.
sorted_items = sorted(per_string_fin_time.items())
# Convert the sorted items back to a dictionary
per_string_fin_time = dict(sorted_items)
# Create a DataFrame
df = pd.DataFrame.from_dict(per_string_fin_time, orient='index', columns=['execution_finish_time'])
# Rename the index column
df = df.rename_axis('Msg_string_num')
# Write to csv file.
tot_exc_cyc_cnt_file_path = os.path.join(res_dir_path, tot_exc_cyc_cnt_file)
df.to_csv(tot_exc_cyc_cnt_file_path, index=True)

### Calculating total errors and other stats. ###
cpu_clock_cycles=4000000000

print("=========== This script avoids taking into account the error in first bit for FA-LRU setup. It calculates the first bit errors but avoids counting them while writing to total_error_count_test.csv file. =============")

if train_data == 1:
    tot_err_cnt_file='total_error_count_train.csv'
    tot_exe_cyc_cnt_file='total_execution_cycle_count_train.csv'
    # Filter files that contain the specified string
    filtered_files = [file for file in file_list if string in file and string1 in file and string2 in file]
else:
    tot_err_cnt_file='total_error_count_test.csv'
    tot_exe_cyc_cnt_file='total_execution_cycle_count_test.csv'

tot_err_cnt_file_path = os.path.join(res_dir_path, tot_err_cnt_file)
tot_exe_cyc_cnt_file_path = os.path.join(res_dir_path, tot_exe_cyc_cnt_file)

finish_time = 0
#Below two variables are used to observe the range of finish time.
max_finish_time = 0
min_finish_time = 0
print(tot_exe_cyc_cnt_file_path)
with open(tot_exe_cyc_cnt_file_path, "r") as file:
    for line_number, line in enumerate(file, start=1):
        if(line_number == 1):
            continue
        line=line.strip()
        line=line.split(',')
        #print(line)
        if(max_finish_time == 0):
            max_finish_time = int(line[1])
            min_finish_time = int(line[1])
            finish_time = int(line[1])

        if(finish_time < int(line[1])):
            finish_time = int(line[1])
            max_finish_time = int(line[1])

        if(min_finish_time > int(line[1])):
            min_finish_time = int(line[1])

print(tot_err_cnt_file_path)
count=0
with open(tot_err_cnt_file_path, "r") as file:
    total_error_count = 0
    total_err_0_to_1 = 0
    total_err_1_to_0 = 0
    max_dur_1 = 0
    min_dur_1 = 0
    max_dur_0 = 0
    min_dur_0 = 0
    for line_number, line in enumerate(file, start=1):
        if(line_number == 1):
            continue
        line=line.strip()
        line=line.split(',')
        #print(line)
        total_error_count += int(line[1])
        total_err_0_to_1 += int(line[2])
        total_err_1_to_0 += int(line[3])
              
        if(line_number == 2):
            max_dur_1 = int(line[4])
            min_dur_1 = int(line[5])
            max_dur_0 = int(line[6])
            min_dur_0 = int(line[7])
        else:
            if(int(line[4]) > max_dur_1):
                max_dur_1 = int(line[4])
            if(int(line[5]) < int(min_dur_1)):
                min_dur_1 = int(line[5])
            if(int(line[6]) > int(max_dur_0)):
                max_dur_0 = int(line[6])
            if(int(line[7]) < int(min_dur_0)):
                min_dur_0 = int(line[7])
    text_file='Total_result_'+'_train_'+str(train_data)+'.txt'
    with open(text_file, 'w') as txtfile:
        ### Calculating bandwdith.
        cycles_to_send_one_bit=(max_dur_1)
        estimated_bandwidth=(cpu_clock_cycles/cycles_to_send_one_bit)/1024
        estimated_bandwidth=round(estimated_bandwidth,2)

        ### Calculating overall bandwidth, including initial prime by receiver threads.
        per_bit_cycles=round((finish_time/511),2)
        overall_bandwidth=(cpu_clock_cycles/per_bit_cycles)/1024
        overall_bandwidth=round(overall_bandwidth,2)
        txtfile.write(f'Total_error: {total_error_count} errors_0_to_1: {total_err_0_to_1} errors_1_to_0: {total_err_1_to_0} estimated_bandwidth: {estimated_bandwidth}Kbps overall_bandwidth: {overall_bandwidth}Kbps Maximum_duration_1: {max_dur_1} Minimum_duration_1: {min_dur_1} Maximum_duration_0: {max_dur_0} Minimum_duration_0: {min_dur_0} minimum_finish_time: {min_finish_time} maximum_finish_time: {max_finish_time} \n')
