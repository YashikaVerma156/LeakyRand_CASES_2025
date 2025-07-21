import os
import re
import pandas as pd


def read_file_line_by_line(file_path, UF, UF_ERR_CORR, atp_num):
    miss_is_seen=0
    row_buffer_hit_is_seen=0
    setup_complete=0
    instr_id=0
    id1=0
    id2=0
    bit_received=0
    pattern1='found a miss for CPU: 0'
    pattern2='row buffer hit'
    pattern3='RDTSC, sub:'
    pattern4='bit received is'

    # Open the file in read mode ('r')
    with open(file_path, 'r') as file:
        # Iterate over each line in the file
        for line in file:
            line=line.strip()    #Remove leading and trailing whitespaces.

            #Skip everything till the setup is not complete.
            if "=========setup is done========" in line:
                setup_complete=1
            if setup_complete == 0:
                continue

            
            if pattern4 in line:
                bit_received+=1
                if((bit_received) % atp_num == 1 and bit_received > 1):
                    bit_received=0
            if pattern1 in line:
                miss_is_seen=1
                # Search for instr_id using regular expression
                match = re.search(r'instr_id: (\d+)', line)

                # Extract instr_id if found
                if match:
                    instr_id = int(match.group(1))
                    #print("instr_id:", instr_id)
                else:
                    print("instr_id not found")
                    exit()
           
            if pattern2 in line and miss_is_seen == 1:
                row_buffer_hit_is_seen=1

            if pattern3 in line:
                # Extract latency
                latency_pattern = r'sub: (\d+)'
                match = re.search(latency_pattern, line)
                # If a match is found, extract the number
                if match:
                    latency_key = int(match.group(1))

                # Search for the pattern using regular expression
                match = re.search(r'id:\s*(\d+)\s*-\s*(\d+)', line)

                # Extract id1 and id2 if found
                if match:
                    id1 = int(match.group(1))
                    id2 = int(match.group(2))
                else:
                    print("IDs not found")
                    exit()
                
                # Assign to corresponding dictionary
                if miss_is_seen == 1 and id1 != 0 and id2 != 0 and id2 - instr_id >= 2 and instr_id >= id1 and id2-id1 > int(UF_ERR_CORR)*2:

                    # These are overlapping latency numbers between the hit and miss latencies. To be verified once.
                    if(latency_key == 110 and UF == 32):
                        print(line,' UF: ' ,UF)
                        print(file_path)
                    if(latency_key == 114 and UF == 32):
                        print(line,' UF: ' ,UF)
                        print(file_path)
                    if(latency_key == 124 and UF == 32):
                        print(line,' UF: ' ,UF)
                        print(file_path)
                    if((latency_key == 87 or latency_key == 69 or latency_key == 109 or latency_key == 118) and UF == 16):
                        print(line,' UF: ' ,UF)
                        print(file_path)
                    if(latency_key == 76 and UF == 8):
                        print(line,' UF: ' ,UF)
                        print(file_path)
                    if(latency_key == 71 and UF == 4):
                        print(line,' UF: ' ,UF)
                        print(file_path)
                    if(latency_key == 69 and UF == 2):
                        print(line,' UF: ' ,UF)
                        print(file_path)
                    if(latency_key == 57 or latency_key == 58 and UF == 1):
                        print(line,' UF: ' ,UF)
                        print(file_path)

                    #Latencies are collected for marker bits.
                    if(bit_received+1) % atp_num == 0:
                        if latency_key in miss_latencies_data_err_corr:
                             miss_latencies_data_err_corr[latency_key][0] += miss_is_seen
                             miss_latencies_data_err_corr[latency_key][1] += row_buffer_hit_is_seen
                        else:
                             # Key is not present, add a new key-value pair
                             miss_latencies_data_err_corr[latency_key] = [miss_is_seen, row_buffer_hit_is_seen]

                    #Latencies are collected for marker bits.
                    elif((bit_received+1) % atp_num == 1 and bit_received > 1):
                        if latency_key in miss_latencies_data_err_corr:
                             miss_latencies_data_err_corr[latency_key][0] += miss_is_seen
                             miss_latencies_data_err_corr[latency_key][1] += row_buffer_hit_is_seen
                        else:
                             # Key is not present, add a new key-value pair
                             miss_latencies_data_err_corr[latency_key] = [miss_is_seen, row_buffer_hit_is_seen]

                    #Latencies are collected for data bits.
                    else:
                        if latency_key in miss_latencies_data:
                             miss_latencies_data[latency_key][0] += miss_is_seen
                             miss_latencies_data[latency_key][1] += row_buffer_hit_is_seen
                        else:
                             # Key is not present, add a new key-value pair
                             miss_latencies_data[latency_key] = [miss_is_seen, row_buffer_hit_is_seen]
                    miss_is_seen=0
                    row_buffer_hit_is_seen=0
                else:
                    #Latencies are collected for marker bits.
                    if(bit_received+1) % atp_num == 0:
                        if latency_key in hit_latencies_data_err_corr:
                            hit_latencies_data_err_corr[latency_key] += 1
                        else:
                            # Key is not present, add a new key-value pair
                            hit_latencies_data_err_corr[latency_key] = 1
                    #Latencies are collected for marker bits.
                    elif((bit_received+1) % atp_num == 1 and bit_received > 1):
                        if latency_key in hit_latencies_data_err_corr:
                            hit_latencies_data_err_corr[latency_key] += 1
                        else:
                            # Key is not present, add a new key-value pair
                            hit_latencies_data_err_corr[latency_key] = 1
                    #Latencies are collected for data bits.
                    else:
                        if latency_key == 628 and UF == 256:
                            print(file_path)
                        if latency_key in hit_latencies_data:
                            hit_latencies_data[latency_key] += 1
                        else:
                            # Key is not present, add a new key-value pair
                            hit_latencies_data[latency_key] = 1
                latency_key = 0
                instr_id=0
                id1=0
                id2=0

UFs=[256]
#UFs=[32, 64, 128, 256]
#Algo_trigger_point=[8,16,32,64,128]
Algo_trigger_point=[128]
#benchmark=["test","train"] 
benchmark=["train"] 
msg_string_len=[512]
#other_blocks=["31","32"]  # 32 in other blocks signify 326 or 327
other_blocks=["326"]  # 32 in other blocks signify 326 or 327

UF_ERR_CORR=16

#Declaring empty dictionary
hit_latencies_data={}
miss_latencies_data={}
hit_latencies_data_err_corr={}
miss_latencies_data_err_corr={}

#directory_path='../LR_ERR_CORR_2core_results_both_algo_4_access'
# Get a list of all files in the specified directory
#file_list = os.listdir(directory_path)

for ob in other_blocks:
    for UF in UFs:
        for msl in msg_string_len:
            for atp in Algo_trigger_point:
                for bench in benchmark:
                    print('processing for ob: ',ob,' UF: ',UF,' msl: ',msl,' atp: ',atp,' benchmark: ',bench)
                    string1='receiver_multiple_access_'+str(UF)+'_other_blocks_'+str(ob)
                    if bench == "train":
                        string2='_train.gz.txt'
                        miss_file='miss_data_UF_'+str(ob)+'_'+str(UF)+'_'+str(msl)+'_'+str(atp)+'_train.csv'
                        hit_file='hit_data_UF_'+str(ob)+'_'+str(UF)+'_'+str(msl)+'_'+str(atp)+'_train.csv'
                        miss_file_err_corr='miss_data_UF_'+str(ob)+'_'+str(UF)+'_'+str(msl)+'_'+str(atp)+'_train_err_corr.csv'
                        hit_file_err_corr='hit_data_UF_'+str(ob)+'_'+str(UF)+'_'+str(msl)+'_'+str(atp)+'_train_err_corr.csv'
                    if bench == "test":
                        string2='_test.gz.txt'
                        miss_file='miss_data_UF_'+str(ob)+'_'+str(UF)+'_'+str(msl)+'_'+str(atp)+'_test.csv'
                        hit_file='hit_data_UF_'+str(ob)+'_'+str(UF)+'_'+str(msl)+'_'+str(atp)+'_test.csv'
                        miss_file_err_corr='miss_data_UF_'+str(ob)+'_'+str(UF)+'_'+str(msl)+'_'+str(atp)+'_test_err_corr.csv'
                        hit_file_err_corr='hit_data_UF_'+str(ob)+'_'+str(UF)+'_'+str(msl)+'_'+str(atp)+'_test_err_corr.csv'
                    string3='_'+str(msl)+'_'+str(atp)+'_with_both_algo'
                    if UF == 256:
                        directory_path='../LR_ERR_CORR_2core_results_both_algo_18_access'
                        # Get a list of all files in the specified directory
                        file_list = os.listdir(directory_path)
                    else:
                        directory_path='../LR_ERR_CORR_2core_results_both_algo_12_access'
                        # Get a list of all files in the specified directory
                        file_list = os.listdir(directory_path)

                    # Filter files that contain the specified string
                    filtered_files = [file for file in file_list if string1 in file and string2 in file and string3 in file]
                    hit_latencies_data={}
                    miss_latencies_data={}
                    hit_latencies_data_err_corr={}
                    miss_latencies_data_err_corr={}
                    print('size of hit_latencies_data_err_corr dict: ', len(hit_latencies_data_err_corr),' size of miss_latencies_data_err_corr dict: ', len(miss_latencies_data_err_corr),' size of hit_latencies_data dict: ', len(hit_latencies_data),' size of miss_latencies_data dict: ', len(miss_latencies_data),' number of files: ',len(filtered_files))
                    if(len(filtered_files) == 0):
                        continue

                    i=0
                    # Read the content of each file
                    for file_name in filtered_files:
                        file_path = os.path.join(directory_path, file_name)
                        i=i+1
                        # Call the function with the file path
                        read_file_line_by_line(file_path, UF, UF_ERR_CORR, atp)
                        print(i)

                    #print(miss_latencies_data)
                    sorted_items = sorted(miss_latencies_data.items())
                    sorted_items1 = sorted(hit_latencies_data.items())
                    sorted_items_err_corr = sorted(miss_latencies_data_err_corr.items())
                    sorted_items1_err_corr = sorted(hit_latencies_data_err_corr.items())

                    # Convert the sorted items back to a dictionary
                    miss_latencies_data = dict(sorted_items)
                    hit_latencies_data = dict(sorted_items1)
                    miss_latencies_data_err_corr = dict(sorted_items_err_corr)
                    hit_latencies_data_err_corr = dict(sorted_items1_err_corr)


                    # Create a DataFrame
                    df1 = pd.DataFrame.from_dict(miss_latencies_data, orient='index', columns=['Frequency', 'Row_buffer_hit_freq'])
                    df2 = pd.DataFrame.from_dict(hit_latencies_data, orient='index', columns=['Frequency'])
                    df1_err_corr= pd.DataFrame.from_dict(miss_latencies_data_err_corr, orient='index', columns=['Frequency', 'Row_buffer_hit_freq'])
                    df2_err_corr = pd.DataFrame.from_dict(hit_latencies_data_err_corr, orient='index', columns=['Frequency'])

                    # Rename the index column
                    df1 = df1.rename_axis('Latency')
                    df2 = df2.rename_axis('Latency')
                    df1_err_corr = df1_err_corr.rename_axis('Latency')
                    df2_err_corr = df2_err_corr.rename_axis('Latency')

                    # Write DataFrame to csv
                    df1.to_csv(miss_file, index=True)
                    df2.to_csv(hit_file, index=True)
                    df1_err_corr.to_csv(miss_file_err_corr, index=True)
                    df2_err_corr.to_csv(hit_file_err_corr, index=True)
