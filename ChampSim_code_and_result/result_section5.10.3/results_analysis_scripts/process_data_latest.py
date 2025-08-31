import os
import re
import subprocess
import sys
import pandas as pd
import draw_plot
import matplotlib.pyplot as plt

def extract_random_seed():

    pattern1="seed_number is: 2249"
    with open(file_path, "r") as file:
        for line_number, line in enumerate(file, start=1):
            if re.search(pattern1, line):
                line = line.strip('\n')
                line = line.split(" ")
                receiver_misses.append(int(line[7]))
                sender_misses.append(int(line[8]))

def process_files_in_directory(directory, benchmark_file, benchmark_type, seeds_file, file_size):
    print("benchmark_type: ",benchmark_type)
    #exit()
    total_err_cnt=0
    string2="_cov_ch_51.gz"
    string3="_cov_ch_1.gz"
    string5="_16_other_blocks_326_1_514_with_both"
    trace1="in_c_md5hash_"+str(file_size)+"_"
    threshold=0
    filtered_files2 = [file for file in os.listdir(directory) if string2 in file and trace1 in file]
    filtered_files3 = [file for file in os.listdir(directory) if string3 in file and trace1 in file]
    filtered_files5 = [file for file in os.listdir(directory) if string5 in file and trace1 in file]
    print("filtered_files2, number of files: ",len(filtered_files2))
    print("filtered_files3, number of files: ",len(filtered_files3))
    print("filtered_files5, number of files: ",len(filtered_files5))

    print("==========lr_controlled=======")
    # Construct the command
    chunks_with_misses_1=[]
    with open(seeds_file, "r") as seedfile:
        for line_number, line in enumerate(seedfile, start=1):
            if(line_number == 1):
                continue
            line=line.strip()
            extracted_seed=line.split(',')[32]
            #print(extracted_seed)
            #exit()
            filename1=""
            filename2=""
            for filename in filtered_files5:
                #print(filename)
                if extracted_seed in filename and trace1 in filename:
                    #print("found")
                    filename1=filename
            filepath1 = os.path.join(directory, filename1)
            if os.path.isfile(filepath1):
                command = f"grep -nri 'cpu 1 is on wait' {filepath1} | head -n 2 | tail -n 1 | awk -F ':' '{{ print $1 }}' "
                result=subprocess.run(command, shell=True, capture_output=True, text=True)
                line_num1=result.stdout.strip()
                #print(line_num1)
                command = f"grep -nri 'cpu 0 is on wait' {filepath1} | head -n 2 | tail -n 1 | awk -F ':' '{{ print $1 }}' "
                result=subprocess.run(command, shell=True, capture_output=True, text=True)
                line_num2=result.stdout.strip()
                #print(line_num2)
                command=f"sed -n '{line_num1},{line_num2}p' {filepath1} | grep -ni 'RDTSC, sub:' | awk '$3 > 124' | wc"
                result=subprocess.run(command, shell=True, capture_output=True, text=True)
                chunks_with_misses1=result.stdout.strip().split()
                chunks_with_misses_1.append(int(chunks_with_misses1[0]))
            else:
                #print("filepath1 is invalid.")
                continue
    result_file = "lr_like_controlled_probe_"+str(file_size)+"_"+str(benchmark_type)+".txt"
    # Create the file path
    file_path = os.path.join(file_dir, result_file)
    # Convert to DataFrame
    df = pd.DataFrame({'md5hash_num_missed_chunks': chunks_with_misses_1})
    # Save to CSV
    df.to_csv(file_path, index=False)
    chunks_with_misses_1.sort()
    #draw_plot.draw_hist_30(chunks_with_misses_1)
    #exit()
    #print(chunks_with_misses_1)
    print("md5_chunks_with_misses, ","min: ",chunks_with_misses_1[0],"max: ",chunks_with_misses_1[-1])


    print("==========lrgen=======")
    probe_cycles1=[]
    # Construct the command
    with open(seeds_file, "r") as seedfile:
        for line_number, line in enumerate(seedfile, start=1):
            if(line_number == 1):
                continue
            line=line.strip()
            extracted_seed=line.split(',')[32]
            #print(extracted_seed)
            #exit()
            filename1=""
            for filename in filtered_files2:
                #print(filename)
                if extracted_seed in filename and trace1 in filename:
                    #print("found")
                    filename1=filename
            #exit()
            filepath1 = os.path.join(directory, filename1)
            #print(filepath1)
            if os.path.isfile(filepath1):
                command = f"grep -nri 'cpu 0 is on wait' {filepath1} | head -n 2 | tail -n 1 | awk '{{ print $12 }}' "
                result=subprocess.run(command, shell=True, capture_output=True, text=True)
                cycle1=result.stdout.strip()
                #print(cycle1)
                command = f"grep -nri 'cpu 1 is on wait' {filepath1} | head -n 2 | tail -n 1 | awk '{{ print $12 }}' "
                result=subprocess.run(command, shell=True, capture_output=True, text=True)
                cycle2=result.stdout.strip()
                probe_cycles1.append(int(cycle1) - int(cycle2))
                #print(cycle2)
                #probe_cycles1=int(cycle1) - int(cycle2)
            else:
                #print("filepath1 is invalid.")
                continue
            #exit()
    result_file = "lrgen_like_probe_"+str(file_size)+"_"+str(benchmark_type)+".txt"
    # Create the file path
    file_path = os.path.join(file_dir, result_file)
    # Convert to DataFrame
    df = pd.DataFrame({'md5hash_probe_latency': probe_cycles1})
    # Save to CSV
    df.to_csv(file_path, index=False)
    probe_cycles1.sort()
    print("================ lrgen like probe ================")
    #print(probe_cycles1)
    print("md5_probe_latency, ","min: ",probe_cycles1[0],"max: ",probe_cycles1[-1])

    # Construct the command
    probe_cycles1=[]
    with open(seeds_file, "r") as seedfile:
        for line_number, line in enumerate(seedfile, start=1):
            if(line_number == 1):
                continue
            line=line.strip()
            extracted_seed=line.split(',')[32]
            #print(extracted_seed)
            #exit()
            filename1=""
            filename2=""
            for filename in filtered_files3:
                #print(filename)
                if extracted_seed in filename and trace1 in filename:
                    #print("found")
                    filename1=filename
            #exit()
            filepath1 = os.path.join(directory, filename1)
            #print(filepath1)
            if os.path.isfile(filepath1):
                command = f"grep -nri 'cpu 0 is on wait' {filepath1} | head -n 2 | tail -n 1 | awk '{{ print $12 }}' "
                result=subprocess.run(command, shell=True, capture_output=True, text=True)
                cycle1=result.stdout.strip()
                #print(cycle1)
                command = f"grep -nri 'cpu 1 is on wait' {filepath1} | head -n 2 | tail -n 1 | awk '{{ print $12 }}' "
                result=subprocess.run(command, shell=True, capture_output=True, text=True)
                cycle2=result.stdout.strip()
                #print(cycle2)
                probe_cycles1.append(int(cycle1) - int(cycle2))
            else:
                #print("filepath1 is invalid.")
                continue
    result_file = "spp_like_probe_"+str(file_size)+"_"+str(benchmark_type)+".txt"
    # Create the file path
    file_path = os.path.join(file_dir, result_file)
    # Convert to DataFrame
    df = pd.DataFrame({'md5hash_probe_latency': probe_cycles1})
    # Save to CSV
    df.to_csv(file_path, index=False)
    probe_cycles1.sort()
    print("================ SPP like probe ================")
    #print(probe_cycles1)
    print("md5_probe_latency, ","min: ",probe_cycles1[0],"max: ",probe_cycles1[-1])
### total arguments
n = len(sys.argv)

if n != 3:
#    print("The execution commands should have two arguments: ")
    print("benchmark_type: 0 for train suite, 1 for test suite")
    print("filesize: 8, 9, 10, 12, 16, 20 etc.")
#    print("|DS| size: till now tested sizes are 9830, 6554, 6250, 6000 and 5500")
    exit()

#directories_to_process = ['NEW_COV_CH_2core_'+str(sys.argv[2])+'_intelligent_sender_version1']
directories_to_process = ['../total_results']

benchmark_dir="benchmark"
seeds_dir="seeds"
if int(sys.argv[1]) == 0:
    bnchmrk_typ="train"
    benchmarks="benchmark_train.txt"
    seeds="seeds_train.txt"
elif int(sys.argv[1]) == 1:
    bnchmrk_typ="test"
    benchmarks="benchmark_test.txt"
    seeds="seeds_test.txt"
benchmark_file = os.path.join(benchmark_dir, benchmarks)
seeds_file = os.path.join(seeds_dir, seeds)

filesize=int(sys.argv[2])

print("PROCESSING FOR BENCHMARK TYPE: ", bnchmrk_typ)
print(directories_to_process)


# 2. Process files in each directory
for directory in directories_to_process:
    if os.path.exists(directory):

      ##### Create a file for later use.
        # File path
        #sender_array_size=directory.split('_')[3]
        msg_size=512
        file_dir="extracted_data"

        # Get the current working directory
        cwd = os.getcwd()
        # Join the cwd and directory name to get the full path of the directory
        directory_path = os.path.join(cwd, file_dir)
        # Create the directory 
        try:
            os.mkdir(directory_path)
            print("Directory created successfully.")
        except OSError as error:
            print(f"Failed to create directory: {error}")
        
        #print(f"Processing files in directory: {directory}")
        process_files_in_directory(directory, benchmark_file, bnchmrk_typ, seeds_file, filesize)

    else:
        print(f"Directory '{directory}' does not exist.")
#seed_number is: 2249

