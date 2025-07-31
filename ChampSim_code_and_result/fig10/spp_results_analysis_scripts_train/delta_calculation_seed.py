import pandas as pd
import sys
import os

file_path=sys.argv[1]
sender_array_size=int(sys.argv[2])
msg_size=int(sys.argv[3])
STR_NUM=int(sys.argv[4])
rand_seed=sys.argv[5]
benchmark_suit_file=sys.argv[6]

bit_received=[]
data=[]
data_bit_latencies=[]
ref_bit_latencies=[]
difference=[]

# 1. Calculate the message Communicated and store it.
# Original message sent
#str_num=STR_NUM
#for num_bits in range(0, msg_size):
#for num_bits in range(0, 40):
#    data.append(str_num%2)
#    a=str_num%2
#    str_num = int(str_num/2)
#    if(str_num == 0):
#        str_num = STR_NUM
   # print(data)

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
        if line_number % 2 != 0:
            latency_for_differential_bit = line.split(',')[3]
            ref_bit_latencies.append(latency_for_differential_bit)
        else:
            latency_for_data_bit = line.split(',')[3]
            data_bit_latencies.append(latency_for_data_bit)

# 3. Calculate the bit received at the receiver end.
        if(line_number > 2 and line_number %2 == 0):
            num_bit=num_bit+1
# Store the difference
            difference.append(int(latency_for_differential_bit) - int(latency_for_data_bit))


# 4. Calculate delta

# Comparing sent and received message.
difference_for_bit_1=[]
difference_for_bit_0=[]
data_bit_latencies_0=[]
data_bit_latencies_1=[]

#### Create an empty DataFrame
df = pd.DataFrame(columns=['bit_num','bit0_delta','bit1_delta'])

for num_bits in range(0, msg_size):
# Divide the latency difference into two arrays.
    if(data[num_bits] == 1):
        difference_for_bit_1.append(difference[num_bits])
        data_bit_latencies_1.append(int(data_bit_latencies[num_bits]))
        # Adding rows one at a time
        df.loc[num_bits] = [str(num_bits), '-', str(difference[num_bits])]
    else:
        difference_for_bit_0.append(difference[num_bits])
        data_bit_latencies_0.append(int(data_bit_latencies[num_bits]))
        # Adding rows one at a time
        df.loc[num_bits] = [str(num_bits), str(difference[num_bits]), '-']


###  Write to a file
folder_path="extracted_data"
result_file = "delta_" + str(sender_array_size) +"_"+str(STR_NUM)+"_"+str(rand_seed)+"_"+str(msg_size)+".txt"
if not os.path.exists(folder_path):
    os.makedirs(folder_path)
    #print(f"Folder '{folder_path}' created.")
# Create the file path
file_path = os.path.join(folder_path, result_file)
df.to_csv(file_path)
