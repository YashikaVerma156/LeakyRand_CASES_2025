import os
import re
import sys
import pandas as pd
import math

#def read_file_line_by_line(file_path, string_number, atp, string_len):
def read_file_line_by_line(file_path,data_file_path,atp,string_number):
    per_bit_data={}
    pattern0=r"cpu awakened from sleep is: 0"
    pattern1=r"cpu awakened from sleep is: 1"
    pattern=r"cpu 0 is on wait,"
    bit_duration_start=0
    bit_duration_end=0
    sender_duration_start=0
    sender_duration_end=0
    bit_received=0
    bit_interval=[]
    sender_interval=[]
    receiver_interval=[]
    with open(file_path, "r") as file:
        for line_number, line in enumerate(file, start=1):
          #  if re.search(pattern, line):
                #print(f"Pattern found in line {line_number}: {line.strip()}")
           #     cpu0_wait_cycle_line_num.append(line_number)

            ## Calculate bit interval.
            if pattern in line:
                match = re.search(r'current core cycle: (\d+) instr_', line)
                if match:
                    if bit_duration_start == 0:
                        bit_duration_start = match.group(1)
                    else:
                        bit_duration_end = match.group(1)
                        if(bit_received < atp):
                            bit_interval.append(int(bit_duration_end)-int(bit_duration_start))
                            #print("bit received: ",bit_received," interval: ",duration)
                        #else:
                            #print("Skipped the bit in bit interval.")
                        bit_duration_start = bit_duration_end
                        sender_duration_start = 0
                        if( (bit_received > int(atp)) and bit_received % int(atp) == 1 ):
                            bit_received=0
                else:
                    print(line)
                    print("Number not found in the string.", line_number)
                    exit(0)
            ## Calculate sender interval start.
            if pattern1 in line:
                match = re.search(r'current cycle: (\d+) instr unique', line)
                if match:
                    if sender_duration_start == 0:
                        sender_duration_start = match.group(1)
                    else:
                        print("Something is wrong in line 51")
                else:
                    print(line)
                    print("Number not found in the string.", line_number)
                    exit(0)
            ## Calculate sender interval end.
            if pattern0 in line:
                match = re.search(r'current cycle: (\d+) instr unique', line)
                if match:
                    sender_duration_end = match.group(1)
                    if int(sender_duration_start) > 0:
                        bit_received += 1
                        sender_duration_end = match.group(1)
                        if(bit_received < atp):
                            sender_interval.append(int(sender_duration_end)-int(sender_duration_start))
                            #duration=int(sender_duration_end)-int(sender_duration_start)
                            #print("bit received: ",bit_received," interval: ",duration)
                        #else:
                            #print("Skipped the bit in sender interval")
                        sender_duration_start = 0
                    else:
                        print("Something is wrong in line 70.")
                else:
                    print(line)
                    print("Number not found in the string.", line_number)
                    exit(0)

    if(len(bit_interval) != 512 and len(sender_interval) != 512):
        print("Something is wrong in line 83.")
        exit(0)

    for i in range(0,512):
        #print(sender_interval[i])
        #print(bit_interval[i])
        receiver_interval.append(int(bit_interval[i]) - int(sender_interval[i]))
    print(len(receiver_interval))

    for i in range(0,512):
        per_bit_data[int(i)] = [bit_interval[i], receiver_interval[i], sender_interval[i]]

    df = pd.DataFrame.from_dict(per_bit_data, orient='index', columns=['bit_interval','receiver_interval','sender_interval'])
    # Rename the index column
    df = df.rename_axis('bit_position')
    df.to_csv(data_file_path, index=True)

#file_path='/data/yashikav/sender_disturb_multiple_LLC_blocks/unoccupied_llc_1_percent/champsim_results/result_random_2_champsim.trace_receiver_multiple_access_16_other_blocks_326_512_512_with_both_algo_98_algo_itr.gz_champsim.trace_sender_512_1_270764_512_with_both_algo_98_access_test.gz.txt'

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

UFs=[16]
#Algo_trigger_point=[512, 256, 128, 64, 32, 16, 8]
Algo_trigger_point=[128]
string_len=512
err_corr_iterations=3  #Maximum number of times error-correction can be repeated.

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

string_number=0
per_string_data={}
#directory_path='/data/yashikav/sender_disturb_multiple_LLC_blocks/unoccupied_llc_1_percent/champsim_results/'
directory_path='/home/yashikav/Desktop/Mirage_project/fig4a/champsim_results/'
res_dir_path='extracted_results'
# Get a list of all files in the specified directory
file_list = os.listdir(directory_path)

for UF in UFs:
    for atp in Algo_trigger_point:
        #other_blocks=31
        other_blocks=326
        if atp == 8:
            err_corr_iterations=4  #Maximum number of times error-correction can be repeated.
        elif atp == 16:
            err_corr_iterations=5  #Maximum number of times error-correction can be repeated.
        elif atp == 32:
            err_corr_iterations=6  #Maximum number of times error-correction can be repeated.
        elif atp == 64:
            err_corr_iterations=9  #Maximum number of times error-correction can be repeated.
        elif atp == 128:
            err_corr_iterations=12  #Maximum number of times error-correction can be repeated.
        elif atp == 256:
            err_corr_iterations=25  #Maximum number of times error-correction can be repeated.
            #err_corr_iterations=26  #Maximum number of times error-correction can be repeated.
        elif atp == 512:
            err_corr_iterations=98  #Maximum number of times error-correction can be repeated.
        elif atp == 514:
            err_corr_iterations=100  #Maximum number of times error-correction can be repeated.

        print('processing for UF: ',UF,' Algo_trigger_point: ',atp)
        string='multiple_access_'+str(UF)+'_other_blocks_'+str(other_blocks)+'_'
        string2='_'+str(atp)+'_with_both_algo_'+str(err_corr_iterations)+'_access_train.gz.txt'
        string3='_'+str(atp)+'_with_both_algo_'+str(err_corr_iterations)+'_access_test.gz.txt'

        if train_data == 1:
            # Filter files that contain the specified string
            filtered_files = [file for file in file_list if string in file and string2 in file]
        else:
            # Filter files that contain the specified string
            filtered_files = [file for file in file_list if string in file and string3 in file]
        count=0
        print(len(filtered_files))
        #exit(0)
        # Read the content of each file
        for file_name in filtered_files:

            file_path = os.path.join(directory_path, file_name)

            # Use regular expression to find the string number.
            match = re.search(r'_\d+_(\d+)_\d+_\d+', file_name)

            if match:
                string_number = int(match.group(1))
                print("string_number ",string_number)
                print("count: ",count)
                count+=1
            else:
                print("Number not found in the string.")
                exit(0)
            if train_data == 1:
                data_file='cdf_data_UF_'+str(UF)+'_'+str(atp)+'_'+str(err_corr_iterations)+'_other_blocks_'+str(other_blocks)+'_train_'+str(string_number)+'.csv'
            else:
                data_file='cdf_data_UF_'+str(UF)+'_'+str(atp)+'_'+str(err_corr_iterations)+'_other_blocks_'+str(other_blocks)+'_test_'+str(string_number)+'.csv'

            data_file_path = os.path.join(res_dir_path, data_file)

            read_file_line_by_line(file_path,data_file_path,atp,string_number)
            #exit(0)
