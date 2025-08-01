import os

# Specify the file name or path
file_path = "bwerror_99_999.txt"

# Check if the file exists before deleting
if os.path.exists(file_path):
    os.remove(file_path)
    print(f"{file_path} has been deleted.")
else:
    print(f"{file_path} does not exist.")

with open("bwerror_99_999.txt", "w") as file:
    file.write("#1.12% UNOCUPIED                             #0.24% UNOCUPIED\n")
    file.write("#ECI  BW  BER  DS  BWZ  BER  DSZ  #ECI  BW  BER  DS  BWZ  BER  DSZ\n")

bench_train=0
uf=16
err_corr_interval=[8,16,32,64,128,256,512]
for eci in err_corr_interval:
    all_values=[]
    other_blocks_99=326
    other_blocks_999=31
    if eci == 8:
        ds_99=4
        ds_999=3
    elif eci == 16:
        ds_99=5
        ds_999=3
    elif eci == 32:
        ds_99=6
        ds_999=3
        other_blocks_99=327
    elif eci == 64:
        ds_99=9
        ds_999=4
    elif eci == 128:
        ds_99=12
        ds_999=4
    elif eci == 256:
        ds_99=25
        ds_999=5
    elif eci == 512:
        ds_99=98
        ds_999=9
    file_name="Total_result_"+str(uf)+"_train_"+str(bench_train)+"_"+str(eci)+"_with_err_corr_both_algo_"+str(ds_99)+"_access_other_blocks_"+str(other_blocks_99)+".txt"
    read_file_path="results_analysis_scripts/"+file_name
    with open (read_file_path,"r") as fl:
        for line in fl:
            if "estimated_bandwidth_with_both_algo:" in line:
                line=line.strip()
                line=line.split(" ")
                bw=line[11] # bandwidth
                value = bw.replace("Kbps", "")
                ber=int(line[1])/(500*512)  # bit error rate
    all_values.append(str(eci))
    all_values.append(str(value))
    all_values.append(str(ber))
    all_values.append(str(ds_99))
    all_values.append('-')
    all_values.append('-')
    all_values.append('-')

    file_name="Total_result_"+str(uf)+"_train_"+str(bench_train)+"_"+str(eci)+"_with_err_corr_both_algo_"+str(ds_999)+"_access_other_blocks_"+str(other_blocks_999)+".txt"
    read_file_path="results_analysis_scripts/"+file_name
    with open (read_file_path,"r") as fl:
        for line in fl:
            if "estimated_bandwidth_with_both_algo:" in line:
                line=line.strip()
                line=line.split(" ")
                bw=line[11]
                value = bw.replace("Kbps", "")
                ber=int(line[1])/(500*512)
    all_values.append(str(eci))
    all_values.append(str(value))
    all_values.append(str(ber))
    all_values.append(str(ds_999))
    all_values.append('-')
    all_values.append('-')
    all_values.append('-')

    with open("bwerror_99_999.txt", "a") as file:
        file.write(" ".join(str(val) for val in all_values) + "\n")
