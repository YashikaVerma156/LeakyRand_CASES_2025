import os

# Specify the file name or path
file_path = "bwerror_ds.txt"

# Check if the file exists before deleting
if os.path.exists(file_path):
    os.remove(file_path)
    print(f"{file_path} has been deleted.")
else:
    print(f"{file_path} does not exist.")

with open(file_path, "w") as file:
    file.write("#DS     BW      BER             T_ERR   0->1    1->0\n")

bench_train=0
uf=16
eci=128
sender_dis=[1,3,6,9,12]
for ds_99 in sender_dis:
    all_values=[]
    other_blocks_99=326
    file_name="Total_result_"+str(uf)+"_train_"+str(bench_train)+"_"+str(eci)+"_with_err_corr_both_algo_"+str(ds_99)+"_access_other_blocks_"+str(other_blocks_99)+".txt"
    with open (file_name,"r") as fl:
        for line in fl:
            if "estimated_bandwidth_with_both_algo:" in line:
                line=line.strip()
                line=line.split(" ")
                all_values.append(str(ds_99))

                bw=line[11] # bandwidth
                bw_val = bw.replace("Kbps", "")
                all_values.append(str(bw_val))

                t_err=int(line[1])  # Total error
                ber=t_err/(500*512)  # bit error rate
                all_values.append(f"{ber:.9f}")
                all_values.append(str(t_err))

                err0to1=int(line[3])  # 0to1 error
                all_values.append(str(err0to1))

                err1to0=int(line[5])  # 1to0 error
                all_values.append(str(err1to0))

    with open(file_path, "a") as file:
        file.write(" ".join(str(val) for val in all_values) + "\n")



