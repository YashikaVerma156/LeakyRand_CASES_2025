import sys
import os

file_path=sys.argv[1]
sender_array_size=int(sys.argv[2])
msg_size=int(sys.argv[3])
STR_NUM=int(sys.argv[4])
rand_seed=sys.argv[5]
THRESHOLD=int(sys.argv[6])
file_dir=sys.argv[7]
benchmark_suit_file=sys.argv[8]

bit_received=[]
data=[]
data_bit_latencies=[]
print("written")

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

# 3. Calculate the bit received at the receiver end.
            if(int(latency_for_data_bit) > THRESHOLD):
                bit_received.append(1)
            else:
                bit_received.append(0)
            num_bit=num_bit+1

# 4. Calculate error
# Comparing sent and received message.
error=0
one_to_zero=0
zero_to_one=0

#Avoid considering the first occurrence of 0 bit and first occurrence of 1 bit as error in communication.
avoid_1st_1bit=-1
avoid_1st_0bit=-1

for num_bits in range(0, msg_size):
# Divide the latency difference into two arrays.
    if(data[num_bits] == 1):
        avoid_1st_1bit += 1
    else:
        avoid_1st_0bit += 1

    if(data[num_bits] == 1 and avoid_1st_1bit == 0) or (data[num_bits] == 0 and avoid_1st_0bit == 0):
        continue

    if(bit_received[num_bits] != data[num_bits]):
        error=error+1
        if(data[num_bits] == 1):
            one_to_zero = one_to_zero + 1
        else:
            zero_to_one = zero_to_one + 1

new_data='Error is: '+str(error)+' zero_to_one: '+str(zero_to_one)+' one_to_zero: '+str(one_to_zero)+' string_num: '+str(STR_NUM)+' rand_seed: '+str(rand_seed)

# File path
file_dir="extracted_data"
result_file = "error_count_sender_arr_size_"+str(sender_array_size)+"_msg_size"+"_"+str(msg_size)+"_th_"+str(THRESHOLD)+".txt"
# Create the file path
file_path = os.path.join(file_dir, result_file)
# Open the file in append mode and write new data
with open(file_path, 'a') as f:
    f.write(new_data + '\n')  # Add a newline after the new data
