# The concept of this script is that verify_error() and verify_1to0_error() is used to identify if there are any error in the file due to incomplete probing.
# i) The verify_error() check for 0to1 errors originating due to the following scenario. 
#    a) The receiver did not probe completely in current bit and the next bit is 0, so in the next bit the array blocks that were not probed and were a miss will 
#       be accessed again and will make the receiver decode the bit as 1( although it is 0). While concluding the next bit, don't consider misses in the un-probed 
#       area and conclude the bit as zero if there are no misses in the probed-area of the previous bit.
# ii) The verify_1to0_error() check for 1to0 errors, for scenario where every access of the sender went to the unprobed region of receiver-array, leading to 1to0 
#     error.


import os
import re
import sys
import subprocess
import pandas as pd

def verify_1to0_error(file_path, bit_position, equivalent_num_uf_chunks_to_avoid):
    bit_position=int(bit_position)+2
    # Construct the command
    command = f'grep -nri "cpu 0 is on wait" {file_path} | head -n {bit_position} | tail -n 2'
    
    # Execute the command
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    # Split the output into lines and count them
    lines = result.stdout.strip().split("\n")  # Strip to remove extra newlines
    #print(lines)
    line_count = len([line for line in lines if line])  # Ignore empty lines
    # Extract numbers before the first colon
    ids = [line.split(":")[0] for line in lines]
    print(ids)
    # Construct the command #XXX
    command=f"sed -n '{ids[0]},{ids[1]}p' {file_path} | grep -i 'RDTSC, sub:' | head -n {equivalent_num_uf_chunks_to_avoid} | awk '$3 > 124'"
    result=subprocess.run(command, shell=True, capture_output=True, text=True)

    # Strip and split the output
    lines = result.stdout.strip().split("\n")

    # Remove empty lines
    lines = [line for line in lines if line]

    # Check if the result is empty
    if lines:
        return 0
    else:
        print("Number of lines in output:", len(lines))
        print("Extracted lines:", lines)
        print('bit_position: ',bit_position)
        return 0

    #The control should not reach here.
    exit(0)

def verify_error(file_path, bit_position, equivalent_num_uf_chunks_to_avoid):   # Identify the false errors, i.e. 0 becoming 1 due to misses in non-probed LLC blocks.
    #print(file_path)
    #print(bit_position)
    #### XXX 1. Process the previous bit of bit in error. To ensure if the previous bit had incomplete probe.
    prev_bit_position=int(bit_position)+1 
    # Construct the command
    command = f'grep -nri "cpu 0 is on wait" {file_path} | head -n {prev_bit_position} | tail -n 2'
    # Execute the command
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    # Split the output into lines and count them
    lines = result.stdout.strip().split("\n")  # Strip to remove extra newlines
    line_count = len([line for line in lines if line])  # Ignore empty lines
    # Extract numbers before the first colon
    ids = [line.split(":")[0] for line in lines]
    command = f"sed -n '{ids[0]},{ids[1]}p' {file_path} | grep 'DONT_PROBE_COMPLETE' "
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    # Strip and split the output
    lines = result.stdout.strip().split("\n")
    # Remove empty lines
    lines = [line for line in lines if line]
    # Check if the result is empty
    if lines:
        prev_bit_probe_incomplete = 1
    else:
        prev_bit_probe_incomplete = 0


    #### XXX 2. Process the bit in error.
    bit_position=int(bit_position)+2 # 2 is added as bit_position is started from 0 and there is an extra "cpu 0 is on wait" due to first probe.
    # Construct the command
    command = f'grep -nri "cpu 0 is on wait" {file_path} | head -n {bit_position} | tail -n 2'

    # Execute the command
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    #print(result)
    # Split the output into lines and count them
    lines = result.stdout.strip().split("\n")  # Strip to remove extra newlines
    #print(lines)
    line_count = len([line for line in lines if line])  # Ignore empty lines
    # Extract numbers before the first colon
    ids = [line.split(":")[0] for line in lines]
    #print(ids[0]," ",ids[1])

    # Print the number of lines
    #print("Number of lines in output:", line_count)
    # Construct the command #XXX 1964 is for 97% only.
    command = f"sed -n '{ids[0]},{ids[1]}p' {file_path} | grep -i 'RDTSC, sub:' | head -n {equivalent_num_uf_chunks_to_avoid} | awk '$3 > 123'"
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    # Strip and split the output
    lines = result.stdout.strip().split("\n")
    # Remove empty lines
    lines = [line for line in lines if line]

    # Check if the result is empty
    if not lines and (prev_bit_probe_incomplete == 1):
        print("==========Correct this error============")
        #exit(0)
        return 0
    else:
        print("Number of lines in output:", len(lines))
        print("Extracted lines:", lines)
        print('bit_position: ',bit_position)
        return 1

    #The control should not reach here.
    exit(0)

def generate_total_error_count(data_file_path):
    errors = 0
    errors_0_to_1 = 0
    errors_1_to_0 = 0
    max_duration_bit_1 = 0
    min_duration_bit_1 = 0
    max_duration_bit_0 = 0
    min_duration_bit_0 = 0
    max_duration_algo1 = 0
    min_duration_algo1 = 0
    max_duration_algo2 = 0
    min_duration_algo2 = 0
    sender_evicted_its_own_block = 0
    hole_was_created = 0
    receiver_inactive_active_block_is_evicted=0
    hole_filled_up = 0
    err=0
    #i=0
    match = re.search(r'per_string_data_strnum_(\d+)_UF', data_file_path)
    if match:
        str_num = match.group(1)
    else:
        print("Error: String number not found in file: ", data_file_path)
        exit(0)

    # Use regular expression to find algo trigger point.
    data_filename = os.path.basename(data_file_path)
    #print(filename)  # Output: abc.txt
    match = re.search(r'UF_\d+_(\d+)_', data_filename)
    if match:
        atp_num = int(match.group(1))
    else:
        print("Number not found in the string.")
        exit(0)

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
            if(err+1) % atp_num == 0:
                if(original_bit == 0):
                    print("Something is wrong")
                    exit(0) 
                err+=1
                if(bit_duration > max_duration_algo1):
                    max_duration_algo1 = bit_duration
                    # First time both the variables are initialized by bit_duration.
                    if(min_duration_algo1 == 0):
                        min_duration_algo1 = bit_duration
                elif(bit_duration < min_duration_algo1):
                    min_duration_algo1 = bit_duration
            elif((err+1) % atp_num == 1 and err > 1):
                if(original_bit == 1):
                    print("Something is wrong")
                    exit(0)
                err = 0
                if(bit_duration > max_duration_algo2):
                    max_duration_algo2 = bit_duration
                    # First time both the variables are initialized by bit_duration.
                    if(min_duration_algo2 == 0):
                        min_duration_algo2 = bit_duration
                elif(bit_duration < min_duration_algo2):
                    min_duration_algo2 = bit_duration
            else:

                # Calculate the errors for data bits only.
                if(original_bit != received_bit):
                    errors += 1
                    if(original_bit == 1):
                        errors_1_to_0 += 1
                    else:
                        errors_0_to_1 += 1

                if(original_bit == 1 and bit_duration > max_duration_bit_1):
                    max_duration_bit_1 = bit_duration
                    # First time both the variables are initialized by bit_duration.
                    if(min_duration_bit_1 == 0):
                        min_duration_bit_1 = bit_duration
                elif(original_bit == 1 and bit_duration < min_duration_bit_1):
                    min_duration_bit_1 = bit_duration
                elif(original_bit == 0 and bit_duration > max_duration_bit_0 ):
                    max_duration_bit_0 = bit_duration
                    # First time both the variables are initialized by bit_duration.
                    if(min_duration_bit_0 == 0):
                        min_duration_bit_0 = bit_duration
                elif(original_bit == 0 and (bit_duration < min_duration_bit_0)):
                    min_duration_bit_0 = bit_duration
             
                #i+=1
                err+=1

    per_string_data[str_num] = [errors, errors_0_to_1, errors_1_to_0, max_duration_bit_1, min_duration_bit_1, max_duration_bit_0, min_duration_bit_0, max_duration_algo1, min_duration_algo1, max_duration_algo2, min_duration_algo2]

def read_file_line_by_line(file_path, string_number, TH, atp, string_len, equivalent_num_uf_chunks_to_avoid, prcnt_bits_with_non_probe_arr_prcnt):
   ## Extract bit duration.
    pattern = r"cpu 0 is on wait,"
    cpus_wait_cycle_line_num=[]
    per_bit_data={}
    #print("file_path: "+file_path)
    with open(file_path, "r") as file:
        for line_number, line in enumerate(file, start=1):
            if re.search(pattern, line):
                #print(f"Pattern found in line {line_number}: {line.strip()}")
                cpus_wait_cycle_line_num.append(line_number)
    #print(cpus_wait_cycle_line_num)
    #exit(0)

    ## Read the actual message.
    original_msg=[]   ## Original message after including the marker bits.
    original_msg1=[]  ##Intermediate message read from benchmark_<test/train>.txt
    MAX_INC_IN_ARR=int(string_len/(atp-1))
    NUM_BITS_WITH_MARKER_BITS= int(string_len + (2*MAX_INC_IN_ARR))
    print("NUM_BITS_WITH_MARKER_BITS: ",NUM_BITS_WITH_MARKER_BITS)
    if train_data == 1:
        with open('benchmark/benchmark_train.txt', "r") as file:
            for line_number, line in enumerate(file, start=1):
                if(line_number == string_number):
                    line = line.strip()
                    # Split the string into single characters and assign to an array
                    original_msg1 = [char for char in line]
                    #print(original_msg1,' ',line_number)
    #print("length of org_msg1 ", len(original_msg1))
    if train_data == 0:
        with open('benchmark/benchmark_test.txt', "r") as file:
            for line_number, line in enumerate(file, start=1):
                if(line_number == string_number):
                    line = line.strip()
                    # Split the string into single characters and assign to an array
                    original_msg1 = [char for char in line]
                   # print(original_msg,' ',line_number)
    err=0
    i=0
    for num_bits in range(0,NUM_BITS_WITH_MARKER_BITS):
        if(err+1) % atp == 0:
            original_msg.append(1)
            err+=1
        elif((err+1) % atp == 1 and err > 1):
            original_msg.append(0)
            err = 0
        else:
            #print("2. length of org_msg1 ", len(original_msg1)," i is: ",i," err: ",err, " atp: ",atp," num_bits: ",num_bits)
            original_msg.append(original_msg1[i]) 
            i+=1
            err+=1
    #print(len(original_msg))
    #print(original_msg)
	#exit(0)


    ## Identify the received message and error.    
    received_msg=[]
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
    with open(file_path, "r") as file:
        for line_number, line in enumerate(file, start=1):
            #print("line is: ",line)
            line=line.strip()
            if re.search(pattern, line):
                match = re.search(r'bit received is: (\d+)', line)
                if match:
                    received_msg.append(match.group(1))
                else:
                    print(line)
                    print("Number not found in the string.", line_number)
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

    for i in range(0, NUM_BITS_WITH_MARKER_BITS):
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

    #Check the errors in transmission and correct them if these were due to wrong inference of receiver while probing the unprobed array in the next bit.
    for i in range(1,NUM_BITS_WITH_MARKER_BITS+1):
        if (int(original_msg[i-1]) != int(received_msg[i-1])) and (int(received_msg[i-1]) == 1 ):   # if the error is of type 0->1, it is corrected here.
            is_it_an_error = verify_error(file_path, (i-1), equivalent_num_uf_chunks_to_avoid)
            if(int(is_it_an_error) == 0):
                received_msg[i-1] = 0 #correct err, it is due to wrong inference as receiver times unprobed arr access. Case of err in bit, next to bit with unprobed accesses.
        per_bit_data[int(i)] = [original_msg[i-1], received_msg[i-1], bit_interval[i-1]]


#    #Check the errors in transmission and correct them if these were due to wrong inference of receiver while probing the unprobed array.
#    if(prcnt_bits_with_non_probe_arr_prcnt == 2):
#        for i in range(1,NUM_BITS_WITH_MARKER_BITS+1):
#            if(i == 52 or i == 103 or i == 181 or i == 232 or i == 310 or i == 361 or i == 439 or i == 490) and int(received_msg[i]) == 1:
#                verify_1to0_error(file_path, i, equivalent_num_uf_chunks_to_avoid)
#    elif(prcnt_bits_with_non_probe_arr_prcnt == 1):
#        for i in range(1,NUM_BITS_WITH_MARKER_BITS+1):
#            if(i == 103 or i == 232 or i == 361 or i == 490) and int(received_msg[i]) == 1:
#                verify_1to0_error(file_path, i, equivalent_num_uf_chunks_to_avoid)
#    elif(prcnt_bits_with_non_probe_arr_prcnt == 6):
#        for i in range(1,NUM_BITS_WITH_MARKER_BITS+1):
#            if int(original_msg[i-1]) != int(received_msg[i-1]) and int(received_msg[i]) == 1:
#                is_it_an_error = verify_1to0_error(file_path, i, equivalent_num_uf_chunks_to_avoid)
#
#                if(int(is_it_an_error) == 0):
#                    print("Something is wrong.")
#                    print("i is error position: ",i)
#                    #exit(0)
#    else:
#        print("Invalid input is provided.")
#        exit(0)

    df = pd.DataFrame.from_dict(per_bit_data, orient='index', columns=['original_bit','received_bit','bit_interval'])
    # Rename the index column
    df = df.rename_axis('bit_position')
    df.to_csv(data_file_path, index=True)
    #print("written_to_file")


############### Programs starting point. ######################
# Check if no command-line arguments are provided
if len(sys.argv) != 4:
    print("Two commandline arguments are expected")
    print(" i.e 1 if the training_dataset results are processed or 0 if the testing_dataset results are processed. Exiting...")
    print("Second argument is for non_probe_arr_prcnt.")
    print("Third argument is for prcnt_bits_with_non_probe_arr_prcnt.")
    sys.exit(1)

if int(sys.argv[1]) > 1 or int(sys.argv[1]) < 0:
    print("The value should be either 0 or 1.")
    sys.exit(1)

if int(sys.argv[2]) != 97 and int(sys.argv[2]) != 95 and int(sys.argv[2]) != 92 and int(sys.argv[2]) != 86:
    print("The value should be 86, 92, 95 or 97.")
    sys.exit(1)

#if int(sys.argv[3]) != 1 and int(sys.argv[3]) != 2 and int(sys.argv[3]) != 6:
#    print("The value should be 1, 2, or 6")
#    sys.exit(1)

if int(sys.argv[2]) == 97:
    equivalent_num_uf_chunks_to_avoid=1964
elif int(sys.argv[2]) == 95:
    equivalent_num_uf_chunks_to_avoid=1923
elif int(sys.argv[2]) == 92:
    equivalent_num_uf_chunks_to_avoid=1863
elif int(sys.argv[2]) == 86:
    equivalent_num_uf_chunks_to_avoid=1741

#sys.exit(1)

# Access command-line argument.
train_data=int(sys.argv[1])
non_probe_arr_prcnt=int(sys.argv[2])
prcnt_bits_with_non_probe_arr_prcnt=int(sys.argv[3])

UFs=[16]
Algo_trigger_point=[128]
string_len=512
err_corr_iterations=12  #Maximum number of times error-correction can be repeated.

# Get the current working directory
cwd = os.getcwd()

# Define the directory name
directory_name = 'extracted_results_'+str(err_corr_iterations)+'_'+str(non_probe_arr_prcnt)+'_'+str(prcnt_bits_with_non_probe_arr_prcnt)+'_error_analysis'

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
#directory_path='../sender_disturb_4_LLC_blocks_unoccupied_llc_point_1_percent'
#directory_path='../champsim_results'
#directory_path='../results'
directory_path='../LR_ERR_CORR_2core_results_both_algo_12_access_'+str(non_probe_arr_prcnt)+'_'+str(prcnt_bits_with_non_probe_arr_prcnt)+'_1'

res_dir_path= 'extracted_results_'+str(err_corr_iterations)+'_'+str(non_probe_arr_prcnt)+'_'+str(prcnt_bits_with_non_probe_arr_prcnt)+'_error_analysis'

# Get a list of all files in the specified directory
file_list = os.listdir(directory_path)

for UF in UFs:
    for atp in Algo_trigger_point:
        #other_blocks=31
        other_blocks=326
        if atp == 8 or atp == 16 or atp == 32:
            err_corr_iterations=3  #3  #Maximum number of times error-correction can be repeated.
        elif atp == 64:
            err_corr_iterations=4  #Maximum number of times error-correction can be repeated.
        elif atp == 128:
            err_corr_iterations=12  #Maximum number of times error-correction can be repeated.
        elif atp == 256:
            err_corr_iterations=5  #5  #Maximum number of times error-correction can be repeated.
        elif atp == 512:
            err_corr_iterations=9  #Maximum number of times error-correction can be repeated.
        elif atp == 514:
            err_corr_iterations=300  #Maximum number of times error-correction can be repeated.
            
        print('processing for UF: ',UF,' Algo_trigger_point: ',atp)
        string='multiple_access_'+str(UF)+'_other_blocks_'+str(other_blocks)+'_'
        string2='_'+str(atp)+'_with_both_algo_'+str(err_corr_iterations)+'_access_train.gz.txt'
        string3='_'+str(atp)+'_with_both_algo_'+str(err_corr_iterations)+'_access_test.gz.txt'
        if(UF == 32):
            TH = 134
        elif(UF == 16):
            TH = 106
        elif(UF == 8):
            TH = 101
        elif(UF == 4):
            TH = 98 
        elif(UF == 2):
            TH = 94 
        elif(UF == 1):
            TH = 85 

	#Declaring empty dictionary to store data per message strings that are communicated.
        per_string_data={}
        if train_data == 1:
            tot_err_cnt_file='total_error_count_UF_'+str(UF)+'_'+str(atp)+'_train_with_err_corr_both_algo_'+str(err_corr_iterations)+'_access_other_blocks_'+str(other_blocks)+'_'+str(non_probe_arr_prcnt)+'_'+str(prcnt_bits_with_non_probe_arr_prcnt)+'.csv'
            # Filter files that contain the specified string
            filtered_files = [file for file in file_list if string in file and string2 in file]
        else:
            tot_err_cnt_file='total_error_count_UF_'+str(UF)+'_'+str(atp)+'_test_with_err_corr_both_algo_'+str(err_corr_iterations)+'_access_other_blocks_'+str(other_blocks)+'_'+str(non_probe_arr_prcnt)+'_'+str(prcnt_bits_with_non_probe_arr_prcnt)+'.csv'
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
            else:
                print("Number not found in the string.")
                exit(0)

            #exit(0)

            #if int(string_number) == 15 and int(atp) == 8:
            #    continue
            #elif (int(string_number) == 50 or int(string_number) == 7 or int(string_number) == 48 or int(string_number) == 15 or int(string_number) == 20 or int(string_number) == 32) and int(atp) == 128:
            #    continue
            #elif (int(string_number) == 15 or int(string_number) == 32 or int(string_number) == 2) and int(atp) == 64:
            #    continue
            #elif int(string_number) == 50 and int(atp) == 8:
            #    continue
            #elif int(string_number) == 20 and int(atp) == 32:
            #    continue
            #elif (int(string_number) == 48 or int(string_number) == 15 or int(string_number) == 32) and int(atp) == 64:
            #    continue
            #elif (int(string_number) == 48 or int(string_number) == 20 or int(string_number) == 32) and int(atp) == 128:
            #    continue
            #elif int(string_number) == 15 and int(atp) == 16:
            #    continue
            #elif int(string_number) == 50 and int(atp) == 32:
            #    continue


            if train_data == 1:
                data_file='per_string_data_strnum_'+str(string_number)+'_UF_'+str(UF)+'_'+str(atp)+'_train_with_err_corr_both_algo_'+str(err_corr_iterations)+'_access_other_blocks_'+str(other_blocks)+'_'+str(non_probe_arr_prcnt)+'_'+str(prcnt_bits_with_non_probe_arr_prcnt)+'.csv'
            else:
                data_file='per_string_data_strnum_'+str(string_number)+'_UF_'+str(UF)+'_'+str(atp)+'_test_with_err_corr_both_algo_'+str(err_corr_iterations)+'_access_other_blocks_'+str(other_blocks)+'_'+str(non_probe_arr_prcnt)+'_'+str(prcnt_bits_with_non_probe_arr_prcnt)+'.csv'
            data_file_path = os.path.join(res_dir_path, data_file)
            print(file_path,"count: ",count,"UF: ",UF)
            read_file_line_by_line(file_path, string_number, TH, atp, string_len, equivalent_num_uf_chunks_to_avoid, prcnt_bits_with_non_probe_arr_prcnt)
            # This script generate total errors and min-max bit duration.
            generate_total_error_count(data_file_path)
            #exit(0)
            count+=1
            ### XXX Verification check.
            #if(count == 2):
            #    break


        # Sort the dictionary.
        sorted_items = sorted(per_string_data.items())

        # Convert the sorted items back to a dictionary
        per_string_data = dict(sorted_items)

        # Create a DataFrame
        df = pd.DataFrame.from_dict(per_string_data, orient='index', columns=['Total_error','errors_0_to_1','errors_1_to_0','Maximum_duration_1','Minimum_duration_1','Maximum_duration_0','Minimum_duration_0','Maximum_duration_algo1','Minimum_duration_algo1','Maximum_duration_algo2','Minimum_duration_algo2'])
        # Rename the index column
        df = df.rename_axis('Msg_string_num')

        # Write to csv file.
        tot_err_cnt_file_path = os.path.join(res_dir_path, tot_err_cnt_file)
        df.to_csv(tot_err_cnt_file_path, index=True)

#exit(0)

### Calculating total errors and other stats. ###
cpu_clock_cycles=4000000000

for UF in UFs:
    for atp in Algo_trigger_point:
        #other_blocks=31
        other_blocks=326
        if atp == 8 or atp == 16 or atp == 32:
            err_corr_iterations=3  #3  #Maximum number of times error-correction can be repeated.
        elif atp == 64:
            err_corr_iterations=4  #Maximum number of times error-correction can be repeated.
        elif atp == 128:
            err_corr_iterations=12  #5  #Maximum number of times error-correction can be repeated.
        elif atp == 256:
            err_corr_iterations=5  #5  #Maximum number of times error-correction can be repeated.
        elif atp == 512:
            err_corr_iterations=9  #Maximum number of times error-correction can be repeated.
        elif atp == 514:
            err_corr_iterations=300  #Maximum number of times error-correction can be repeated.
        #if atp == 32:
        #    other_blocks = 327
        #else:
        #    other_blocks = 326
        if train_data == 1:
            tot_err_cnt_file='total_error_count_UF_'+str(UF)+'_'+str(atp)+'_train_with_err_corr_both_algo_'+str(err_corr_iterations)+'_access_other_blocks_'+str(other_blocks)+'_'+str(non_probe_arr_prcnt)+'_'+str(prcnt_bits_with_non_probe_arr_prcnt)+'.csv'
        else:
            tot_err_cnt_file='total_error_count_UF_'+str(UF)+'_'+str(atp)+'_test_with_err_corr_both_algo_'+str(err_corr_iterations)+'_access_other_blocks_'+str(other_blocks)+'_'+str(non_probe_arr_prcnt)+'_'+str(prcnt_bits_with_non_probe_arr_prcnt)+'.csv'
        tot_err_cnt_file_path = os.path.join(res_dir_path, tot_err_cnt_file)
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
            min_algo1 = 0
            max_algo1 = 0
            min_algo2 = 0
            max_algo2 = 0
            #hole_was_created = 0
            #hole_filled_up = 0
            #receiver_inactive_active_block_is_evicted = 0
            #sender_evicted_its_own_block = 0
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
                    max_algo1 = int(line[8])
                    min_algo1 = int(line[9])
                    max_algo2 = int(line[10])
                    min_algo2 = int(line[11])
                    #print(" line_number: ",line_number," line[4]: ",line[4]," line[5]: ",line[5]," line[6]: ",line[6]," line[7]: ",line[7]," line[8]: ",line[8]," line[9]: ",line[9]," line[10]: ",line[10]," line[11]: ",line[11])
                else:
                    if(int(line[4]) > max_dur_1):
                        max_dur_1 = int(line[4])
                    if(int(line[5]) < int(min_dur_1)):
                        min_dur_1 = int(line[5])
                    if(int(line[6]) > int(max_dur_0)):
                        max_dur_0 = int(line[6])
                    if(int(line[7]) < int(min_dur_0)):
                        min_dur_0 = int(line[7])
                    if(int(line[8]) > int(max_algo1)):
                        max_algo1 = int(line[8])
                    if(int(line[9]) < int(min_algo1)):
                        min_algo1 = int(line[9])
                    if(int(line[10]) > int(max_algo2)):
                        max_algo2 = int(line[10])
                    if(int(line[11]) < int(min_algo2)):
                        min_algo2 = int(line[11])
                    #if(min_algo2 == 0 or min_algo1 == 0 or min_dur_0 == 0 or min_dur_1 == 0):
                        #print("Something is wrong."," line_number: ",line_number," line[4]: ",line[4]," line[5]: ",line[5]," line[6]: ",line[6]," line[7]: ",line[7]," line[8]: ",line[8]," line[9]: ",line[9]," line[10]: ",line[10]," line[11]: ",line[11])
                        #exit(1)
            text_file='Total_result_'+str(UF)+'_train_'+str(train_data)+'_'+str(atp)+'_with_err_corr_both_algo_'+str(err_corr_iterations)+'_access_other_blocks_'+str(other_blocks)+'_'+str(non_probe_arr_prcnt)+'_'+str(prcnt_bits_with_non_probe_arr_prcnt)+'_reverified.txt'
            with open(text_file, 'w') as txtfile:
                ### Calculating bandwdith with both algorithms.
                cycles_to_send_one_bit=(max_dur_1*(int(atp)-1)+max_algo1+max_algo2)/(atp-1)
                cycles_to_send_one_bit=round(cycles_to_send_one_bit,2)
                bandwidth_with_both_algo=(cpu_clock_cycles/cycles_to_send_one_bit)/1024
                bandwidth_with_both_algo=round(bandwidth_with_both_algo,2)
                ### Calculating bandwdith with hole-filling only.
                cycles_to_send_one_bit=(max_dur_1*(int(atp)-1)+max_algo1)/(atp-1)
                cycles_to_send_one_bit=round(cycles_to_send_one_bit,2)
                bandwidth_with_hole_filling_only=(cpu_clock_cycles/cycles_to_send_one_bit)/1024
                bandwidth_with_hole_filling_only=round(bandwidth_with_hole_filling_only,2)
                ### Calculating estimated bandwidth with hole-filling and llc_occ_chk.
                cycles_to_send_one_bit=(max_dur_1*(int(atp)-1)+2*max_algo1)/(atp-1)
                cycles_to_send_one_bit=round(cycles_to_send_one_bit,2)
                estimated_bandwidth_with_both_algo=(cpu_clock_cycles/cycles_to_send_one_bit)/1024
                estimated_bandwidth_with_both_algo=round(estimated_bandwidth_with_both_algo,2)
                txtfile.write(f'Total_error: {total_error_count} errors_0_to_1: {total_err_0_to_1} errors_1_to_0: {total_err_1_to_0} bandwidth_with_both_algo: {bandwidth_with_both_algo}Kbps bandwidth_with_hole_filling_only: {bandwidth_with_hole_filling_only}Kbps estimated_bandwidth_with_both_algo: {estimated_bandwidth_with_both_algo}Kbps Maximum_duration_1: {max_dur_1} Minimum_duration_1: {min_dur_1} Maximum_duration_0: {max_dur_0} Minimum_duration_0: {min_dur_0} Maximum_duration_algo1: {max_algo1} Minimum_duration_algo1: {min_algo1} Maximum_duration_algo2: {max_algo2} Minimum_duration_algo2: {min_algo2} \n')

