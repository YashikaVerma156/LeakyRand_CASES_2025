import os
import re
import subprocess
import sys


def extract_random_seed():

    pattern1="seed_number is: 2249"
    with open(file_path, "r") as file:
        for line_number, line in enumerate(file, start=1):
            if re.search(pattern1, line):
                line = line.strip('\n')
                line = line.split(" ")
                receiver_misses.append(int(line[7]))
                sender_misses.append(int(line[8]))

def process_files_in_directory(directory, benchmark_file, benchmark_type, file_dir):
    total_err_cnt=0
    filtered_files = [file for file in os.listdir(directory) if benchmark_type in file]
    print("number of files: ",len(filtered_files))
    #exit()
    for filename in filtered_files:
        if(benchmark_type not in benchmark_file or benchmark_type not in filename):
            print("benchmark_type and filename are not same. EXITING ...")
            exit()
        filepath = os.path.join(directory, filename)
        if os.path.isfile(filepath):
            # Process the file here (e.g., read, modify, analyze)
            #print(f"Processing file: {filepath}")

            # Extract seed number from the file.
            pattern1="seed_number is:"
            with open(filepath, "r") as file:
                for line_number, line in enumerate(file, start=1):
                    if re.search(pattern1, line):
                        line = line.strip('\n')
                        line = line.split(" ")
                        seed = str(line[2])
                        print(filepath)
                       # exit()
                        filename = filepath.split("/")[2]
                        #print(filename)
                        msg_size = str(filename.split("_")[5])
                        msg_string = str(filename.split("_")[13])
                        sender_arr_size = str(filename.split("_")[17])
                        history_length = str(filename.split("_")[18])
                        print('msg_size: ',msg_size,' msg_string: ', msg_string,' sender_arr_size: ',sender_arr_size)
                        #exit()
                        data_extract_script ='data_extraction_seed.py'
        #XXX below line is commented intentionally.
                        subprocess.run(["python3", data_extract_script, filepath, sender_arr_size, msg_size, msg_string, seed, history_length, file_dir])
                        #exit() 
                        #delta_calculation_script ='delta_calculation_seed.py'
                        #extracted_data_filename="output_" + str(sender_arr_size) +"_"+str(msg_string)+"_"+str(seed)+"_"+str(msg_size)+".txt"
                        #extracted_data_dir="extracted_data"
                        #extracted_file_path = os.path.join(extracted_data_dir, extracted_data_filename)
        #XXX below line is commented intentionally.
                        #subprocess.run(["python3", delta_calculation_script, extracted_file_path, sender_arr_size, msg_size, msg_string, seed, benchmark_file])
       ######   Error calculation   ######
                        #thresholds=[-500,-1000,-1500,-2000,-3000,-4000,-5000]
                        #thresholds=[-5500, -6000, -7000, -8000]
                        #thresholds=[265000]
                        #thresholds=[209015]
                        #thresholds=[221955]
                        #print("sender_arr_size: ",sender_arr_size)
                        #if int(sender_arr_size) == 1638:
                        #    thresholds=[195708]
                        #thresholds=[195708]

                        #thresholds=[-3150, -3250, -3350, -3400, -3450]    #[-5500]
                        #thresholds=[100, 0, -100, -200, -500, -1000, -1500, -2000, -3000, -4000, -5000, -6000, -7000, -8000]
                        #for th in thresholds:
                        #    error_calculation_script ='error_calculation_seed.py'
                        #    extracted_data_filename="output_" + str(sender_arr_size) +"_"+str(msg_string)+"_"+str(seed)+"_"+str(msg_size)+".txt"
                        #    extracted_data_dir="extracted_data"
                        #    extracted_file_path = os.path.join(extracted_data_dir, extracted_data_filename)
        #XXX below line is commented intentionally.
                        #    subprocess.run(["python3", error_calculation_script, extracted_file_path, sender_arr_size, msg_size, msg_string, seed, str(th), benchmark_file])

                        #exit()
                        #min_max_script ='min_max_latency_diff.py'
                        #extracted_data_filename="output_" + str(sender_arr_size) +"_"+str(msg_string)+"_"+str(seed)+"_"+str(msg_size)+".txt"
                        #extracted_data_dir="extracted_data"
                        #extracted_file_path = os.path.join(extracted_data_dir, extracted_data_filename)
                        #subprocess.run(["python3", min_max_script, extracted_file_path, sender_arr_size, msg_size, msg_string, seed, benchmark_file])
                        #exit()

######   Add errors for whole directory   ######
    #thresholds=[265000]
    #thresholds=[209015]
    #thresholds=[221955]
    #if sender_arr_size == 1638:
    #    thresholds=[195708]
    #thresholds=[195708]
    #for th in thresholds:
    #    total_err_cnt=0
    #    zero_to_one=0
    #    one_to_zero=0
        # File path
    #    err_file_dir="extracted_data"
    #    err_result_file = "error_count_sender_arr_size_"+str(sender_arr_size)+"_msg_size"+"_"+str(msg_size)+"_th_"+str(th)+".txt"
        # Create the file path
    #    err_file_path = os.path.join(err_file_dir, err_result_file)
    #    str_to_match="Error is:"
    #    with open(err_file_path, 'r') as file:
    #        for line_number, line in enumerate(file, start=1):
    #            if re.search(str_to_match, line):
    #                err=int(line.split(" ")[2])
    #                err01=int(line.split(" ")[4])
    #                err10=int(line.split(" ")[6])
    #                total_err_cnt+= err
    #                zero_to_one+= err01
    #                one_to_zero+= err10
    #    new_data='Total Error: '+str(total_err_cnt)+' Total zero_to_one: '+str(zero_to_one)+' Total one_to_zero: '+str(one_to_zero)
        # Open the file in append mode and write new data
        #XXX below lines are commented intentionally.
    #    with open(err_file_path, 'a') as file:
    #        file.write(new_data+ '\n')  # Add a newline after the new data

# 1. List of directories to process
#directories_to_process = ['sender_arr_size_3276', 'sender_arr_size_6554', 'sender_arr_size_9830', 'sender_arr_size_13108', 'sender_arr_size_16380']
#directories_to_process = [ 'sender_arr_size_13108', 'sender_arr_size_16380']
#directories_to_process = [ 'sender_arr_size_409']
#directories_to_process = [ 'sender_arr_size_819']
#directories_to_process = [ 'sender_arr_size_1638']
#directories_to_process = [ 'sender_arr_size_3276']
#directories_to_process = [ 'sender_arr_size_4000']
#directories_to_process = [ 'sender_arr_size_4500']
#directories_to_process = [ 'sender_arr_size_5000']
#directories_to_process = [ 'sender_arr_size_5500']
#directories_to_process = [ 'sender_arr_size_6000']
#directories_to_process = [ 'sender_arr_size_6554']
#directories_to_process = [ 'sender_arr_size_9830']
#directories_to_process = [ 'sender_arr_size_13108']
#directories_to_process = [ 'sender_arr_size_16384']
#directories_to_process = [ 'NEW_COV_CH_2core_9830']
#directories_to_process = [ 'NEW_COV_CH_2core_6554']
#directories_to_process = [ 'NEW_COV_CH_2core_5500']

#### total arguments
n = len(sys.argv)

if n < 3:
    print("The execution commands should have two arguments: ")
    print("benchmark_type: 0 for train suite, 1 for test suite")
    print("|DS| size: till now tested sizes are 9830, 6554, 6250, 6000 and 5500")
    exit()
directories_to_process = ['../NEW_COV_CH_2core_'+str(sys.argv[2])]
#directories_to_process = ['NEW_COV_CH_2core_'+str(sys.argv[2])+'_53_and_1probeRound']
#directories_to_process = ['NEW_COV_CH_2core_'+str(sys.argv[2])+'_intelligent_sender_version2_corrected']

benchmark_dir="benchmark"
if int(sys.argv[1]) == 0:
    bnchmrk_typ="train"
    benchmarks="benchmark_train.txt"
    file_dir="data_6250_train_suite_version2_corrected"
elif int(sys.argv[1]) == 1:
    bnchmrk_typ="test"
    benchmarks="benchmark_test.txt"
    file_dir="data_6250_test_suite_version2_corrected"
benchmark_file = os.path.join(benchmark_dir, benchmarks)

print("PROCESSING FOR BENCHMARK TYPE: ", bnchmrk_typ)
print(directories_to_process)

# 2. Process files in each directory
for directory in directories_to_process:
    if os.path.exists(directory):

      ##### Create a file for later use.
        # File path
        sender_array_size=directory.split('_')[4]
        print(sender_array_size)
        #exit()
        msg_size=512

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
        
        #result_file = "min_max_latency_difference_sender_arr_size_"+str(sender_array_size)+"_msg_size"+"_"+str(msg_size)+".txt"
        # Create the file path
        #file_path = os.path.join(file_dir, result_file)
        # Open the file in append mode and write new data
        #with open(file_path, 'w') as f:
        #    f.write('min_bit0,max_bit0,min_bit1,max_bit1,msg_string,seed,bit_num_min0,bit_num_max0,bit_num_min1,bit_num_max1'+'\n')  # Add a newline after the new data

        #print(f"Processing files in directory: {directory}")
        process_files_in_directory(directory, benchmark_file, bnchmrk_typ, file_dir)

    else:
        print(f"Directory '{directory}' does not exist.")
#seed_number is: 2249

