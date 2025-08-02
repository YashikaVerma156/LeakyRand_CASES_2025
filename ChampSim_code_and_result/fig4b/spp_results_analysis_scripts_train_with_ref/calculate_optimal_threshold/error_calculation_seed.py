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
ref_bit_latencies=[]
difference=[]

# 1. Calculate the message Communicated and store it.
# Original message sent
#str_num=STR_NUM
#for num_bits in range(0, msg_size):
#for num_bits in range(0, 40):
#    data.append(str_num%2)
#    a=str_num%2
    #print('bit ', num_bits, ' is: ',a)
#    str_num = int(str_num/2)
#    if(str_num == 0):
#        str_num = STR_NUM
#print("data length: ",len(data))

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
            #print('latency_for_differential_bit: ', latency_for_differential_bit,', latency_for_data_bit: ', latency_for_data_bit)
            #if(latency_for_differential_bit >= latency_for_data_bit):
            if((int(latency_for_differential_bit) - int(latency_for_data_bit)) >= THRESHOLD):
                bit_received.append(0)
                #print('bit ',num_bit,' is 0')
            else:
                bit_received.append(1)
                #print('bit ', num_bit,' is 1')
            num_bit=num_bit+1
# Store the difference
            difference.append(int(latency_for_differential_bit) - int(latency_for_data_bit))

# 4. Calculate error

# Comparing sent and received message.
error=0
one_to_zero=0
zero_to_one=0
difference_for_bit_1=[]
difference_for_bit_0=[]
data_bit_latencies_0=[]
data_bit_latencies_1=[]

# For identifying the threshold.
diff_zero_to_one=[]
diff_one_to_zero=[]

#print(len(difference),' ',len(data))

for num_bits in range(0, msg_size):
# Divide the latency difference into two arrays.
    if(data[num_bits] == 1):
        difference_for_bit_1.append(difference[num_bits])
        data_bit_latencies_1.append(int(data_bit_latencies[num_bits]))
    else:
        difference_for_bit_0.append(difference[num_bits])
        data_bit_latencies_0.append(int(data_bit_latencies[num_bits]))

    if(bit_received[num_bits] != data[num_bits]):
        error=error+1
        if(data[num_bits] == 1):
            one_to_zero = one_to_zero + 1
            diff_one_to_zero.append(difference[num_bits])
        else:
            zero_to_one = zero_to_one + 1
            diff_zero_to_one.append(difference[num_bits])



        #print('error_location: ',num_bits, ' error type: ',data[num_bits],' -> ',bit_received[num_bits])
#print('Error is: ',error,' zero_to_one: ',zero_to_one,' one_to_zero: ',one_to_zero)
new_data='Error is: '+str(error)+' zero_to_one: '+str(zero_to_one)+' one_to_zero: '+str(one_to_zero)+' string_num: '+str(STR_NUM)+' rand_seed: '+str(rand_seed)
#for num_bits in range(0, msg_size):
#    print('latency: ',data_bit_latencies[num_bits],' data bit sent: ',data[num_bits],' bit received: ',bit_received[num_bits])

# File path
result_file = "error_count_sender_arr_size_"+str(sender_array_size)+"_msg_size"+"_"+str(msg_size)+"_th_"+str(THRESHOLD)+".txt"
# Create the file path
file_path = os.path.join(file_dir, result_file)
# Open the file in append mode and write new data
with open(file_path, 'a') as f:
    f.write(new_data + '\n')  # Add a newline after the new data

# File path
result_file = "error_diff_1to0_sender_arr_size_"+str(sender_array_size)+"_msg_size"+"_"+str(msg_size)+"_th_"+str(THRESHOLD)+".txt"
# Create the file path
file_path = os.path.join(file_dir, result_file)
# Open the file in append mode and write new data
with open(file_path, 'a') as f:
    # Iterate over the list and write each item to the file
    for item in diff_one_to_zero:
        f.write(str(item) + '\n')

# File path
result_file = "error_diff_0to1_sender_arr_size_"+str(sender_array_size)+"_msg_size"+"_"+str(msg_size)+"_th_"+str(THRESHOLD)+".txt"
# Create the file path
file_path = os.path.join(file_dir, result_file)
# Open the file in append mode and write new data
with open(file_path, 'a') as f:
    # Iterate over the list and write each item to the file
    for item in diff_zero_to_one:
        f.write(str(item) + '\n')

