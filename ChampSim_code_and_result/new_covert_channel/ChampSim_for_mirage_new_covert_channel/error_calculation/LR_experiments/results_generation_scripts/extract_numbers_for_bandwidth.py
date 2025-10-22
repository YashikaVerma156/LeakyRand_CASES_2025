import os
import math

#size=[3276, 6554, 9830, 13108, 16380]
size=[6250]
dir_path='data_6250_test_suite_version2_corrected'
for sz in size:
    files=[]
    string='output_'+str(sz)+'_'
    for f in os.listdir(dir_path):
        if string in f:
            files.append(os.path.join(dir_path, f))
    #print(files)
    #print("Files found:",len(files))
    MAX_EXECUTED_CYCLES_DATA_BIT=0
    count = 0
    initial_overhead = 0 # Initial Cache Region Identification overhead.
    for f in files:
        with open(f, "r") as open_f:
            seen_0_once=0
            seen_1_once = 0
            for line_num, line in enumerate(open_f, start=1):
                if(line_num == 2):
                    if(initial_overhead < int(line.split(',')[1])):
                        initial_overhead = int(line.split(',')[1])
                if(line_num < 3):
                    continue
                #print(line)
                #print(f)
                #exit(0)
                # Checking if sender has executed once
                #if(int(line.split(',')[5]) < 100 and seen_0_once == 0):
                #    seen_0_once = 1
                #    continue
                #if(int(line.split(',')[5]) > 100 and seen_1_once == 0):
                #    seen_1_once = 1
                #    continue

                if(int(line.split(',')[2]) > MAX_EXECUTED_CYCLES_DATA_BIT):
                    MAX_EXECUTED_CYCLES_DATA_BIT = int(line.split(',')[2])
                    FILE_NAME_DATA_BIT = f
                    LINE_NUMBER_DATA_BIT = line_num
        count=count+1
    #print("Processed files count:",count)
    #if(len(files) != count or count != 750):
        #print("750 files should be processed, 250 for train suite and 500 for test suite.")
        #exit(0)
    number_of_bits_per_sec=4000000000/(MAX_EXECUTED_CYCLES_DATA_BIT)
    bandwidth=(number_of_bits_per_sec/1024)
    bandwidth=round(bandwidth,2)
    #print("Bandwidth: ",bandwidth,"Kbps")
    #print("MAX_EXECUTED_CYCLES_DATA_BIT: ",MAX_EXECUTED_CYCLES_DATA_BIT," FILE_NAME_DATA_BIT: ",FILE_NAME_DATA_BIT, " LINE_NUMBER_DATA_BIT: ",LINE_NUMBER_DATA_BIT)
    # Bandwidth with initial overhead.
    Total_cycles = MAX_EXECUTED_CYCLES_DATA_BIT*512 + (initial_overhead-525000)
    per_bit_cycles = math.ceil(Total_cycles/512)
    number_of_bits_per_sec=4000000000/(per_bit_cycles)
    bandwidth=(number_of_bits_per_sec/1024)
    bandwidth=round(bandwidth,2)
    #print("Bandwidth including initial overhead: ",bandwidth,"Kbps")
    print("\n \n================== \n Bandwidth observed is: ",bandwidth,"Kbps")
    #print("MAX_EXECUTED_CYCLES_DATA_BIT: ",MAX_EXECUTED_CYCLES_DATA_BIT," FILE_NAME_DATA_BIT: ",FILE_NAME_DATA_BIT, " LINE_NUMBER_DATA_BIT: ",LINE_NUMBER_DATA_BIT)

