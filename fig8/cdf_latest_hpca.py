from pylab import *
from matplotlib.ticker import FuncFormatter
import matplotlib.pyplot as plt
import numpy as np
from brokenaxes import brokenaxes
from collections import Counter
from collections import OrderedDict
import sys
import os
import re

if(len(sys.argv) != 2):
    print("One commandline argument is expected i.e 1 if the training_dataset results are processed or 0 if the testing_dataset results are processed. Exiting...")
    sys.exit(1)

train_data=int(sys.argv[1])
#file_name=sys.argv[2]
#num_access=sys.argv[3]
#print(mini)
#print(file_name)
#print(num_access)
#f=open("closed_row_access_8_plot.txt","r")
#f=open("rows.txt","r")


string1='train'
string2='test'
results_path='extracted_results'
file_list = os.listdir(results_path)
filtered_files=[]

if train_data == 1:
		# Filter files that contain the specified string
		filtered_files = [file for file in file_list if string1 in file]
else:
		# Filter files that contain the specified string
		filtered_files = [file for file in file_list if string2 in file]

print(len(filtered_files))
#exit(0)

x=[]
y=[]
hulla=0

# Read the content of each file
for file_name in filtered_files:
    file_path = os.path.join(results_path, file_name)
    f=open(file_path,"r")
    lines=f.readlines()
    skip=0
    for line in lines:
        line = line.strip('\n')
        #skip header.
        if(skip==0):
            skip=1
            continue
        #print(line)
        #    if(line.isnumeric()):
        #if(int(line.split(' ')[0]) > 259  or (int(line.split(' ')[0]) == int(mini) and hulla != 1)):
        y.append(int(line.split(',')[2]))  #receiver latencies
        x.append(int(line.split(',')[3]))  #sender latencies

    f.close()
y=np.sort(y)
x=np.sort(x)


#print(y)
#print(x)
#y=['92', '209', '210', '210', '210', '210', '211', '211', '211', '213', '213', '213', '214', '214']
#exit()
#print(y)

counts = Counter(y)
counts_x = Counter(x)
plt.grid()
y_prob=[]
x_prob=[]
y_unique=[]
x_unique=[]
prob_sum=0
for latency, frequency in counts.items():
    y_prob.append(prob_sum+float(frequency/len(y)))
    y_unique.append(latency)
    prob_sum += float(frequency/len(y))
    #print(latency, ":", frequency)

prob_sum=0

for latency, frequency in counts_x.items():
    x_prob.append(prob_sum+float(frequency/len(x)))
    x_unique.append(latency)
    prob_sum += float(frequency/len(x))
    #print(latency, ":", frequency)


# Convert to log10 values
x_unique_log = log10(x_unique)
y_unique_log = log10(y_unique)
#print(prob_sum)
#print(y_prob)
#print(x_prob)
print(y_unique)
print(y_unique_log)
print(x_unique)
print(x_unique_log)
#plt.margins(x=0)
#plt.margins(y=0)

if(train_data == 1):
    output_file_sender="cdf_output_sender_train.txt"
    output_file_receiver="cdf_output_receiver_train.txt"
    output_file_sender_absolute="cdf_output_sender_train_absolute.txt"
    output_file_receiver_absolute="cdf_output_receiver_train_absolute.txt"
if(train_data == 0):
    output_file_sender="cdf_output_sender_test.txt"
    output_file_receiver="cdf_output_receiver_test.txt"
    output_file_sender_absolute="cdf_output_sender_test_absolute.txt"
    output_file_receiver_absolute="cdf_output_receiver_test_absolute.txt"

# Create text files to be used in gnuplot.
# Open a file in write mode
with open(output_file_sender, "w") as file:
    # Write the header (optional)
    file.write("#Execution_latency_sender,Probability_sender\n")
    
    # Write each pair of values from the two arrays
    for a, b in zip(x_unique_log, x_prob):
        file.write(f"{a},{b}\n")

with open(output_file_receiver, "w") as file:
    # Write the header (optional)
    file.write("#Execution_latency_receiver,Probability_receiver\n")
    
    # Write each pair of values from the two arrays
    for a, b in zip(y_unique_log, y_prob):
        file.write(f"{a},{b}\n")

# Create text files to be used in gnuplot.
# Open a file in write mode
with open(output_file_sender_absolute, "w") as file:
    # Write the header (optional)
    file.write("#Execution_latency_sender,Probability_sender\n")
    
    # Write each pair of values from the two arrays
    for a, b in zip(x_unique, x_prob):
        file.write(f"{a},{b}\n")

with open(output_file_receiver_absolute, "w") as file:
    # Write the header (optional)
    file.write("#Execution_latency_receiver,Probability_receiver\n")
    
    # Write each pair of values from the two arrays
    for a, b in zip(y_unique, y_prob):
        file.write(f"{a},{b}\n")

print("Data has been written to output.csv")



#plt.step(x_unique,x_prob,linewidth=3.0)
#plt.step(y_unique,y_prob,linewidth=3.0)
plt.step(x_unique_log, x_prob, linewidth=3.0)
plt.step(y_unique_log, y_prob, linewidth=3.0)
plt.ylabel("CDF")
plt.xlabel("Sender/Receiver execution time in log scale")
#plt.title("Load access latency with unrolling factor as 8 \nwith open row policy at DRAM.")
#plt.title("Load access latency with unrolling factor as \nwith open row policy at DRAM.")

plt.legend(["sender", "receiver"], loc="center")
fig1 = plt.gcf()
#plt.show()
plt.draw()
#fig1.savefig("cdf_"+num_access+"_open_row_MSHR_32_latest.png", bbox_inches='tight')
if int(train_data) == 1:
    fig1.savefig("cdf_train_suite.pdf", bbox_inches='tight')
else:
    fig1.savefig("cdf_test_suite.pdf", bbox_inches='tight')
