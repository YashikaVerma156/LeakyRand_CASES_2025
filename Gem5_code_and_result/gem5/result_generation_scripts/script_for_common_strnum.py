import re
import os
import sys
import pandas as pd
from pathlib import Path

if len(sys.argv) != 3:
    print("Enter tested ds values, i.e. either 1500, 4500 or 3000")
    print("Enter invalid_buffer values, i.e. 500")
    exit(0)


ds=int(sys.argv[1])
buffer=int(sys.argv[2])
print("ds:",ds)


result_dir="result_test_ds_"+str(ds)+"_buffer_"+str(buffer)
path_to_result_dir="../results_gem5_lr/result_strlen_512"
result_dir_path=os.path.join(path_to_result_dir,result_dir)
print(result_dir_path)


root_dir = Path(result_dir_path)

target_file="board.pc.com_1.device"
target_line="0 1.200000 9"

min_occ_bit1=10000000
max_occ_bit1=0
min_occ_bit0=10000000
max_occ_bit0=0

for subdir in root_dir.iterdir():
    target_line_found=0
    seen_next_line=0
    local_min_occ_bit1=10000000
    local_max_occ_bit1=0
    local_min_occ_bit0=10000000
    local_max_occ_bit0=0
    if subdir.is_dir():
        match = re.search(r"results_strnum_(\d+)", subdir.name)
        if match:
            number = int(match.group(1))
            print(number)
        else:
            print("something is wrong, matching directory name not found.") 
            exit(0)

        with open("benchmark_test.txt", "r") as bench_f:
            line_number=0
            for line in bench_f:
                line_number+=1
                if line_number == number:
                    #print(line)
                    line=line.strip()
                    mesg = [int(c) for c in line]
                    if(len(mesg) != 512):
                        exit(0)
                    
        occ_arr=[]
        target_file_path=subdir/target_file
        if target_file_path.exists():
            with open(target_file_path, "r", encoding="utf-8", errors="ignore") as f:
                for line in f:
                    line=line.strip()
                    if target_line_found==1 and seen_next_line==1:
                        #print(line)
                        line=line.split()[2]
                        occ_arr.append(int(line))
                        # Don't read after 512 lines.
                        if len(occ_arr) ==512:
                            break
                        #exit(0)
                    # Skipping one line after the target_line.
                    if target_line_found==1 and seen_next_line==0:
                        seen_next_line=1
                    if target_line in line:
                        target_line_found=1
                        #print(f"Found  {line.strip()}")
                     
        if len(occ_arr) == 0:
            continue
            
        if len(occ_arr) !=512:
            print("Something is wrong. len(occ_arr)",len(occ_arr))
            exit(0)
        if len(mesg) !=512:
            print("Something is wrong. len(mesg)",len(mesg))
            exit(0)
        for i in range(0,(len(mesg)-1)):  # (len(mesg)-1) because we don't want to consider the last bit in min_max range calculation.
            if mesg[i] == 0:
                if occ_arr[i] < min_occ_bit0:
                    min_occ_bit0 = occ_arr[i]
                if occ_arr[i] > max_occ_bit0:
                    max_occ_bit0 = occ_arr[i]
            if mesg[i] == 1:
                if occ_arr[i] < min_occ_bit1:
                    min_occ_bit1 = occ_arr[i]
                if occ_arr[i] > max_occ_bit1:
                    max_occ_bit1 = occ_arr[i]
            if mesg[i] == 0:
                if occ_arr[i] < local_min_occ_bit0:
                    local_min_occ_bit0 = occ_arr[i]
                if occ_arr[i] > local_max_occ_bit0:
                    local_max_occ_bit0 = occ_arr[i]
            if mesg[i] == 1:
                if occ_arr[i] < local_min_occ_bit1:
                    local_min_occ_bit1 = occ_arr[i]
                if occ_arr[i] > local_max_occ_bit1:
                    local_max_occ_bit1 = occ_arr[i]
        print("local_min_occ_bit0",local_min_occ_bit0,"local_max_occ_bit0",local_max_occ_bit0,"local_min_occ_bit1",local_min_occ_bit1,"local_max_occ_bit1",local_max_occ_bit1)
        print("min_occ_bit0",min_occ_bit0,"max_occ_bit0",max_occ_bit0,"min_occ_bit1",min_occ_bit1,"max_occ_bit1",max_occ_bit1)

print("min_occ_bit0",min_occ_bit0,"max_occ_bit0",max_occ_bit0,"min_occ_bit1",min_occ_bit1,"max_occ_bit1",max_occ_bit1)


############ Error calculation ###################

# decide range for threshold testing.
if min_occ_bit0 < min_occ_bit1:
    min_th=min_occ_bit0
else:
    min_th=min_occ_bit1

if max_occ_bit0 > max_occ_bit1:
    max_th=max_occ_bit0
else:
    max_th=max_occ_bit1

print("min_th:",min_th,"max_th",max_th)

least_global_err_th=0
least_global_err=10000000000
least_global_err_0to1=0
least_global_err_1to0=0
for th in range(min_th, max_th+1): #max_th+1 because max_th should be included in the test for optimal_threshold.
    print("Processing TH:",th,"min_th:",min_th,"max_th",max_th,"ds:",ds)
    global_err=0
    global_err_0to1=0
    global_err_1to0=0
    for subdir in root_dir.iterdir():
        target_line_found=0
        seen_next_line=0
        if subdir.is_dir():
            match = re.search(r"results_strnum_(\d+)", subdir.name)
            if match:
                number = int(match.group(1))
        
            with open("benchmark_test.txt", "r") as bench_f:
                line_number=0
                for line in bench_f:
                    line_number+=1
                    if line_number == number:
                        #print(line)
                        line=line.strip()
                        mesg = [int(c) for c in line]
                        if(len(mesg) != 512):
                            exit(0)

            occ_arr=[]
            target_file_path=subdir/target_file
            if target_file_path.exists():
                with open(target_file_path, "r", encoding="utf-8", errors="ignore") as f:
                    for line in f:
                        line=line.strip()
                        if target_line_found==1 and seen_next_line==1:
                            #print(line)
                            line=line.split()[2]
                            occ_arr.append(int(line))
                            # Don't read after 512 lines.
                            if len(occ_arr) ==512:
                                break
                            #exit(0)
                        if target_line_found==1 and seen_next_line==0:
                            seen_next_line=1
                        if target_line in line:
                            target_line_found=1
                            #print(f"Found  {line.strip()}")
            if len(occ_arr) == 0:
                continue
            if len(occ_arr) !=512:
                print("Something is wrong. len(occ_arr)",len(occ_arr))
            if len(mesg) !=512:
                print("Something is wrong. len(mesg)",len(mesg))
            
            #Error calculation.
            received_mesg=[]
            for i in range(0,len(occ_arr)):     
                if int(occ_arr[i]) < int(th):
                    received_mesg.append(1)
                else:
                    received_mesg.append(0)
            err=0
            err_1to0=0
            err_0to1=0
            for i in range(0,len(mesg)-1):  # Intentionally skipping the last bit as it observes the thread exit disturbance.       
                if int(mesg[i]) != int(received_mesg[i]):
                    err += 1
                    if int(mesg[i]) == 0:
                        err_0to1 += 1
                    elif int(mesg[i]) == 1:
                        err_1to0 += 1
                    else:
                        print("Something is wrong.")
                        exit(0)
        global_err += err
        global_err_0to1 += err_0to1
        global_err_1to0 += err_1to0
    
    if global_err < least_global_err:
        least_global_err_th=th
        least_global_err=global_err
        least_global_err_0to1=global_err_0to1
        least_global_err_1to0=global_err_1to0
print("least_err_th:",least_global_err_th,"total_err:",least_global_err,"0to1_err:",least_global_err_0to1,"1to0_err:",least_global_err_1to0)

th=least_global_err_th
str_num_list=[]
err_list=[]
for subdir in root_dir.iterdir():
    target_line_found=0
    seen_next_line=0
    if subdir.is_dir():
        match = re.search(r"results_strnum_(\d+)", subdir.name)
        if match:
            number = int(match.group(1))

        with open("benchmark_test.txt", "r") as bench_f:
            line_number=0
            for line in bench_f:
                line_number+=1
                if line_number == number:
                    #print(line)
                    line=line.strip()
                    mesg = [int(c) for c in line]
                    if(len(mesg) != 512):
                        exit(0)

        occ_arr=[]
        target_file_path=subdir/target_file
        if target_file_path.exists():
            with open(target_file_path, "r", encoding="utf-8", errors="ignore") as f:
                for line in f:
                    line=line.strip()
                    if target_line_found==1 and seen_next_line==1:
                        #print(line)
                        line=line.split()[2]
                        occ_arr.append(int(line))
                        # Don't read after 512 lines.
                        if len(occ_arr) ==512:
                            break
                        #exit(0)
                    if target_line_found==1 and seen_next_line==0:
                        seen_next_line=1
                    if target_line in line:
                        target_line_found=1
                        #print(f"Found  {line.strip()}")
        if len(occ_arr) == 0:
            continue
        if len(occ_arr) !=512:
            print("Something is wrong. len(occ_arr)",len(occ_arr))
        if len(mesg) !=512:
            print("Something is wrong. len(mesg)",len(mesg))

        #Error calculation.
        received_mesg=[]
        for i in range(0,len(occ_arr)):         
            if int(occ_arr[i]) < int(th):
                received_mesg.append(1)
            else:
                received_mesg.append(0)
        err=0
        err_1to0=0
        err_0to1=0
        for i in range(0,len(mesg)-1):  # Intentionally skipping the last bit as it observes the thread exit disturbance.       
            if int(mesg[i]) != int(received_mesg[i]):
                err += 1
                if int(mesg[i]) == 0:
                    err_0to1 += 1
                elif int(mesg[i]) == 1:
                    err_1to0 += 1
                else:
                    print("Something is wrong.")
                    exit(0)
        str_num_list.append(int(number))
        err_list.append(int(err))

# Create the DataFrame
df = pd.DataFrame({
    'str_num': str_num_list,
    'tot_err': err_list
})

df.to_csv("output_"+str(ds)+"_"+str(buffer)+".csv", index=False)
