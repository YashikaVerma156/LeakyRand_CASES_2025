import sys
import pandas as pd
import matplotlib.pyplot as plt

if len(sys.argv) < 2:
    print("Enter the sender disturbance set value. It should be 409, or 819, or 1638, or 3276, or 6554, or 9830.")
    exit()

sender_ds=int(sys.argv[1])

######### 1. Calculate the errors in communication from the latency observed at LLC level. ###########

if sender_ds == 409:
    THRESHOLD=5689  # Best optimal threshold for |DS| 409.
    min_th=-8086
    max_th=5883
elif sender_ds == 819:
    THRESHOLD=3656  # Best optimal threshold for |DS| 819.
    min_th=-4538
    max_th=3656
elif sender_ds == 1638:
    THRESHOLD=1491  # Best optimal threshold for |DS| 1638.
    min_th=-3948
    max_th=2783
elif sender_ds == 3276:
    THRESHOLD=1485  # Best optimal threshold for |DS| 3276.
    min_th=-3657
    max_th=1717
elif sender_ds == 6554:
    THRESHOLD=450  # Best optimal threshold for |DS| 6554.
    min_th=-1944
    max_th=450
elif sender_ds == 9830:
    THRESHOLD=-1800 #-988 #199 #710  # Best optimal threshold for |DS| 9830.
    min_th=-2167
    max_th=710

STR_NUM=1
#Actual message sent.
original_msg=[]
file_path = '../results_analysis_scripts/benchmark/benchmark_test.txt'
with open(file_path, 'r') as file:
        for current_line, line in enumerate(file, start=1):
            if current_line == STR_NUM:
                original_msg = line.strip()

# Specify the path to your file.
file_path = 'extracted_data/output_'+str(sender_ds)+'_1_1000_512.txt'
print(file_path)
# Read the space-separated file
data = pd.read_csv(file_path, sep=',')  # header=None if there are no column headers

# Rename columns (optional)
data.columns = ['Column1', 'Column2', 'Column3', 'Column4', 'Column5']
prev_error=0
for i in range(min_th, max_th):
    THRESHOLD=i
    Bits_received=[]
    receiver_latency_diff=[]
    latency_diff=0
    bit_received=0

# Identify the bit received on the basis of dynamic(just the previous) thresholding.
# Loop over rows to access both column values
    for index, row in data.iterrows():
        col4 = int(row['Column4'])
        col5 = int(row['Column5'])
    #print(f"Row {index}: Column4 = {col4}, Column5 = {col5}")
    #exit()
    # Skip the first row completely.
        if index == 0:
            continue
        if index == 1:
            if col5 > 1000:
                bit_received = 1
            else:
                bit_received = 0
            reference_latency = col4
        else:
            #if col4 > reference_latency or col4 == reference_latency:
            if reference_latency + THRESHOLD >= col4 :
                bit_received = 0
                #print("bit_received is : 1")
            else:
                bit_received = 1
                #print("bit_received is : 0")
            latency_diff=(reference_latency-col4)
            receiver_latency_diff.append(latency_diff)
            reference_latency = col4
        # Save the concluded bit.
        Bits_received.append(bit_received)

#XXX 511 in length.
#print(len(receiver_latency_diff))
#print(receiver_latency_diff)
    receiver_latency_diff_for_errors=[]
    error=0
    zero_to_one_err=0
    one_to_zero_err=0
    prev_bit_0_0to1=0
    prev_bit_1_0to1=0
    prev_bit_0_1to0=0
    prev_bit_1_1to0=0

    for i in range(len(original_msg)):
        if(i>0):
            previous_bit=int(original_msg[i-1])
        if(int(original_msg[i]) != int(Bits_received[i])):
            if(int(original_msg[i]) == 0):
                zero_to_one_err=zero_to_one_err+1
                #print(" 0->1 error position: ",i)
                error=error+1
                if(previous_bit == 0):
                    prev_bit_0_0to1 += 1
                elif(previous_bit == 1):
                    prev_bit_1_0to1 += 1
            else:
                one_to_zero_err=one_to_zero_err+1
                #print(" 1->0 error position: ",i)
                error=error+1
                if(previous_bit == 0):
                    prev_bit_0_1to0 += 1
                elif(previous_bit == 1):
                    prev_bit_1_1to0 += 1
                # XXX i-1 is used as there are 511 elements, first bit diff is to be calculated from cache region identification number.TODO Not done here.
            receiver_latency_diff_for_errors.append(receiver_latency_diff[i-1])
    if(prev_error > error or prev_error == 0):
        print("THRESHOLD: ",THRESHOLD," Total error: ",error," 0->1 error: ",zero_to_one_err," 1->0 error: ",one_to_zero_err)
        receiver_latency_diff_for_errors=sorted(receiver_latency_diff_for_errors)
        print(len(receiver_latency_diff_for_errors))
        #print(receiver_latency_diff_for_errors)
        print("prev_bit_0_0to1: ",prev_bit_0_0to1," prev_bit_1_0to1: ",prev_bit_1_0to1," prev_bit_0_1to0: ",prev_bit_0_1to0," prev_bit_1_1to0: ",prev_bit_1_1to0)
        prev_error = error

exit()

######### 3. Calculate the difference of latencies for a consecutive 00, 01, 10 and 11. #########
lat_diff_consecutive_00=[]
lat_diff_consecutive_01=[]
lat_diff_consecutive_10=[]
lat_diff_consecutive_11=[]
for i in range(len(original_msg)):
    if i == 0:
        prev_bit=int(original_msg[i])
    else:
        curr_bit=int(original_msg[i])
        if(prev_bit == 0 and curr_bit == 0):
            lat_diff_consecutive_00.append(data.iloc[(i), 3] - data.iloc[(i+1), 3])
       #     print(data.iloc[(i), 3])
       #     print(data.iloc[(i+1), 3])
        elif(prev_bit == 0 and curr_bit == 1):
            lat_diff_consecutive_01.append(data.iloc[(i), 3] - data.iloc[(i+1), 3])
        elif(prev_bit == 1 and curr_bit == 0):
            lat_diff_consecutive_10.append(data.iloc[(i), 3] - data.iloc[(i+1), 3])
        elif(prev_bit == 1 and curr_bit == 1):
            lat_diff_consecutive_11.append(data.iloc[(i), 3] - data.iloc[(i+1), 3])
        prev_bit=int(original_msg[i])

if( ( (len(lat_diff_consecutive_00)) + (len(lat_diff_consecutive_01)) + (len(lat_diff_consecutive_10)) + (len(lat_diff_consecutive_11)) ) != 511):
    print("Sum is not accurate.")
    exit()

print(len(lat_diff_consecutive_00))
print(len(lat_diff_consecutive_01))
print(len(lat_diff_consecutive_10))
print(len(lat_diff_consecutive_11))

#sort
lat_diff_consecutive_00=sorted(lat_diff_consecutive_00)
lat_diff_consecutive_01=sorted(lat_diff_consecutive_01)
lat_diff_consecutive_10=sorted(lat_diff_consecutive_10)
lat_diff_consecutive_11=sorted(lat_diff_consecutive_11)

print('00: ',lat_diff_consecutive_00)
print('01: ',lat_diff_consecutive_01)
print('10: ',lat_diff_consecutive_10)
print('11: ',lat_diff_consecutive_11)

######### 4. Calculate the difference of latencies for a one previous, consecutive 00, 01, 10 and 11. #########
lat_diff_one_prev_consecutive_00=[]
lat_diff_one_prev_consecutive_01=[]
lat_diff_one_prev_consecutive_10=[]
lat_diff_one_prev_consecutive_11=[]
for i in range(len(original_msg)):
    if i == 0:
        prev_bit=int(original_msg[i])
    else:
        if i == 1:
            continue
        curr_bit=int(original_msg[i])
        if(prev_bit == 0 and curr_bit == 0):
            lat_diff_one_prev_consecutive_00.append(data.iloc[(i-1), 3] - data.iloc[(i+1), 3])
            #print('i-1: ',data.iloc[(i-1), 3],' ',data.iloc[(i-1), 4])
            #print('i: ',data.iloc[(i), 3],' ',data.iloc[(i), 4])
            #print('i+1: ',data.iloc[(i+1), 3],' ',data.iloc[(i+1), 4])
        elif(prev_bit == 0 and curr_bit == 1):
            lat_diff_one_prev_consecutive_01.append(data.iloc[(i-1), 3] - data.iloc[(i+1), 3])
        elif(prev_bit == 1 and curr_bit == 0):
            lat_diff_one_prev_consecutive_10.append(data.iloc[(i-1), 3] - data.iloc[(i+1), 3])
        elif(prev_bit == 1 and curr_bit == 1):
            lat_diff_one_prev_consecutive_11.append(data.iloc[(i-1), 3] - data.iloc[(i+1), 3])
        prev_bit=int(original_msg[i])

#print(len(lat_diff_one_prev_consecutive_00))
#print(len(lat_diff_one_prev_consecutive_01))
#print(len(lat_diff_one_prev_consecutive_10))
#print(len(lat_diff_one_prev_consecutive_11))

#sort
lat_diff_one_prev_consecutive_00=sorted(lat_diff_one_prev_consecutive_00)
lat_diff_one_prev_consecutive_01=sorted(lat_diff_one_prev_consecutive_01)
lat_diff_one_prev_consecutive_10=sorted(lat_diff_one_prev_consecutive_10)
lat_diff_one_prev_consecutive_11=sorted(lat_diff_one_prev_consecutive_11)

#print('00: ',lat_diff_one_prev_consecutive_00)
#print('01: ',lat_diff_one_prev_consecutive_01)
#print('10: ',lat_diff_one_prev_consecutive_10)
#print('11: ',lat_diff_one_prev_consecutive_11)


