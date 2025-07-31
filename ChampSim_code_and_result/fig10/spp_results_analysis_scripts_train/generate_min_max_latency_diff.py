import os
import sys

#sender_arr_sizes=[3276, 6554, 9830, 13108, 16384]
sender_arr_sizes=[4915, 19660, 9830]
msg_size=512

min_bit0=sys.maxsize
max_bit0=-(sys.maxsize-1)
min_bit1=sys.maxsize
max_bit1=-(sys.maxsize-1)

# Process files in each directory
for arr_size in sender_arr_sizes:
        # File path
        sender_array_size=arr_size
        file_dir="extracted_data"
        result_file = "min_max_latency_difference_sender_arr_size_"+str(sender_array_size)+"_msg_size"+"_"+str(msg_size)+".txt"
        # Create the file path
        file_path = os.path.join(file_dir, result_file)
        # Open the file in append mode and write new data
        with open(file_path, 'r') as f:
            min_bit0=sys.maxsize
            max_bit0=-(sys.maxsize-1)
            min_bit1=sys.maxsize
            max_bit1=-(sys.maxsize-1)
            for line_number, line in enumerate(f, start=1):
                #print(line_number, line)
                if line_number > 1:
                    if min_bit0 > int(line.split(",")[0]):
                        min_bit0 = int(line.split(",")[0])
                    if max_bit0 < int(line.split(",")[1]):
                        max_bit0 = int(line.split(",")[1])
                    if min_bit1 > int(line.split(",")[2]):
                        min_bit1 = int(line.split(",")[2])
                    if max_bit1 < int(line.split(",")[3]):
                        max_bit1 = int(line.split(",")[3])
        print("sender_arr_size: ",sender_array_size," min_bit1: ",min_bit1," max_bit1: ",max_bit1," min_bit0: ",min_bit0," max_bit0: ",max_bit0)
        new_data="sender_arr_size: "+str(sender_array_size)+" min_bit1: "+str(min_bit1)+" max_bit1: "+str(max_bit1)+" min_bit0: "+str(min_bit0)+" max_bit0: "+str(max_bit0)
        output_file="temp_min_max_range_"+str(sender_array_size)+".txt"
        # Open the file in append mode and write new data
        with open(output_file, 'w') as file:
            file.write(new_data+ '\n')  # Add a newline after the new data

