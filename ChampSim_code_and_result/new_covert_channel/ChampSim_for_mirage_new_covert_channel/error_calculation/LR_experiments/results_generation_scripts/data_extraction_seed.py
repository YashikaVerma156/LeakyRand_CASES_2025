import pandas as pd
import re
import os
import sys
import subprocess


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
history_length=sys.argv[6]
folder_path=sys.argv[7]
#folder_path="extracted_data_corrected"
print(folder_path)
sender_execution_cycles=[]
sender_execution_cycles.append(0)

receiver_execution_cycles=[]

#Initial checks
command1=f'grep -n "cpu 0 is on wait" {file_path} | head -n 1 | awk -F\':\' \'{{ print $1 }}\''
result1 = subprocess.run(command1, shell=True, capture_output=True, text=True)
if result1.returncode == 0:
    command2=f'sed -n \'1,{result1.stdout.strip()}p\' {file_path} | grep "LLC percentage occupied:" | wc | awk \'{{ print $1}}\''
    result2 = subprocess.run(command2, shell=True, capture_output=True, text=True)
    if(int(result2.stdout.strip()) != 55+int(history_length)): # 55 is the number calculated for 50(to reach 99% LLC occupancy) + 1(extra) round of receiver probe.
        print("Something is wrong ", int(result2.stdout.strip()))
        #exit(0)
else:
    print("Error:", result1.stderr.strip())
    exit(0)


# Command to execute
command1=f'grep "LLC percentage occupied:" {file_path} | head -n 52 | tail -n 1 | awk \'{{ print $8 }}\' '
# Run the command
result1 = subprocess.run(command1, shell=True, capture_output=True, text=True)
# Output handling
if result1.returncode == 0:
    #print("Result:", result1.stdout.strip())
    command2=f'grep "LLC percentage occupied:" {file_path} | head -n 53 | tail -n 1 | awk \'{{ print $8 }}\' '
    result2 = subprocess.run(command2, shell=True, capture_output=True, text=True)
    if result2.returncode == 0:
        #print("Result:", result1.stdout.strip()," ",result2.stdout.strip())
        result=int(result2.stdout.strip()) - int(result1.stdout.strip())
        #print("Result: ",result)
        receiver_execution_cycles.append(result)
    else:
        print("Error:", result2.stderr.strip())
        exit(0)
    
else:
    print("Error:", result1.stderr.strip())
    exit(0)


sender_execution_start=[]
receiver_execution_start=[]

def extract_llc_occupancy_of_sender_and_receiver():

    pattern1="Completed exec cpu: 1"
    pattern="Completed exec cpu: 0"

    receiver_llc_occ_after_sender_run.append(0)
    sender_llc_occ_after_sender_run.append(0)

    with open(file_path, "r") as file:
        for line_number, line in enumerate(file, start=1):
            if re.search(pattern, line):
                line = line.strip('\n')
                line = line.split(" ")
                receiver_llc_occ_after_receiver_run.append(int(line[14]))
                sender_llc_occ_after_receiver_run.append(int(line[15]))
            elif re.search(pattern1, line):
                line = line.strip('\n')
                line = line.split(" ")
                receiver_llc_occ_after_sender_run.append(int(line[14]))
                sender_llc_occ_after_sender_run.append(int(line[15]))


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


middle_string = file_path.split("champsim.trace_sender_")[1]
last_string = middle_string.split("new_cov_ch_")[1]
benchmark = str(last_string.split(".")[0])

#### Create an empty DataFrame
df = pd.DataFrame()
df1 = pd.DataFrame()

receiver_misses=[]
sender_misses=[]
receiver_llc_occ_after_sender_run=[]
sender_llc_occ_after_sender_run=[]
receiver_llc_occ_after_receiver_run=[]
sender_llc_occ_after_receiver_run=[]

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

#print('length: ', len(cpus_wait_cycle_line_num))
#print(cpus_wait_cycle_line_num)

#exit()

pattern = r"cpu 0 is on wait"
cpu_0_wait_cycle_num=[]
with open(file_path, "r") as file:
    for line_number, line in enumerate(file, start=1):
        if re.search(pattern, line):
            #print(f"Pattern found in line {line_number}: {line.strip()}")
            line = line.strip('\n')
            line = line.split(" ")
            cpu_0_wait_cycle_num.append(int(line[11]))

#print('length: ', len(cpu_0_wait_cycle_num))
#print(cpu_0_wait_cycle_num)
#exit()

####XXX file gets closed automatically on exiting the block

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

print(len(df)," sender: ",len(sender_misses)," receiver ",len(receiver_misses),)

df['receiver_misses'] = receiver_misses
df['sender_misses'] = sender_misses
#print(df)
#print(len(df))

#### Generate receiver and sender LLC occupancy.
extract_llc_occupancy_of_sender_and_receiver()

df['receiver_llc_occ_after_sender_run']=receiver_llc_occ_after_sender_run
df['sender_llc_occ_after_sender_run']=sender_llc_occ_after_sender_run
df['receiver_llc_occ_after_receiver_run']=receiver_llc_occ_after_receiver_run
df['sender_llc_occ_after_receiver_run']=sender_llc_occ_after_receiver_run

###  Write to a file
if benchmark == "test":
    result_file = "output_" + str(sender_array_size) +"_"+str(string_num)+"_"+str(rand_seed)+"_"+str(msg_size)+"_"+str(history_length)+"_test.txt"
if benchmark == "train":
    result_file = "output_" + str(sender_array_size) +"_"+str(string_num)+"_"+str(rand_seed)+"_"+str(msg_size)+"_"+str(history_length)+"_train.txt"
if not os.path.exists(folder_path):
    os.makedirs(folder_path)
    print(f"Folder '{folder_path}' created.")
# Create the file path
file_path = os.path.join(folder_path, result_file)
df.to_csv(file_path)

