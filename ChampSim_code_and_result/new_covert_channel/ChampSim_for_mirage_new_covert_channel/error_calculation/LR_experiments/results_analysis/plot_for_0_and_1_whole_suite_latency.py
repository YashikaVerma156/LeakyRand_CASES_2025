import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import math
import sys
import os

if len(sys.argv) != 4:
    print("Input correct arguments.")
    print("1. bench_test: 0 for train or 1 for test")
    print("2. unrolling_factor")
    print("2. distrubance_set size")
    exit(0)

bench_test=int(sys.argv[1])
uf=int(sys.argv[2])
ds=int(sys.argv[3])

if bench_test == 0:
    bench_f="benchmark_train.txt"
    benchmark_string="train"
elif bench_test == 1:
    bench_f="benchmark_test.txt"
    benchmark_string="test"

file_dir_outer="latency_results_"+str(ds)+"_"+benchmark_string
file_dir_inner="latency_results_"+str(uf)

file_dir=os.path.join(file_dir_outer,file_dir_inner)
#print(file_dir)
files=os.listdir(file_dir)
#print(files)
#print(len(files))
#exit(0)

msg_arr=[]
arr_bit0=[]
arr_bit1=[]

str_num=1
with open (bench_f, "r") as bench_file:
    for (line_number, line) in enumerate(bench_file, start=1):
        msg_arr_temp=[]
        line=line.strip()
        msg_arr=[int(x) for x in line]
        msg_arr_temp=[int(x) for x in line]
        #print(msg_arr)
        #print(line_number)
        f="latency_count_"+str(line_number)+"_"+str(uf)+"_"+benchmark_string+".txt"

        file_dir_path=os.path.join(file_dir,f)        
        with open(file_dir_path, "r") as latency_numbers_file:
            line_num=0
            for line in latency_numbers_file:
                line_num += 1
                arr = [0] * int((32768/uf))
                if(line_num == 1):  # skip file header
                    continue
                line=line.strip().split(',')
                #print(line)
                arr=[int(x) for x in line]
                #print(arr)
                #exit()
                if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                    for i in range(0,len(arr)):
                        arr_bit1.append(arr[i])
                elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                    for i in range(0,len(arr)):
                        arr_bit0.append(arr[i])
                else:
                    print(line_num)
                    print("Something is wrong in bit value.")
                    exit()

        #sanity_check
        #temp_sum=0
        #for itr in range(0, len(arr)):
        #    temp_sum += arr[itr] 
        #if temp_sum != (32768/uf):
        #    print("something is wrong line 55. temp_sum: ",temp_sum," line_number: ",line_number)
        #    exit(0)
        
#sanity_check
temp_sum=0
if benchmark_string == 'train':
    if len(arr_bit0)+len(arr_bit1) != (32768/uf)*512*250:
        num1=len(arr_bit0)+len(arr_bit1)
        num2=(32768/uf)*512*250
        print(num1,num2)
        print("something is wrong, line 87.")
        exit(0)
    print("passed train")
if benchmark_string == 'test':
    if len(arr_bit0)+len(arr_bit1) != (32768/uf)*512*500:
        print("something is wrong, line 92.")
        exit(0)
    print("passed test")

arr_bit0.sort()
arr_bit1.sort()
#print(arr_bit1)
#log_arr_bit0 = [math.log10(x) for x in arr_bit0 if x > 0]
#log_arr_bit1 = [math.log10(x) for x in arr_bit1 if x > 0]
#print(log_arr_bit0)
#print(log_arr_bit1)
#exit(0)

size=15

#plt.set(style="whitegrid")
#plt.plot(arr_bit0, color='skyblue')
#plt.plot(arr_bit1, color='skyblue')
#plt.xticks(np.arange(0,len(log_arr_bit1)),fontsize=size)
counts, bins = np.histogram(arr_bit0, bins=30)
plt.plot(bins[:-1], counts, label='bitvalue 0')
print(counts)
print(bins)
print(len(counts))
print(len(bins))
counts, bins = np.histogram(arr_bit1, bins=30)
plt.plot(bins[:-1], counts, label='bitvalue 1')
plt.yscale('log')
#plt.plot(log_arr_bit0, color='skyblue', label= 'bitvalue 0')
#plt.plot(log_arr_bit1, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Frequency of latency for "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Latency values observed in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency", fontsize=size)
plt.xticks(fontsize=size)
plt.yticks(fontsize=size)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("latency_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots
