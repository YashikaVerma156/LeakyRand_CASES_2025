import os

# Specify the file name or path
file_path = "llc_size_sensitivity.txt"

# Check if the file exists before deleting
if os.path.exists(file_path):
    os.remove(file_path)
    print(f"{file_path} has been deleted.")
else:
    print(f"{file_path} does not exist.")

with open(file_path, "w") as fl:
    fl.write("#llc_size LR_BW_128_UF DS SPP_BW\n")

llc_sz_1mb="1MB"
llc_sz_2mb="2MB"
llc_sz_4mb="4MB"

# Read bandwidth numbers for SPP.
file_bw="/home/yashikav/Desktop/Mirage_project/fig10/spp_results_analysis_scripts_test/bandwidth_4915.txt"
with open(file_bw,"r") as fl:
    line_number=0
    for line in fl:
        line=line.strip()
        line=line.split(" ")
        bw_spp_1mb=line[1]

file_bw="/home/yashikav/Desktop/Mirage_project/fig10/spp_results_analysis_scripts_test/bandwidth_9830.txt"
with open(file_bw,"r") as fl:
    line_number=0
    for line in fl:
        line=line.strip()
        line=line.split(" ")
        bw_spp_2mb=line[1]

file_bw="/home/yashikav/Desktop/Mirage_project/fig10/spp_results_analysis_scripts_test/bandwidth_19660.txt"
with open(file_bw,"r") as fl:
    line_number=0
    for line in fl:
        line=line.strip()
        line=line.split(" ")
        bw_spp_4mb=line[1]

other_blocks_1=163
other_blocks_2=326
other_blocks_4=652
ds_1=17
ds_2=13
ds_4=15
bench_train=0
eci=128
uf=128

#Read max_duration_for_bit_1 and max_duration_for_algo_1 from file for 1MB, 2MB, and 4MB.
file_name="Total_result_"+str(uf)+"_train_"+str(bench_train)+"_"+str(eci)+"_with_err_corr_both_algo_"+str(ds_1)+"_access_other_blocks_"+str(other_blocks_1)+".txt"
with open (file_name,"r") as fl:
    for line in fl:
        if "Maximum_duration_1:" in line:
            line=line.strip()
            line=line.split(" ")
            bw_lr_1mb=line[11]
            bw_lr_1mb=bw_lr_1mb.replace("Kbps","")

file_name="Total_result_"+str(uf)+"_train_"+str(bench_train)+"_"+str(eci)+"_with_err_corr_both_algo_"+str(ds_2)+"_access_other_blocks_"+str(other_blocks_2)+".txt"
with open (file_name,"r") as fl:
    for line in fl:
        if "Maximum_duration_1:" in line:
            line=line.strip()
            line=line.split(" ")
            bw_lr_2mb=line[11]
            bw_lr_2mb=bw_lr_2mb.replace("Kbps","")

file_name="Total_result_"+str(uf)+"_train_"+str(bench_train)+"_"+str(eci)+"_with_err_corr_both_algo_"+str(ds_4)+"_access_other_blocks_"+str(other_blocks_4)+".txt"
with open (file_name,"r") as fl:
    for line in fl:
        if "Maximum_duration_1:" in line:
            line=line.strip()
            line=line.split(" ")
            bw_lr_4mb=line[11]
            bw_lr_4mb=bw_lr_4mb.replace("Kbps","")

#Write to corresponding text file as per the plot. TODO Do CRFill and CRProbe for 2MB.
with open(file_path, "w") as file:
    file.write(f"{llc_sz_1mb} {bw_lr_1mb} {ds_1} {bw_spp_1mb} \n")
    file.write(f"{llc_sz_2mb} {bw_lr_2mb} {ds_2} {bw_spp_2mb} \n")
    file.write(f"{llc_sz_4mb} {bw_lr_4mb} {ds_4} {bw_spp_4mb} \n")


