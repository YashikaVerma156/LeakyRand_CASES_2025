import os

msg_len=512
num_err_corr_algo=2

# Specify the file name or path
file_path = "lr_cycles_normalized.txt"

# Check if the file exists before deleting
if os.path.exists(file_path):
    os.remove(file_path)
    print(f"{file_path} has been deleted.")
else:
    print(f"{file_path} does not exist.")

with open("lr_cycles_normalized.txt", "w") as file:
    file.write("#LR overhead normalized to ECI = 8.\n")
    file.write("#ECI Data_bits_overhead  Marker_bits_overhead  Error_correction_overhead\n")

bench_train=0
uf=16
max_cycle_bit1=[]
max_cycle_algo1=[]
ds=[]
err_corr_interval=[8,16,32,64,128,256,512]
for eci in err_corr_interval:
    other_blocks_99=326
    if eci == 8:
        ds_99=4
    elif eci == 16:
        ds_99=5
    elif eci == 32:
        ds_99=6
        other_blocks_99=327
    elif eci == 64:
        ds_99=9
    elif eci == 128:
        ds_99=12
    elif eci == 256:
        ds_99=25
    elif eci == 512:
        ds_99=98
    file_name="Total_result_"+str(uf)+"_train_"+str(bench_train)+"_"+str(eci)+"_with_err_corr_both_algo_"+str(ds_99)+"_access_other_blocks_"+str(other_blocks_99)+".txt"
    with open (file_name,"r") as fl:
        for line in fl:
            if "Maximum_duration_1:" in line:
                line=line.strip()
                line=line.split(" ")
                max_cycle_bit1.append(int(line[13]))
                max_cycle_algo1.append(int(line[21]))
                ds.append(int(ds_99))
if len(err_corr_interval) != len(max_cycle_bit1):
    print("Something is wrong 1.")
if len(err_corr_interval) != len(max_cycle_algo1):
    print("Something is wrong 2.")
if len(err_corr_interval) != len(ds):
    print("Something is wrong 3.")

#print(max_cycle_bit1)
#print(max_cycle_algo1)
#print(ds)

# Total overhead calculation.
data_bit_overhead=[]
marker_bit_overhead=[]
err_corr_overhead=[]
total_overhead=[]

for i in range(0,len(err_corr_interval)):                
    data_bit_overhead.append(int(msg_len*max_cycle_bit1[i]))

    per_marker_bit_overhead = int(int(max_cycle_algo1[i]) / (int(ds[i])+1))
    num_err_corr_occr = int(int(msg_len / (int(err_corr_interval[i])-1)))
    total_marker_bit_overhead = (per_marker_bit_overhead * num_err_corr_occr * num_err_corr_algo)
    #print(total_marker_bit_overhead)
    marker_bit_overhead.append(total_marker_bit_overhead)

    err_corr_overh = per_marker_bit_overhead * int(ds[i])
    total_err_corr_overhead = (err_corr_overh * num_err_corr_occr * num_err_corr_algo)
    #print(total_err_corr_overhead)
    err_corr_overhead.append(total_err_corr_overhead)

    total = int(data_bit_overhead[i]) + int(marker_bit_overhead[i]) + int(err_corr_overhead[i])
    total_overhead.append(int(total))

# Overhead(in %) normalized to ECI 8.
norm_data_bit_overhead=[]
norm_marker_bit_overhead=[]
norm_err_corr_overhead=[]
for i in range(0,len(err_corr_interval)):       
    all_values=[]
    all_values.append(err_corr_interval[i])

    num=(data_bit_overhead[i]*100) / total_overhead[0]
    num = round(num, 2)
    norm_data_bit_overhead.append(num)
    all_values.append(num)

    num=(marker_bit_overhead[i]*100) / total_overhead[0]
    num = round(num, 2)
    norm_marker_bit_overhead.append(num)
    all_values.append(num)

    num=(err_corr_overhead[i]*100) / total_overhead[0]
    num = round(num, 2)
    norm_err_corr_overhead.append(num)
    all_values.append(num)

    with open("lr_cycles_normalized.txt", "a") as file:
        file.write(" ".join(str(val) for val in all_values) + "\n")
