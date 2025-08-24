import os
import sys

# Check if no command-line arguments are provided
if len(sys.argv) != 2:
    print("one commandline argument expected")
    print("str_num : number of experiments run.")
    sys.exit(1)

# Access command-line argument.
str_num=int(sys.argv[1])


# Specify the file name or path
file_path = "error_lr_noise_ds_sensitivity.txt"

# Check if the file exists before deleting
if os.path.exists(file_path):
    os.remove(file_path)
    print(f"{file_path} has been deleted.")
else:
    print(f"{file_path} does not exist.")

with open(file_path, "w") as file:
    file.write("#noise_trigger_point  BER_DS_100  BER_DS_800\n")

bench_train=0
uf=16
eci=514
noise_trigger_point=[1,2,4,8,16,32]
for ntp in noise_trigger_point:
    all_values=[]
    other_blocks_99=326

    file_name="Total_result_"+str(uf)+"_train_"+str(bench_train)+"_"+str(eci)+"_with_err_corr_both_algo_100_access_other_blocks_"+str(other_blocks_99)+"_noise_point_"+str(ntp)+".txt"
    read_file_path="champsim_results/"+file_name
    with open (read_file_path,"r") as fl:
        for line in fl:
            if "estimated_bandwidth_with_both_algo:" in line:
                line=line.strip()
                line=line.split(" ")
                ber=int(line[1])/(str_num*512)  # bit error rate
    all_values.append(str(ntp))
    all_values.append(str(ber))

    file_name="Total_result_"+str(uf)+"_train_"+str(bench_train)+"_"+str(eci)+"_with_err_corr_both_algo_800_access_other_blocks_"+str(other_blocks_99)+"_noise_point_"+str(ntp)+".txt"
    read_file_path="champsim_results/"+file_name
    with open (read_file_path,"r") as fl:
        for line in fl:
            if "estimated_bandwidth_with_both_algo:" in line:
                line=line.strip()
                line=line.split(" ")
                ber=int(line[1])/(str_num*512)  # bit error rate
    all_values.append(str(ber))

    with open(file_path, "a") as file:
        file.write(" ".join(str(val) for val in all_values) + "\n")
