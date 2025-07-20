import os
import re
import sys
import pandas as pd
import math

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

def read_file_line_by_line(file_path, string_number, TH, atp, string_len):
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
   #Calculate the errors in transmission.
    counta=0
    for i in range(1,NUM_BITS_WITH_MARKER_BITS+1):
        per_bit_data[int(i)] = [original_msg[i-1], received_msg[i-1], bit_interval[i-1]]
        if(int(original_msg[i-1]) != int(received_msg[i-1])):
            print("i is error position: ",i," original bit: ",int(original_msg[i-1])," received bit: ", int(received_msg[i-1]))
            counta+=1
    #print('counta: ',counta)
    #exit(0)

    df = pd.DataFrame.from_dict(per_bit_data, orient='index', columns=['original_bit','received_bit','bit_interval'])
    # Rename the index column
    df = df.rename_axis('bit_position')
    df.to_csv(data_file_path, index=True)
    #print("written_to_file")


############### Programs starting point. ######################
# Check if no command-line arguments are provided
if len(sys.argv) != 4:
    print("First commandline argument is 1 if the training_dataset results are processed or 0 if the testing_dataset results are processed.")
    print("Second commandline argument is Unrollingfactor value.")
    print("Third commandline argument is error_correction_iterations. Exiting...")
    sys.exit(1)

if int(sys.argv[1]) > 1 or int(sys.argv[1]) < 0:
    print("The value of first argument should be either 0 or 1.")
    sys.exit(1)

# Access command-line argument.
train_data=int(sys.argv[1])

#UFs=[32, 64, 128, 256]
#UFs=[32, 64, 128]
UFs=[]
UFs.append(int(sys.argv[2]))
err_corr_iterations=(int(sys.argv[3]))
#Algo_trigger_point=[128, 64, 32, 16, 8]
Algo_trigger_point=[128]
#Algo_trigger_point=[64, 128]
string_len=512

string_number=0
per_string_data={}

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

res_dir_path='extracted_results'
directory_path='../champsim_results'
file_list = os.listdir(directory_path)

# Get a list of all files in the specified directory

for UF in UFs:
    for atp in Algo_trigger_point:
        other_blocks=326
        #if UF == 256:
        #    err_corr_iterations=30 #Maximum number of times error-correction can be repeated.
        #elif UF == 64 or UF == 128:
        #    err_corr_iterations=13 #Maximum number of times error-correction can be repeated.
        #elif UF == 32:
        #    err_corr_iterations=12 #Maximum number of times error-correction can be repeated.
            
        print('processing for UF: ',UF,' Algo_trigger_point: ',atp)
        string='multiple_access_'+str(UF)+'_other_blocks_'+str(other_blocks)+'_'
        string2='_'+str(atp)+'_with_both_algo_'+str(err_corr_iterations)+'_access_train.gz.txt'
        string3='_'+str(atp)+'_with_both_algo_'+str(err_corr_iterations)+'_access_test.gz.txt'
        if(UF == 32):
            TH = 134
        elif(UF == 64):
            TH = 250
        elif(UF == 128):
            TH = 340
        elif(UF == 256):
            TH = 340
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
            tot_err_cnt_file='total_error_count_UF_'+str(UF)+'_'+str(atp)+'_train_with_err_corr_both_algo_'+str(err_corr_iterations)+'_access_other_blocks_'+str(other_blocks)+'.csv'
            # Filter files that contain the specified string
            filtered_files = [file for file in file_list if string in file and string2 in file]
        else:
            tot_err_cnt_file='total_error_count_UF_'+str(UF)+'_'+str(atp)+'_test_with_err_corr_both_algo_'+str(err_corr_iterations)+'_access_other_blocks_'+str(other_blocks)+'.csv'
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


            if (int(string_number) == 83 or int(string_number) == 378) and int(UF) == 256 and int(err_corr_iterations) == 18:
                continue
            elif (int(string_number) == 71 or int(string_number) == 487 or int(string_number) == 346) and int(UF) == 256 and int(err_corr_iterations) == 19:
                continue
            #elif int(string_number) == 12 and int(atp) == 64:
            #    continue
            #elif int(string_number) == 162 and int(atp) == 128:
            #    continue


            if train_data == 1:
                data_file='per_string_data_strnum_'+str(string_number)+'_UF_'+str(UF)+'_'+str(atp)+'_train_with_err_corr_both_algo_'+str(err_corr_iterations)+'_access_other_blocks_'+str(other_blocks)+'.csv'
            else:
                data_file='per_string_data_strnum_'+str(string_number)+'_UF_'+str(UF)+'_'+str(atp)+'_test_with_err_corr_both_algo_'+str(err_corr_iterations)+'_access_other_blocks_'+str(other_blocks)+'.csv'
            data_file_path = os.path.join(res_dir_path, data_file)
            print(file_path,"count: ",count,"UF: ",UF)
            read_file_line_by_line(file_path, string_number, TH, atp, string_len)
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

### Calculating total errors and other stats. ###
cpu_clock_cycles=4000000000

for UF in UFs:
    for atp in Algo_trigger_point:
        other_blocks=326
        #if atp == 32:
        #    other_blocks = 327
        #else:
        #    other_blocks = 326
        #if UF == 256:
        #    err_corr_iterations=30 #Maximum number of times error-correction can be repeated.
        #elif UF == 64 or UF == 128:
        #    err_corr_iterations=13 #Maximum number of times error-correction can be repeated.
        #elif UF == 32:
        #    err_corr_iterations=12 #Maximum number of times error-correction can be repeated.

        if train_data == 1:
            tot_err_cnt_file='total_error_count_UF_'+str(UF)+'_'+str(atp)+'_train_with_err_corr_both_algo_'+str(err_corr_iterations)+'_access_other_blocks_'+str(other_blocks)+'.csv'
        else:
            tot_err_cnt_file='total_error_count_UF_'+str(UF)+'_'+str(atp)+'_test_with_err_corr_both_algo_'+str(err_corr_iterations)+'_access_other_blocks_'+str(other_blocks)+'.csv'
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
            text_file='Total_result_'+str(UF)+'_train_'+str(train_data)+'_'+str(atp)+'_with_err_corr_both_algo_'+str(err_corr_iterations)+'_access_other_blocks_'+str(other_blocks)+'.txt'
            with open(text_file, 'w') as txtfile:
                ### Calculating bandwdith with both algorithms.
                cycles_to_send_one_bit=(max_dur_1*(int(atp)-1)+max_algo1+max_algo2)/(atp-1)
                cycles_to_send_one_bit=math.ceil(cycles_to_send_one_bit)
                bandwidth_with_both_algo=(cpu_clock_cycles/cycles_to_send_one_bit)/1024
                bandwidth_with_both_algo=round(bandwidth_with_both_algo,2)

                ### Calculating bandwdith with hole-filling only.
                cycles_to_send_one_bit=(max_dur_1*(int(atp)-1)+max_algo1)/(atp-1)
                cycles_to_send_one_bit=math.ceil(cycles_to_send_one_bit)
                bandwidth_with_hole_filling_only=(cpu_clock_cycles/cycles_to_send_one_bit)/1024
                bandwidth_with_hole_filling_only=round(bandwidth_with_hole_filling_only,2)

                ### Calculating estimated bandwidth with hole-filling and llc_occ_chk.
                cycles_to_send_one_bit=(max_dur_1*(int(atp)-1)+2*max_algo1)/(atp-1)
                cycles_to_send_one_bit=math.ceil(cycles_to_send_one_bit)
                estimated_bandwidth_with_both_algo=(cpu_clock_cycles/cycles_to_send_one_bit)/1024
                estimated_bandwidth_with_both_algo=round(estimated_bandwidth_with_both_algo,2)
                txtfile.write(f'Total_error: {total_error_count} errors_0_to_1: {total_err_0_to_1} errors_1_to_0: {total_err_1_to_0} bandwidth_with_both_algo: {bandwidth_with_both_algo}Kbps bandwidth_with_hole_filling_only: {bandwidth_with_hole_filling_only}Kbps estimated_bandwidth_with_both_algo: {estimated_bandwidth_with_both_algo}Kbps Maximum_duration_1: {max_dur_1} Minimum_duration_1: {min_dur_1} Maximum_duration_0: {max_dur_0} Minimum_duration_0: {min_dur_0} Maximum_duration_algo1: {max_algo1} Minimum_duration_algo1: {min_algo1} Maximum_duration_algo2: {max_algo2} Minimum_duration_algo2: {min_algo2} \n')
