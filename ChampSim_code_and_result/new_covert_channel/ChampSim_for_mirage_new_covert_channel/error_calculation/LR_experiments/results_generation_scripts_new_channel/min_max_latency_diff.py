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

# 1. Calculate the message Communicated and store it.
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
        if line_number > 2:
            latency_for_data_bit = line.split(',')[3]
            data_bit_latencies.append(latency_for_data_bit)

# For identifying the threshold.
min_bit0=sys.maxsize
max_bit0=-(sys.maxsize-1)
min_bit1=sys.maxsize
max_bit1=-(sys.maxsize-1)
bit_num_min0=-1
bit_num_max0=-1
bit_num_min1=-1
bit_num_max1=-1
skip_first_one=0
skip_first_zero=0
for num_bits in range(0, msg_size):
# Divide the latency difference into two arrays.
    if(data[num_bits] == 1):
        if(skip_first_one == 0):
            skip_first_one=1
            continue
        # Initialize the elements.
        if(bit_num_max1 == -1 and bit_num_min1 == -1):
            max_bit1 = int(data_bit_latencies[num_bits])
            min_bit1 = int(data_bit_latencies[num_bits])
            bit_num_max1 = num_bits
            bit_num_min1 = num_bits
        else:    
            if(int(data_bit_latencies[num_bits]) > max_bit1):
                max_bit1 = int(data_bit_latencies[num_bits])
                bit_num_max1 = num_bits
            elif(int(data_bit_latencies[num_bits]) < min_bit1):
                min_bit1 = int(data_bit_latencies[num_bits])
                bit_num_min1 = num_bits
    else:
        if(skip_first_zero == 0):
            skip_first_zero=1
            continue
        # Initialize the elements.
        if(bit_num_max0 == -1 and bit_num_min0 == -1):
            max_bit0 = int(data_bit_latencies[num_bits])
            min_bit0 = int(data_bit_latencies[num_bits])
            bit_num_max0 = num_bits
            bit_num_min0 = num_bits
        else:    
            if(int(data_bit_latencies[num_bits]) > max_bit0):
                max_bit0 = int(data_bit_latencies[num_bits])
                bit_num_max0 = num_bits
            elif(int(data_bit_latencies[num_bits]) < min_bit0):
                min_bit0 = int(data_bit_latencies[num_bits])
                bit_num_min0 = num_bits

# File path
file_dir="extracted_data"
result_file = "min_max_latency_difference_sender_arr_size_"+str(sender_array_size)+"_msg_size"+"_"+str(msg_size)+".txt"
# Create the file path
file_path = os.path.join(file_dir, result_file)
# Open the file in append mode and write new data
with open(file_path, 'a') as f:
    f.write(str(min_bit0)+','+str(max_bit0)+','+str(min_bit1)+','+str(max_bit1) +','+str(STR_NUM)+','+str(rand_seed)+','+str(bit_num_min0)+','+str(bit_num_max0)+','+str(bit_num_min1)+','+str(bit_num_max1) +'\n')  # Add a newline after the new data
