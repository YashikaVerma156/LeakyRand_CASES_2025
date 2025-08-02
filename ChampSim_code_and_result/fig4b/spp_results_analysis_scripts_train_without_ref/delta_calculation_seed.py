import pandas as pd
import sys
import os

file_path=sys.argv[1]
sender_array_size=int(sys.argv[2])
msg_size=int(sys.argv[3])
STR_NUM=int(sys.argv[4])
rand_seed=sys.argv[5]
benchmark_suit_file=sys.argv[6]

data=[]
data_bit_latencies=[]

with open(benchmark_suit_file, "r") as f:
    for line_number, line in enumerate(f, start=1):
        if line_number == STR_NUM:
            line=line.strip()
            line=list(line)
            for i in line:
                data.append(int(i))

# 2. Open file that has SPP stats related to data transmission.
with open(file_path, "r") as file:
    num_bit=0
    for line_number, line in enumerate(file, start=1):
        #print("line: ",line," line_number: ",line_number)
        if line_number > 2:
            latency_for_data_bit = line.split(',')[3]
            data_bit_latencies.append(latency_for_data_bit)

#print(data_bit_latencies)
#print(len(data_bit_latencies))
#exit()

# 4. Calculate delta

#### Create an empty DataFrame
df = pd.DataFrame(columns=['bit0_latency','bit1_latency'])

for num_bits in range(0, msg_size):
# Divide the latency into two arrays.
    if(data[num_bits] == 1):
        # Adding rows one at a time
        df.loc[num_bits] = ['-', str(data_bit_latencies[num_bits])]
    else:
        # Adding rows one at a time
        df.loc[num_bits] = [str(data_bit_latencies[num_bits]), '-']

###  Write to a file
folder_path="extracted_data"
result_file = "delta_" + str(sender_array_size) +"_"+str(STR_NUM)+"_"+str(rand_seed)+"_"+str(msg_size)+".txt"
if not os.path.exists(folder_path):
    os.makedirs(folder_path)
    #print(f"Folder '{folder_path}' created.")
# Create the file path
file_path = os.path.join(folder_path, result_file)
df.to_csv(file_path)
