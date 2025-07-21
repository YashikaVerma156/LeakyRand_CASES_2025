import os
import re
import pandas as pd


def read_file_line_by_line(file_path, UF):
    miss_is_seen=0
    miss_instr=0
    row_buffer_hit_is_seen=0
    value1=0
    value2=0
    setup_complete=0
    pattern1='found a miss for CPU: 0'
    pattern2='row buffer hit'
    pattern3='RDTSC, sub:'

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


            if pattern1 in line:
                miss_is_seen=1
                match = re.search(r'instr_id: (\d+)', line)
                if match:
                    miss_instr = int(match.group(1))
                    #print("Extracted value:", value)
                else:
                    print("Value not found in the line.")
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
                matches = re.findall(r'\b\d+\b', line)
                values = [int(match) for match in matches]

                value1 = values[-2]  # Extracting the starting instruction id.
                value2 = values[-1]  # Extracting the last instruction id.

                # Assign to corresponding dictionary
                if miss_is_seen == 1 and ( (miss_instr > value1 or miss_instr == value1) and (miss_instr < value2 or miss_instr == value2) ):
                    #print(latency_key)
                    #print(file_path)
                    #print(line)
                    #exit()
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
                    if(latency_key == 86 and UF == 16):
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
                    if latency_key in miss_latencies_data:
                         miss_latencies_data[latency_key][0] += miss_is_seen
                         miss_latencies_data[latency_key][1] += row_buffer_hit_is_seen
                    else:
                         # Key is not present, add a new key-value pair
                         miss_latencies_data[latency_key] = [miss_is_seen, row_buffer_hit_is_seen]
                    miss_is_seen=0
                    row_buffer_hit_is_seen=0
                    miss_instr=0
                    value1=0
                    value2=0
                else:
                    if latency_key in hit_latencies_data:
                         hit_latencies_data[latency_key] += 1
                    else:
                         # Key is not present, add a new key-value pair
                         hit_latencies_data[latency_key] = 1
                    miss_is_seen=0
                    row_buffer_hit_is_seen=0
                    miss_instr=0
                    value1=0
                    value2=0
                latency_key = 0

UFs=[128]
#Algo_trigger_point=[8,16,32,64,128]
#64: 32 ; 128: 31
Algo_trigger_point=[128]
#benchmark=["test","train"] 
benchmark=["train"] 
msg_string_len=[512]
#other_blocks=["29","30","31","32"]  # 32 in other blocks signify 326 or 327
other_blocks=["31"]  # 32 in other blocks signify 326 or 327

#Declaring empty dictionary
hit_latencies_data={}
miss_latencies_data={}

#directory_path='../results/special_results'
directory_path='../LR_ERR_CORR_2core_results_both_algo_4_access'
# Get a list of all files in the specified directory
file_list = os.listdir(directory_path)

print(file_list)

#exit()

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
                    if bench == "test":
                        string2='_test.gz.txt'
                        miss_file='miss_data_UF_'+str(ob)+'_'+str(UF)+'_'+str(msl)+'_'+str(atp)+'_test.csv'
                        hit_file='hit_data_UF_'+str(ob)+'_'+str(UF)+'_'+str(msl)+'_'+str(atp)+'_test.csv'
                    string3='_'+str(msl)+'_'+str(atp)+'_with_both_algo'

                    # Filter files that contain the specified string
                    filtered_files = [file for file in file_list if string1 in file and string2 in file and string3 in file]
                    hit_latencies_data={}
                    miss_latencies_data={}
                    print('size of hit_latencies_data dict: ', len(hit_latencies_data),' size of miss_latencies_data dict: ', len(miss_latencies_data),' number of files: ',len(filtered_files))
                    if(len(filtered_files) == 0):
                        continue
                    i=0
                    # Read the content of each file
                    for file_name in filtered_files:
                        file_path = os.path.join(directory_path, file_name)
                        i=i+1
                        # Call the function with the file path
                        read_file_line_by_line(file_path, UF)
                        print(i)

                    #print(miss_latencies_data)
                    sorted_items = sorted(miss_latencies_data.items())
                    sorted_items1 = sorted(hit_latencies_data.items())

                    # Convert the sorted items back to a dictionary
                    miss_latencies_data = dict(sorted_items)
                    hit_latencies_data = dict(sorted_items1)


                    # Create a DataFrame
                    df1 = pd.DataFrame.from_dict(miss_latencies_data, orient='index', columns=['Frequency', 'Row_buffer_hit_freq'])
                    df2 = pd.DataFrame.from_dict(hit_latencies_data, orient='index', columns=['Frequency'])

                    # Rename the index column
                    df1 = df1.rename_axis('Latency')
                    df2 = df2.rename_axis('Latency')

                    # Write DataFrame to csv
                    df1.to_csv(miss_file, index=True)
                    df2.to_csv(hit_file, index=True)
