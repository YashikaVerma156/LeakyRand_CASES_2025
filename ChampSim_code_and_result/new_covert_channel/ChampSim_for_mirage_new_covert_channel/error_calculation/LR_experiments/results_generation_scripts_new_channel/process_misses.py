import pandas as pd
import matplotlib.pyplot as plt

######### 1. Calculate the errors in communication from the miss count observed at LLC level. ###########

STR_NUM=1
#Actual message sent.
original_msg=[]
file_path = '../results_analysis_scripts/benchmark/benchmark_test.txt'
with open(file_path, 'r') as file:
        for current_line, line in enumerate(file, start=1):
            if current_line == STR_NUM:
                original_msg = line.strip() 

#print(len(original_msg))
#print(original_msg)

# Specify the path to your file.
file_path = 'misses.txt'  # Replace with your actual file name.

# Read the space-separated file
data = pd.read_csv(file_path, delim_whitespace=True, header=None)  # header=None if there are no column headers

# Rename columns (optional)
data.columns = ['Column1', 'Column2']

# Display the data
#print(data.head())

Bits_received=[]
receiver_llc_miss_diff=[]
miss_diff=0
bit_received=0
# Loop over rows to access both column values
for index, row in data.iterrows():
    col1 = int(row['Column1'])
    col2 = int(row['Column2'])
    #print(f"Row {index}: Column1 = {col1}, Column2 = {col2}")
    # Skip the first row completely.
    if index == 0:
        continue
    if index == 1:
        if col2 > 1000:
            bit_received = 1
        else:
            bit_received = 0
        reference_miss_count = col1
    else:
        if col1 > reference_miss_count or col1 == reference_miss_count:
            bit_received = 1
            #print("bit_received is : 1")
        else:
            bit_received = 0
            #print("bit_received is : 0")
        miss_diff=(reference_miss_count-col1)
        receiver_llc_miss_diff.append(miss_diff)
        reference_miss_count = col1
    # Save the concluded bit.
    Bits_received.append(bit_received)

#print(len(Bits_received))
#print(Bits_received)

#XXX 511 in length.
print(len(receiver_llc_miss_diff))
print(receiver_llc_miss_diff)
receiver_llc_miss_diff_for_errors=[]

error=0
zero_to_one_err=0
one_to_zero_err=0

for i in range(len(original_msg)):
    if(int(original_msg[i]) != int(Bits_received[i])):
        if(int(original_msg[i]) == 0):
            zero_to_one_err=zero_to_one_err+1
            #print(" 0->1 error position: ",i)
            error=error+1
        else:
            one_to_zero_err=one_to_zero_err+1
            #print(" 1->0 error position: ",i)
            error=error+1
            # XXX i-1 is used as there are 511 elements, first bit diff is to be calculated from cache region identification number.TODO Not done here.
            receiver_llc_miss_diff_for_errors.append(receiver_llc_miss_diff[i-1])

print("Total error: ",error," 0->1 error: ",zero_to_one_err," 1->0 error: ",one_to_zero_err)
receiver_llc_miss_diff_for_errors=sorted(receiver_llc_miss_diff_for_errors)
print(len(receiver_llc_miss_diff_for_errors))
print(receiver_llc_miss_diff_for_errors)
######### 2. Calculate the range of misses for a bit value 0 and 1. #########
bit_0_misses=[]
bit_1_misses=[]

# Filter out the misses for bit value 0 and 1.
for i in range(len(original_msg)):
    if(int(original_msg[i]) == 0):
        bit_0_misses.append(data.iloc[(i+1), 0])
    else:
        bit_1_misses.append(data.iloc[(i+1), 0])

sorted_bit_0_misses = sorted(bit_0_misses)
sorted_bit_1_misses = sorted(bit_1_misses)

# Print the smallest and largest value
print("Misses range for bit value 0.")
print(f"Smallest value: {sorted_bit_0_misses[0]}")  # First element (smallest)
print(f"Largest value: {sorted_bit_0_misses[-1]}")  # Last element (largest)
print("Misses range for bit value 1.")
print(f"Smallest value: {sorted_bit_1_misses[0]}")  # First element (smallest)
print(f"Largest value: {sorted_bit_1_misses[-1]}")  # Last element (largest)

######### 3. Calculate the difference of misses for a consecutive 00, 01, 10 and 11. #########
miss_diff_consecutive_00=[]
miss_diff_consecutive_01=[]
miss_diff_consecutive_10=[]
miss_diff_consecutive_11=[]
for i in range(len(original_msg)):
    if i == 0:
        prev_bit=int(original_msg[i])
    else:
        curr_bit=int(original_msg[i])
        if(prev_bit == 0 and curr_bit == 0):
            miss_diff_consecutive_00.append(data.iloc[(i), 0] - data.iloc[(i+1), 0])
        elif(prev_bit == 0 and curr_bit == 1):
            miss_diff_consecutive_01.append(data.iloc[(i), 0] - data.iloc[(i+1), 0])
        elif(prev_bit == 1 and curr_bit == 0):
            miss_diff_consecutive_10.append(data.iloc[(i), 0] - data.iloc[(i+1), 0])
        elif(prev_bit == 1 and curr_bit == 1):
            miss_diff_consecutive_11.append(data.iloc[(i), 0] - data.iloc[(i+1), 0])
        prev_bit=int(original_msg[i])

print(len(miss_diff_consecutive_00))
print(len(miss_diff_consecutive_01))
print(len(miss_diff_consecutive_10))
print(len(miss_diff_consecutive_11))

#sort
miss_diff_consecutive_00=sorted(miss_diff_consecutive_00)
miss_diff_consecutive_01=sorted(miss_diff_consecutive_01)
miss_diff_consecutive_10=sorted(miss_diff_consecutive_10)
miss_diff_consecutive_11=sorted(miss_diff_consecutive_11)

print(miss_diff_consecutive_00)
print(miss_diff_consecutive_01)
print(miss_diff_consecutive_10)
print(miss_diff_consecutive_11)

# Plot the data #
# Create a line plot
plt.plot(miss_diff_consecutive_00, label='consecutive_00')
plt.plot(miss_diff_consecutive_01, label='consecutive_01')
plt.plot(miss_diff_consecutive_10, label='consecutive_10')
plt.plot(miss_diff_consecutive_11, label='consecutive_11')

plt.legend()

# Add labels and title for clarity (optional)
#plt.xlabel('consecutive 00 points')
plt.ylabel('Difference in receiver_LLC_misses')
#plt.title('Line Plot of Array')
plt.savefig('line_plot_misses_diff.pdf')
