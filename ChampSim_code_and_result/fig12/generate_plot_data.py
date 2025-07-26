import os

# Specify the file name or path
file_path = "lr_cycles_normalized_diffr_llc_size.txt"

# Check if the file exists before deleting
if os.path.exists(file_path):
    os.remove(file_path)
    print(f"{file_path} has been deleted.")
else:
    print(f"{file_path} does not exist.")

with open(file_path, "w") as fl:
    fl.write("# LR overhead normalized to 4MB LLc, in %\n")
    fl.write("#LLC_sz CRFill CRProbe Data_bits Marker_bits Error_correction\n")

llc_sz_1mb="1MB"
llc_sz_2mb="2MB"
llc_sz_4mb="4MB"

# Read Fill_cycles and Probe_cycles from temp_1MB.txt
with open("temp_1MB.txt","r") as fl:
    line_number=0
    for line in fl:
        line_number += 1
        if line_number == 1:
            line=line.strip()
            line=line.split(" ")
            f_1mb=int(line[4])
        if line_number == 2:
            line=line.strip()
            line=line.split(" ")
            p_1mb=int(line[4])

f_p_1mb=f_1mb+p_1mb

# Read Fill_cycles and Probe_cycles from temp_2MB.txt
with open("temp_2MB.txt","r") as fl:
    line_number=0
    for line in fl:
        line_number += 1
        if line_number == 1:
            line=line.strip()
            line=line.split(" ")
            f_2mb=int(line[4])
        if line_number == 2:
            line=line.strip()
            line=line.split(" ")
            p_2mb=int(line[4])

f_p_2mb=f_2mb+p_2mb

# Read Fill_cycles and Probe_cycles from temp_4MB.txt
with open("temp_4MB.txt","r") as fl:
    line_number=0
    for line in fl:
        line_number += 1
        if line_number == 1:
            line=line.strip()
            line=line.split(" ")
            f_4mb=int(line[4])
        if line_number == 2:
            line=line.strip()
            line=line.split(" ")
            p_4mb=int(line[4])

f_p_4mb=f_4mb+p_4mb

bench_train=0
eci=128
uf=128
ds_1=17  #DS for 1MB.
ds_2=13  #DS for 2MB.
ds_4=15  #DS for 4MB.

other_blocks_1=163
other_blocks_2=326
other_blocks_4=652

#Read max_duration_for_bit_1 and max_duration_for_algo_1 from file for 1MB, 2MB, and 4MB.
file_name="Total_result_"+str(uf)+"_train_"+str(bench_train)+"_"+str(eci)+"_with_err_corr_both_algo_"+str(ds_1)+"_access_other_blocks_"+str(other_blocks_1)+".txt"
with open (file_name,"r") as fl:
    for line in fl:
        if "Maximum_duration_1:" in line:
            line=line.strip()
            line=line.split(" ")
            max_dur_bit1_1mb=int(line[13])
            max_dur_algo1_1mb=int(line[21])

file_name="Total_result_"+str(uf)+"_train_"+str(bench_train)+"_"+str(eci)+"_with_err_corr_both_algo_"+str(ds_2)+"_access_other_blocks_"+str(other_blocks_2)+".txt"
with open (file_name,"r") as fl:
    for line in fl:
        if "Maximum_duration_1:" in line:
            line=line.strip()
            line=line.split(" ")
            max_dur_bit1_2mb=int(line[13])
            max_dur_algo1_2mb=int(line[21])

file_name="Total_result_"+str(uf)+"_train_"+str(bench_train)+"_"+str(eci)+"_with_err_corr_both_algo_"+str(ds_4)+"_access_other_blocks_"+str(other_blocks_4)+".txt"
with open (file_name,"r") as fl:
    for line in fl:
        if "Maximum_duration_1:" in line:
            line=line.strip()
            line=line.split(" ")
            max_dur_bit1_4mb=int(line[13])
            max_dur_algo1_4mb=int(line[21])

#Calculate data bit overhead.
data_bit_1mb=512*max_dur_bit1_1mb
data_bit_2mb=512*max_dur_bit1_2mb
data_bit_4mb=512*max_dur_bit1_4mb

#Calculate marker bit overhead.
marker_bit_1mb=((int(max_dur_algo1_1mb/(ds_1+1))) * (int(512/(eci-1))))*2
marker_bit_2mb=((int(max_dur_algo1_2mb/(ds_2+1))) * (int(512/(eci-1))))*2
marker_bit_4mb=((int(max_dur_algo1_4mb/(ds_4+1))) * (int(512/(eci-1))))*2

#Calculate error-correction overhead.
err_corr_1mb=(((int(max_dur_algo1_1mb/(ds_1+1)))*ds_1) * (int(512/(eci-1))))*2
err_corr_2mb=(((int(max_dur_algo1_2mb/(ds_2+1)))*ds_2) * (int(512/(eci-1))))*2
err_corr_4mb=(((int(max_dur_algo1_4mb/(ds_4+1)))*ds_4) * (int(512/(eci-1))))*2

#Calculate total overhead.
total_1mb=data_bit_1mb + marker_bit_1mb + err_corr_1mb + f_p_1mb
total_2mb=data_bit_2mb + marker_bit_2mb + err_corr_2mb + f_p_2mb
total_4mb=data_bit_4mb + marker_bit_4mb + err_corr_4mb + f_p_4mb

#Per bit overhead.
per_bit_1mb=total_1mb/512
per_bit_2mb=total_2mb/512
per_bit_4mb=total_4mb/512

#Bandwidth in Kbps for 4GHz processor. The results are put in section 5.9.
bw_1mb=(4000000000/per_bit_1mb)/1024
bw_2mb=(4000000000/per_bit_2mb)/1024
bw_4mb=(4000000000/per_bit_4mb)/1024

print("end to end bandwidth for 1MB cache",round(bw_1mb,2),"Kbps")
print("end to end bandwidth for 2MB cache",round(bw_2mb,2),"Kbps")
print("end to end bandwidth for 4MB cache",round(bw_4mb,2),"Kbps")

#Overhead(in %) normalized to 4MB.
norm_f_1mb=(f_1mb/total_4mb)*100
norm_p_1mb=(p_1mb/total_4mb)*100
norm_data_bit_1mb=(data_bit_1mb/total_4mb)*100
norm_marker_bit_1mb=(marker_bit_1mb/total_4mb)*100
norm_err_corr_1mb=(err_corr_1mb/total_4mb)*100

norm_f_2mb=(f_2mb/total_4mb)*100
norm_p_2mb=(p_2mb/total_4mb)*100
norm_data_bit_2mb=(data_bit_2mb/total_4mb)*100
norm_marker_bit_2mb=(marker_bit_2mb/total_4mb)*100
norm_err_corr_2mb=(err_corr_2mb/total_4mb)*100

norm_f_4mb=(f_4mb/total_4mb)*100
norm_p_4mb=(p_4mb/total_4mb)*100
norm_data_bit_4mb=(data_bit_4mb/total_4mb)*100
norm_marker_bit_4mb=(marker_bit_4mb/total_4mb)*100
norm_err_corr_4mb=(err_corr_4mb/total_4mb)*100

#Write to corresponding text file as per the plot. TODO Do CRFill and CRProbe for 2MB.
with open(file_path, "a") as file:
    file.write(f"{llc_sz_1mb} {norm_f_1mb} {norm_p_1mb} {norm_data_bit_1mb} {norm_marker_bit_1mb} {norm_err_corr_1mb} \n")
    file.write(f"{llc_sz_2mb} {norm_f_2mb} {norm_p_2mb} {norm_data_bit_2mb} {norm_marker_bit_2mb} {norm_err_corr_2mb} \n")
    file.write(f"{llc_sz_4mb} {norm_f_4mb} {norm_p_4mb} {norm_data_bit_4mb} {norm_marker_bit_4mb} {norm_err_corr_4mb} \n")


