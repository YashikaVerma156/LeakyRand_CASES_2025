import os
import re
import subprocess
import sys

def process_files_in_directory(directory):
    rsa_receiver_latency=[]
    aes_receiver_latency=[]
    rsa_chunks_with_misses=[]
    aes_chunks_with_misses=[]
    string1='receiver_1_new_cov_ch_1.gz'
    string2='rsa_encrypt_decrypt_with_wait.gz_'
    filtered_files = [file for file in os.listdir(directory) if string1 in file and string2 in file]
    #print(filtered_files)
    for filename in filtered_files:
        file_path = os.path.join(directory, filename)
        #print(filename)

        command = f'grep "RDTSC" {file_path} | head -n 1 | awk \'{{ print $3 }}\' '
        result = subprocess.run(command, shell=True, capture_output=True, text=True)

        # Strip and split the output
        lines = result.stdout.strip().split("\n")

        # Remove empty lines
        lines = [line for line in lines if line]
        if len(lines) == 0:
            continue
        rsa_receiver_latency.append(int(lines[0]))
    #print(rsa_receiver_latency)
    rsa_receiver_latency.sort()
    print(rsa_receiver_latency)
    string1='receiver_1_new_cov_ch_1.gz'
    string2='aes_encrypt_decrypt_with_wait.gz_'
    filtered_files = [file for file in os.listdir(directory) if string1 in file and string2 in file]
    #print(filtered_files)
    for filename in filtered_files:
        file_path = os.path.join(directory, filename)
        #print(filename)

        command = f'grep "RDTSC" {file_path} | head -n 1 | awk \'{{ print $3 }}\' '
        result = subprocess.run(command, shell=True, capture_output=True, text=True)

        # Strip and split the output
        lines = result.stdout.strip().split("\n")

        # Remove empty lines
        lines = [line for line in lines if line]
        if len(lines) == 0:
            continue
        aes_receiver_latency.append(int(lines[0]))
    #print(aes_receiver_latency)
    aes_receiver_latency.sort()
    print(aes_receiver_latency)

    string1='receiver_1_new_cov_ch_lr_probe.gz'
    string2='aes_encrypt_decrypt_with_wait.gz_'
    filtered_files = [file for file in os.listdir(directory) if string1 in file and string2 in file]
    #print(filtered_files)
    for filename in filtered_files:
        file_path = os.path.join(directory, filename)
        #print(filename)

        command = f'grep "NUM_CHUNKS_WITH_MISSES:" {file_path} | awk \'{{ print $2 }}\' '
        result = subprocess.run(command, shell=True, capture_output=True, text=True)

        # Strip and split the output
        lines = result.stdout.strip().split("\n")

        # Remove empty lines
        lines = [line for line in lines if line]
        if len(lines) == 0:
            continue
        aes_chunks_with_misses.append(int(lines[0]))
    #print(aes_chunks_with_misses)
    aes_chunks_with_misses.sort()
    print(aes_chunks_with_misses)

    string1='receiver_1_new_cov_ch_lr_probe.gz'
    string2='rsa_encrypt_decrypt_with_wait.gz_'
    filtered_files = [file for file in os.listdir(directory) if string1 in file and string2 in file]
    #print(filtered_files)
    for filename in filtered_files:
        file_path = os.path.join(directory, filename)
        #print(filename)

        command = f'grep "NUM_CHUNKS_WITH_MISSES:" {file_path} | awk \'{{ print $2 }}\' '
        result = subprocess.run(command, shell=True, capture_output=True, text=True)

        # Strip and split the output
        lines = result.stdout.strip().split("\n")

        # Remove empty lines
        lines = [line for line in lines if line]
        if len(lines) == 0:
            continue
        rsa_chunks_with_misses.append(int(lines[0]))
    #print(rsa_chunks_with_misses)
    rsa_chunks_with_misses.sort()
    print(rsa_chunks_with_misses)

directories_to_process=['results_new']

for directory in directories_to_process:
    if os.path.exists(directory):
        process_files_in_directory(directory)
