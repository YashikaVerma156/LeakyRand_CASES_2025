import os

# Specify the file name or path
file_path = "bwerror_spp_with_without.txt"

# Check if the file exists before deleting
if os.path.exists(file_path):
    os.remove(file_path)
    print(f"{file_path} has been deleted.")
else:
    print(f"{file_path} does not exist.")

with open(file_path, "w") as fl:
    fl.write("#Ds     DsA     BW      BER             OptTh    BW BER\n")

sender_sz_10_per="10%"
sender_sz_20_per="20%"
sender_sz_30_per="30%"
sender_sz_40_per="40%"
sender_sz_50_per="50%"

sender_sz_10=3276
sender_sz_20=6554
sender_sz_30=9830
sender_sz_40=13108
sender_sz_50=16384

pattern="error_count_sender_arr_size_"

# Read bandwidth numbers for SPP.
file_bw="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_with_ref/bandwidth_3276.txt"
with open(file_bw,"r") as fl:
    line_number=0
    for line in fl:
        line=line.strip()
        line=line.split(" ")
        bw_spp_with_10=line[1]

th_fl="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_train_with_ref/calculate_optimal_threshold/temp_optimal_threshold_3276.txt"
with open(th_fl,"r") as fl:
    for line in fl:
        line=line.strip()
        line=line.split()
        with_th_10=line[5]

directory="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_with_ref/extracted_data"
for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        if (pattern in filename) and ("3276" in filename) and (with_th_10 in filename):
            line_number=0
            with open(filepath,"r") as fl:
                for line in fl:
                    if "Error is: " in line:
                        line_number += 1
                    if "Total Error: " in line:
                        line=line.strip()
                        line=line.split(" ")
                        total_err=int(line[2])
ber_spp_with_10=(total_err/(line_number*512))

file_bw="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_with_ref/bandwidth_6554.txt"
with open(file_bw,"r") as fl:
    line_number=0
    for line in fl:
        line=line.strip()
        line=line.split(" ")
        bw_spp_with_20=line[1]

th_fl="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_train_with_ref/calculate_optimal_threshold/temp_optimal_threshold_6554.txt"
with open(th_fl,"r") as fl:
    for line in fl:
        line=line.strip()
        line=line.split()
        with_th_20=line[5]

directory="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_with_ref/extracted_data"
for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        if (pattern in filename) and ("6554" in filename) and (with_th_20 in filename):
            line_number=0
            with open(filepath,"r") as fl:
                for line in fl:
                    if "Error is: " in line:
                        line_number += 1
                    if "Total Error: " in line:
                        line=line.strip()
                        line=line.split(" ")
                        total_err=int(line[2])
ber_spp_with_20=(total_err/(line_number*512))

file_bw="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_with_ref/bandwidth_9830.txt"
with open(file_bw,"r") as fl:
    line_number=0
    for line in fl:
        line=line.strip()
        line=line.split(" ")
        bw_spp_with_30=line[1]

th_fl="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_train_with_ref/calculate_optimal_threshold/temp_optimal_threshold_9830.txt"
with open(th_fl,"r") as fl:
    for line in fl:
        line=line.strip()
        line=line.split()
        with_th_30=line[5]

directory="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_with_ref/extracted_data"
for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        if (pattern in filename) and ("9830" in filename) and (with_th_30 in filename):
            line_number=0
            with open(filepath,"r") as fl:
                for line in fl:
                    if "Error is: " in line:
                        line_number += 1
                    if "Total Error: " in line:
                        line=line.strip()
                        line=line.split(" ")
                        total_err=int(line[2])
ber_spp_with_30=(total_err/(line_number*512))

file_bw="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_with_ref/bandwidth_13108.txt"
with open(file_bw,"r") as fl:
    line_number=0
    for line in fl:
        line=line.strip()
        line=line.split(" ")
        bw_spp_with_40=line[1]

th_fl="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_train_with_ref/calculate_optimal_threshold/temp_optimal_threshold_13108.txt"
with open(th_fl,"r") as fl:
    for line in fl:
        line=line.strip()
        line=line.split()
        with_th_40=line[5]

directory="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_with_ref/extracted_data"
for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        if (pattern in filename) and ("13108" in filename) and (with_th_40 in filename):
            line_number=0
            with open(filepath,"r") as fl:
                for line in fl:
                    if "Error is: " in line:
                        line_number += 1
                    if "Total Error: " in line:
                        line=line.strip()
                        line=line.split(" ")
                        total_err=int(line[2])
ber_spp_with_40=(total_err/(line_number*512))

file_bw="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_with_ref/bandwidth_16384.txt"
with open(file_bw,"r") as fl:
    line_number=0
    for line in fl:
        line=line.strip()
        line=line.split(" ")
        bw_spp_with_50=line[1]

th_fl="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_train_with_ref/calculate_optimal_threshold/temp_optimal_threshold_16384.txt"
with open(th_fl,"r") as fl:
    for line in fl:
        line=line.strip()
        line=line.split()
        with_th_50=line[5]

directory="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_with_ref/extracted_data"
for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        if (pattern in filename) and ("16384" in filename) and (with_th_50 in filename):
            line_number=0
            with open(filepath,"r") as fl:
                for line in fl:
                    if "Error is: " in line:
                        line_number += 1
                    if "Total Error: " in line:
                        line=line.strip()
                        line=line.split(" ")
                        total_err=int(line[2])
ber_spp_with_50=(total_err/(line_number*512))

file_bw="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_without_ref/bandwidth_3276.txt"
with open(file_bw,"r") as fl:
    line_number=0
    for line in fl:
        line=line.strip()
        line=line.split(" ")
        bw_spp_without_10=line[1]

th_fl="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_train_without_ref/calculate_optimal_threshold/temp_optimal_threshold_3276.txt"
with open(th_fl,"r") as fl:
    for line in fl:
        line=line.strip()
        line=line.split()
        without_th_10=line[5]

directory="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_without_ref/extracted_data"
for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        if (pattern in filename) and ("3276" in filename) and (without_th_10 in filename):
            line_number=0
            with open(filepath,"r") as fl:
                for line in fl:
                    if "Error is: " in line:
                        line_number += 1
                    if "Total Error: " in line:
                        line=line.strip()
                        line=line.split(" ")
                        total_err=int(line[2])
ber_spp_without_10=(total_err/(line_number*512))

file_bw="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_without_ref/bandwidth_6554.txt"
with open(file_bw,"r") as fl:
    line_number=0
    for line in fl:
        line=line.strip()
        line=line.split(" ")
        bw_spp_without_20=line[1]

th_fl="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_train_without_ref/calculate_optimal_threshold/temp_optimal_threshold_6554.txt"
with open(th_fl,"r") as fl:
    for line in fl:
        line=line.strip()
        line=line.split()
        without_th_20=line[5]

directory="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_without_ref/extracted_data"
for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        if (pattern in filename) and ("6554" in filename) and (without_th_20 in filename):
            line_number=0
            with open(filepath,"r") as fl:
                for line in fl:
                    if "Error is: " in line:
                        line_number += 1
                    if "Total Error: " in line:
                        line=line.strip()
                        line=line.split(" ")
                        total_err=int(line[2])
ber_spp_without_20=(total_err/(line_number*512))

file_bw="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_without_ref/bandwidth_9830.txt"
with open(file_bw,"r") as fl:
    line_number=0
    for line in fl:
        line=line.strip()
        line=line.split(" ")
        bw_spp_without_30=line[1]

th_fl="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_train_without_ref/calculate_optimal_threshold/temp_optimal_threshold_9830.txt"
with open(th_fl,"r") as fl:
    for line in fl:
        line=line.strip()
        line=line.split()
        without_th_30=line[5]

directory="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_without_ref/extracted_data"
for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        if (pattern in filename) and ("9830" in filename) and (without_th_30 in filename):
            line_number=0
            with open(filepath,"r") as fl:
                for line in fl:
                    if "Error is: " in line:
                        line_number += 1
                    if "Total Error: " in line:
                        line=line.strip()
                        line=line.split(" ")
                        total_err=int(line[2])
ber_spp_without_30=(total_err/(line_number*512))

file_bw="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_without_ref/bandwidth_13108.txt"
with open(file_bw,"r") as fl:
    line_number=0
    for line in fl:
        line=line.strip()
        line=line.split(" ")
        bw_spp_without_40=line[1]

th_fl="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_train_without_ref/calculate_optimal_threshold/temp_optimal_threshold_13108.txt"
with open(th_fl,"r") as fl:
    for line in fl:
        line=line.strip()
        line=line.split()
        without_th_40=line[5]

directory="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_without_ref/extracted_data"
for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        if (pattern in filename) and ("13108" in filename) and (without_th_40 in filename):
            line_number=0
            with open(filepath,"r") as fl:
                for line in fl:
                    if "Error is: " in line:
                        line_number += 1
                    if "Total Error: " in line:
                        line=line.strip()
                        line=line.split(" ")
                        total_err=int(line[2])
ber_spp_without_40=(total_err/(line_number*512))

file_bw="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_without_ref/bandwidth_16380.txt"
with open(file_bw,"r") as fl:
    line_number=0
    for line in fl:
        line=line.strip()
        line=line.split(" ")
        bw_spp_without_50=line[1]

th_fl="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_train_without_ref/calculate_optimal_threshold/temp_optimal_threshold_16380.txt"
with open(th_fl,"r") as fl:
    for line in fl:
        line=line.strip()
        line=line.split()
        without_th_50=line[5]

directory="/home/yashikav/Desktop/Mirage_project/fig4b/spp_results_analysis_scripts_test_without_ref/extracted_data"
for filename in os.listdir(directory):
        filepath = os.path.join(directory, filename)
        if (pattern in filename) and ("16380" in filename) and (without_th_50 in filename):
            line_number=0
            with open(filepath,"r") as fl:
                for line in fl:
                    if "Error is: " in line:
                        line_number += 1
                    if "Total Error: " in line:
                        line=line.strip()
                        line=line.split(" ")
                        total_err=int(line[2])
ber_spp_without_50=(total_err/(line_number*512))


#Write to corresponding text file as per the plot. TODO Do CRFill and CRProbe for 2MB.
with open(file_path, "w") as fl:
    fl.write(f"{sender_sz_10_per} {sender_sz_10} {bw_spp_with_10} {ber_spp_with_10} {with_th_10} {bw_spp_without_10} {ber_spp_without_10} \n")
    fl.write(f"{sender_sz_20_per} {sender_sz_20} {bw_spp_with_20} {ber_spp_with_20} {with_th_20} {bw_spp_without_20} {ber_spp_without_20} \n")
    fl.write(f"{sender_sz_30_per} {sender_sz_30} {bw_spp_with_30} {ber_spp_with_30} {with_th_30} {bw_spp_without_30} {ber_spp_without_30} \n")
    fl.write(f"{sender_sz_40_per} {sender_sz_40} {bw_spp_with_40} {ber_spp_with_40} {with_th_40} {bw_spp_without_40} {ber_spp_without_40} \n")
    fl.write(f"{sender_sz_50_per} {sender_sz_50} {bw_spp_with_50} {ber_spp_with_50} {with_th_50} {bw_spp_without_50} {ber_spp_without_50} \n")


