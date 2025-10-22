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

file_dir_outer="misses_results_"+str(ds)+"_"+benchmark_string
file_dir_inner="misses_results_"+str(uf)

file_dir=os.path.join(file_dir_outer,file_dir_inner)
#print(file_dir)
files=os.listdir(file_dir)
#print(files)
#print(len(files))
#exit(0)

msg_arr=[]
arr_bit0_0 = [0] * (uf+1)
arr_bit1_0 = [0] * (uf+1)
arr_bit0_1 = [0] * (uf+1)
arr_bit1_1 = [0] * (uf+1)
arr_bit0_2 = [0] * (uf+1)
arr_bit1_2 = [0] * (uf+1)
arr_bit0_3 = [0] * (uf+1)
arr_bit1_3 = [0] * (uf+1)
arr_bit0_4 = [0] * (uf+1)
arr_bit1_4 = [0] * (uf+1)
arr_bit0_5 = [0] * (uf+1)
arr_bit1_5 = [0] * (uf+1)
arr_bit0_6 = [0] * (uf+1)
arr_bit1_6 = [0] * (uf+1)
arr_bit0_7 = [0] * (uf+1)
arr_bit1_7 = [0] * (uf+1)
arr_bit0_8 = [0] * (uf+1)
arr_bit1_8 = [0] * (uf+1)
arr_bit0_9 = [0] * (uf+1)
arr_bit1_9 = [0] * (uf+1)
arr_bit0_10 = [0] * (uf+1)
arr_bit1_10 = [0] * (uf+1)
arr_bit0_11 = [0] * (uf+1)
arr_bit1_11 = [0] * (uf+1)
arr_bit0_12 = [0] * (uf+1)
arr_bit1_12 = [0] * (uf+1)
arr_bit0_13 = [0] * (uf+1)
arr_bit1_13 = [0] * (uf+1)
arr_bit0_14 = [0] * (uf+1)
arr_bit1_14 = [0] * (uf+1)
arr_bit0_15 = [0] * (uf+1)
arr_bit1_15 = [0] * (uf+1)

str_num=1
num1=0
num2=0
num3=0
num4=0

with open (bench_f, "r") as bench_file:
    for (line_number, line) in enumerate(bench_file, start=1):
        msg_arr_temp=[]
        line=line.strip()
        msg_arr=[int(x) for x in line]
        msg_arr_temp=[int(x) for x in line]
        #print(msg_arr)
        #print(line_number)
        f="misses_count_"+str(line_number)+"_"+str(uf)+"_"+benchmark_string+".txt"

        file_dir_path=os.path.join(file_dir,f)        
        with open(file_dir_path, "r") as miss_numbers_file:
            line_num=0
            for line in miss_numbers_file:
                line_num += 1
                arr = [0] * (uf+1)
                if(line_num == 1):  # skip file header
                    continue
                line=line.strip().split(',')
                #print(line)
                arr=[int(x) for x in line]
                #print(arr)
                #exit()
                if num1==0 and num2==0 and num3==0 and num4==0:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_0[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_0[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])
                elif num1==0 and num2==0 and num3==0 and num4==1:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_1[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_1[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])
                    
                elif num1==0 and num2==0 and num3==1 and num4==0:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_2[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_2[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])
                   
                elif num1==0 and num2==0 and num3==1 and num4==1:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_3[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_3[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])
                    
                elif num1==0 and num2==1 and num3==0 and num4==0:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_4[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_4[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])
                    
                elif num1==0 and num2==1 and num3==0 and num4==1:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_5[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_5[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])
                    
                elif num1==0 and num2==1 and num3==1 and num4==0:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_6[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_6[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])
                elif num1==0 and num2==1 and num3==1 and num4==1:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_7[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_7[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])
                elif num1==1 and num2==0 and num3==0 and num4==0:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_8[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_8[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])
                elif num1==1 and num2==0 and num3==0 and num4==1:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_9[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_9[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])
                elif num1==1 and num2==0 and num3==1 and num4==0:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_10[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_10[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])
                elif num1==1 and num2==0 and num3==1 and num4==1:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_11[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_11[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])
                elif num1==1 and num2==1 and num3==0 and num4==0:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_12[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_12[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])
                elif num1==1 and num2==1 and num3==0 and num4==1:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_13[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_13[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])
                elif num1==1 and num2==1 and num3==1 and num4==0:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_14[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_14[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])
                elif num1==1 and num2==1 and num3==1 and num4==1:
                    if(msg_arr[line_num-2] == 1): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0, len(arr)) :
                            arr_bit1_15[i] += arr[i]
                    elif(msg_arr[line_num-2] == 0): # -2 because there is a header in file and line_number starts from 1.
                        for i in range(0,len(arr)):
                            arr_bit0_15[i] += arr[i]
                    else:
                        print(line_num)
                        print("Something is wrong in bit value.")
                        exit()
                    num1=num2
                    num2=num3
                    num3=num4
                    num4=int(msg_arr[line_num-2])

        #sanity_check
        #temp_sum=0
        #for itr in range(0, len(arr)):
        #    temp_sum += arr[itr] 
        #if temp_sum != (32768/uf):
        #    print("something is wrong line 55. temp_sum: ",temp_sum," line_number: ",line_number)
        #    exit(0)
        
#sanity_check
'''
temp_sum=0
if benchmark_string == 'train':
    for itr in range(0, len(arr_bit0)):
        temp_sum += arr_bit0[itr] 
    for itr in range(0, len(arr_bit1)):
        temp_sum += arr_bit1[itr] 
    if temp_sum != (32768/uf)*512*250:
        print("something is wrong line 65.")
        exit(0)
    print("passed train")
if benchmark_string == 'test':
    for itr in range(0, len(arr_bit0)):
        temp_sum += arr_bit0[itr] 
    for itr in range(0, len(arr_bit1)):
        temp_sum += arr_bit1[itr] 
    if temp_sum != (32768/uf)*512*500:
        print("something is wrong line 65.")
        exit(0)
    print("passed test")
'''
##print(arr_bit0)
##print(arr_bit1)
log_arr_bit0_0 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_0]
log_arr_bit1_0 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_0]
log_arr_bit0_1 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_1]
log_arr_bit1_1 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_1]
log_arr_bit0_2 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_2]
log_arr_bit1_2 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_2]
log_arr_bit0_3 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_3]
log_arr_bit1_3 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_3]
log_arr_bit0_4 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_4]
log_arr_bit1_4 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_4]
log_arr_bit0_5 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_5]
log_arr_bit1_5 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_5]
log_arr_bit0_6 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_6]
log_arr_bit1_6 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_6]
log_arr_bit0_7 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_7]
log_arr_bit1_7 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_7]
log_arr_bit0_8 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_8]
log_arr_bit1_8 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_8]
log_arr_bit0_9 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_9]
log_arr_bit1_9 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_9]
log_arr_bit0_10 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_10]
log_arr_bit1_10 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_10]
log_arr_bit0_11 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_11]
log_arr_bit1_11 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_11]
log_arr_bit0_12 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_12]
log_arr_bit1_12 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_12]
log_arr_bit0_13 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_13]
log_arr_bit1_13 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_13]
log_arr_bit0_14 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_14]
log_arr_bit1_14 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_14]
log_arr_bit0_15 = [math.log10(x) if x > 0 else -1 for x in arr_bit0_15]
log_arr_bit1_15 = [math.log10(x) if x > 0 else -1 for x in arr_bit1_15]
##print(log_arr_bit0)
##print(log_arr_bit1)
##exit(0)
'''
print("arr_bit0_7:",len(arr_bit0_7),arr_bit0_7)
print("arr_bit1_7:",len(arr_bit1_7),arr_bit1_7)
print("log_arr_bit0_7:",len(log_arr_bit0_7),log_arr_bit0_7)
print("log_arr_bit1_7:",len(log_arr_bit1_7),log_arr_bit1_7)

print("arr_bit0_15:",len(arr_bit0_15),arr_bit0_15)
print("arr_bit1_15:",len(arr_bit1_15),arr_bit1_15)
print("log_arr_bit0_15:",len(log_arr_bit0_15),log_arr_bit0_15)
print("log_arr_bit1_15:",len(log_arr_bit1_15),log_arr_bit1_15)
print("log_arr_bit0_15_1:",len(log_arr_bit0_15_1),log_arr_bit0_15_1)
print("log_arr_bit1_15_1:",len(log_arr_bit1_15_1),log_arr_bit1_15_1)
'''

size=15

#plt.set(style="whitegrid")
#plt.plot(arr_bit0, color='skyblue')
#plt.plot(arr_bit1, color='skyblue')
#plt.xticks(np.arange(0,len(log_arr_bit1)),fontsize=size)
plt.xticks(np.arange(0,len(log_arr_bit1_0),uf/16),fontsize=size)
#plt.yticks(fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_0, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_0, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace0, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_0)-1,0, -1):
    #print(i)
    if log_arr_bit1_0[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace0_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots

plt.xticks(np.arange(0,len(log_arr_bit1_1),uf/16),fontsize=size)
#plt.yticks(fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_1, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_1, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace1, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_1)-1,0, -1):
    #print(i)
    if log_arr_bit1_1[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace1_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots

plt.xticks(np.arange(0,len(log_arr_bit1_2),uf/16),fontsize=size)
#plt.yticks(fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_2, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_2, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace2, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_2)-1,0, -1):
    #print(i)
    if log_arr_bit1_2[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace2_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots

plt.xticks(np.arange(0,len(log_arr_bit1_3),uf/16),fontsize=size)
#plt.yticks(fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_3, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_3, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace3, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_3)-1,0, -1):
    #print(i)
    if log_arr_bit1_3[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace3_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots

plt.xticks(np.arange(0,len(log_arr_bit1_4),uf/16),fontsize=size)
#plt.yticks(fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_4, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_4, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace4, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_4)-1,0, -1):
    #print(i)
    if log_arr_bit1_4[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace4_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots

plt.xticks(np.arange(0,len(log_arr_bit1_5),uf/16),fontsize=size)
#plt.yticks(fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_5, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_5, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace5, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_5)-1,0, -1):
    #print(i)
    if log_arr_bit1_5[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace5_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots

plt.xticks(np.arange(0,len(log_arr_bit1_6),uf/16),fontsize=size)
#plt.yticks(fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_6, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_6, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace6, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_6)-1,0, -1):
    #print(i)
    if log_arr_bit1_6[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace6_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots

plt.xticks(np.arange(0,len(log_arr_bit1_7),uf/16),fontsize=size)
#plt.yticks(fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_7, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_7, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace7, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_7)-1,0, -1):
    #print(i)
    if log_arr_bit1_7[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace7_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots

plt.xticks(np.arange(0,len(log_arr_bit1_8),uf/16),fontsize=size)
#plt.yticks(fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_8, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_8, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace8, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_8)-1,0, -1):
    #print(i)
    if log_arr_bit1_8[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace8_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots

plt.xticks(np.arange(0,len(log_arr_bit1_9),uf/16),fontsize=size)
#plt.yticks(fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_9, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_9, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace9, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_9)-1,0, -1):
    #print(i)
    if log_arr_bit1_9[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace9_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots

plt.xticks(np.arange(0,len(log_arr_bit1_10),uf/16),fontsize=size)
#plt.yticks(fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_10, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_10, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace10, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_10)-1,0, -1):
    #print(i)
    if log_arr_bit1_10[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace10_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots

plt.xticks(np.arange(0,len(log_arr_bit1_11),uf/16),fontsize=size)
#plt.yticks(fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_11, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_11, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace11, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_11)-1,0, -1):
    #print(i)
    if log_arr_bit1_11[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace11_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots

plt.xticks(np.arange(0,len(log_arr_bit1_12),uf/16),fontsize=size)
#plt.yticks(fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_12, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_12, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace12, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_12)-1,0, -1):
    #print(i)
    if log_arr_bit1_12[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace12_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots

plt.xticks(np.arange(0,len(log_arr_bit1_13),uf/16),fontsize=size)
#plt.yticks(fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_13, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_13, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace13, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_13)-1,0, -1):
    #print(i)
    if log_arr_bit1_13[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace13_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots

plt.xticks(np.arange(0,len(log_arr_bit1_14),uf/16),fontsize=size)
#plt.yticks(fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_14, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_14, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace14, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_14)-1,0, -1):
    #print(i)
    if log_arr_bit1_14[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace14_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots

plt.xticks(np.arange(0,len(log_arr_bit1_15),uf/16),fontsize=size)
plt.yticks(range(0,7,1),fontsize=size)
plt.plot(log_arr_bit0_15, color='skyblue', label= 'bitvalue 0')
plt.plot(log_arr_bit1_15, color='black', label= 'bitvalue 1')
#plt.xticks(range(0,(uf+1),1))
plt.title("Misses frequency, subspace15, "+benchmark_string+" suite, |DS| "+str(ds), fontsize=size)
plt.xlabel("Number of misses in an iteration of UF "+str(uf), fontsize=size)
plt.ylabel("Frequency in log 10", fontsize=size)
plt.ylim(-1,)
for i in range(len(log_arr_bit1_15)-1,0, -1):
    #print(i)
    if log_arr_bit1_15[i] != -1:
        break
plt.xlim(0,i+2)
plt.legend(fontsize=size)
# Save the plot
plt.savefig("subspace15_plot_bit0_bit1_"+str(uf)+"_"+str(ds)+".pdf", dpi=300, bbox_inches='tight')  # You can change the filename/format
plt.close()  # Close the plot to avoid displaying it or overlapping with future plots
