import pandas as pd
import re
import os
import sys


sender_execution_cycles=[]
sender_execution_cycles.append(0)

receiver_execution_cycles=[]
receiver_execution_cycles.append(0)

sender_execution_start=[]
receiver_execution_start=[]

def execution_latency_of_sender_receiver():

    pattern1="TURN: 1"
    pattern="TURN: 0"

    with open(file_path, "r") as file:
        for line_number, line in enumerate(file, start=1):
            if re.search(pattern, line):
                line = line.strip('\n')
                line = line.split(" ")
                receiver_execution_start.append(int(line[11]))
            elif re.search(pattern1, line):
                line = line.strip('\n')
                line = line.split(" ")
                sender_execution_start.append(int(line[11]))

    for i in range(1, len(sender_execution_start)):
        sender_execution_cycles.append(receiver_execution_start[i] - sender_execution_start[i-1])
        receiver_execution_cycles.append(sender_execution_start[i] - receiver_execution_start[i])

def count_misses_for_sender_and_receiver():

    pattern1="found a miss for CPU: 1"
    pattern="found a miss for CPU: 0"
    sender_misses.append(0)
    receiver_misses.append(0)
    for i in range(1, len(cpus_wait_cycle_line_num)):
        start_line = cpus_wait_cycle_line_num[i-1]
        end_line = cpus_wait_cycle_line_num[i]

        count_sender=0
        count_receiver=0
        with open(file_path, "r") as file:
          for line_number, line in enumerate(file, start=1):
                if start_line <= line_number <= end_line and pattern in line:
                    #print(f"Pattern found in line {line_number}: {line.strip()}")
                    count_receiver=count_receiver+1
                elif start_line <= line_number <= end_line and pattern1 in line:
                    count_sender=count_sender+1
        if i%2 == 1:
            sender_misses.append(count_sender)
        else:
            receiver_misses.append(count_receiver)

def extract_misses_for_sender_and_receiver():

    pattern1="Misses observed for cpu0 and cpu1 are:"
    #sender_misses.append(0)
    #receiver_misses.append(0)
    with open(file_path, "r") as file:
        for line_number, line in enumerate(file, start=1):
            if re.search(pattern1, line):
                line = line.strip('\n')
                line = line.split(" ")
                receiver_misses.append(int(line[7]))
                sender_misses.append(int(line[8]))

#### total arguments
n = len(sys.argv)
#print("Total arguments passed:", n)

#### Arguments passed
#print("\nName of Python script:", sys.argv[0])

#print("\nArguments passed:", end = " ")
#for i in range(1, n):
#    print(sys.argv[i], end = " ")

file_path=sys.argv[1]
sender_array_size=sys.argv[2]
msg_size=sys.argv[3]
string_num=sys.argv[4]
rand_seed=sys.argv[5]

#print("Hiiiiiiiiiiiiiiiiiiiii")

#### Create an empty DataFrame
df = pd.DataFrame()
df1 = pd.DataFrame()

receiver_misses=[]
sender_misses=[]

flag=0
pattern = r"wait"
cpus_wait_cycle_line_num=[]
print("file_path: "+file_path)
with open(file_path, "r") as file:
    for line_number, line in enumerate(file, start=1):
        if re.search(pattern, line):
            #print(f"Pattern found in line {line_number}: {line.strip()}")
            # Don't capture the first line number as it is not needed.
            if flag != 0:
                cpus_wait_cycle_line_num.append(line_number)
            flag = flag + 1

#print(cpus_wait_cycle_line_num)

pattern = r"cpu 0 is on wait"
cpu_0_wait_cycle_num=[]
with open(file_path, "r") as file:
    for line_number, line in enumerate(file, start=1):
        if re.search(pattern, line):
            #print(f"Pattern found in line {line_number}: {line.strip()}")
            line = line.strip('\n')
            line = line.split(" ")
            cpu_0_wait_cycle_num.append(int(line[11]))
####XXX file gets closed automatically on exiting with block

new_column_title = "cpu_0_wait_cycle_num"
df[new_column_title] = cpu_0_wait_cycle_num

per_bit_latency_with_differential_signaling = []

for i in range(len(df)):
    if i > 0:
        subtraction_result = df.at[i, 'cpu_0_wait_cycle_num'] - df.at[i - 1, 'cpu_0_wait_cycle_num']
        per_bit_latency_with_differential_signaling.append(subtraction_result)
    else:
        per_bit_latency_with_differential_signaling.append(0)
        #subtracted_values.append(None)

#### Add the subtracted values to the DataFrame as a new column
df['per_bit_latency_with_differential_signaling'] = per_bit_latency_with_differential_signaling

total_cycles_spent_per_bit = []

for i in range(2, len(df), 2):
    subtraction_result = df.at[i, 'per_bit_latency_with_differential_signaling'] + df.at[i - 1, 'per_bit_latency_with_differential_signaling']
    total_cycles_spent_per_bit.append(subtraction_result)

#### Add the subtracted values to the DataFrame as a new column
df1['total_cycles_spent_per_bit'] = total_cycles_spent_per_bit
#print(df1)
#print(len(df1))

#### Calculate execution cycles of sender and receiver.
execution_latency_of_sender_receiver()

df['receiver_execution_cycles'] = receiver_execution_cycles
df['sender_execution_cycles'] = sender_execution_cycles

#### Generate receiver and sender misses.
extract_misses_for_sender_and_receiver()

#print(len(df)," sender: ",len(sender_misses)," receiver ",len(receiver_misses),)

df['sender_misses'] = sender_misses
df['receiver_misses'] = receiver_misses
#print(df)
#print(len(df))

###  Write to a file
folder_path="extracted_data"
result_file = "output_" + str(sender_array_size) +"_"+str(string_num)+"_"+str(rand_seed)+"_"+str(msg_size)+".txt"
if not os.path.exists(folder_path):
    os.makedirs(folder_path)
    print(f"Folder '{folder_path}' created.")
# Create the file path
file_path = os.path.join(folder_path, result_file)
df.to_csv(file_path)

