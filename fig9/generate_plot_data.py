import os
import sys

TEX=int(sys.argv[1]) #Total_experiments run.

# Specify the file name or path
file_path = "bwerror_unroll.txt"

# Check if the file exists before deleting
if os.path.exists(file_path):
    os.remove(file_path)
    print(f"{file_path} has been deleted.")
else:
    print(f"{file_path} does not exist.")

with open(file_path, "w") as file:
    file.write("#URF    BW      BER             DS      T_ERR   0->1    1->0\n")

bench_train=0
eci=128
uf=0
ds_99=0
for i in range(0,5):
    all_values=[]
    other_blocks_99=326
    if i==0:
        uf = 32
        ds_99 = 12
    elif i == 1:
        uf = 64
        ds_99 = 13
    elif i == 2:
        uf = 128
        ds_99 = 13
    elif i == 3:
        uf = 256
        ds_99 = 18
    elif i == 4:
        uf = 256
        ds_99 = 30
    
    file_name="Total_result_"+str(uf)+"_train_"+str(bench_train)+"_"+str(eci)+"_with_err_corr_both_algo_"+str(ds_99)+"_access_other_blocks_"+str(other_blocks_99)+".txt"
    with open (file_name,"r") as fl:
        for line in fl:
            if "estimated_bandwidth_with_both_algo:" in line:
                line=line.strip()
                line=line.split(" ")
                bw=line[11] # bandwidth
                value = bw.replace("Kbps", "")
                ber=int(line[1])/(TEX*512)  # bit error rate
                terr=int(line[1])
                err0to1=int(line[3])
                err1to0=int(line[5]) 
    all_values.append(str(uf))
    all_values.append(str(value))
    all_values.append(str(ber))
    all_values.append(str(ds_99))
    all_values.append(str(terr))
    all_values.append(str(err0to1))
    all_values.append(str(err1to0))

    with open(file_path, "a") as file:
        file.write(" ".join(str(val) for val in all_values) + "\n")
