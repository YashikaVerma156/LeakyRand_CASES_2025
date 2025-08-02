import os

size=[3276, 6554, 9830, 13108, 16380]
dir_path='extracted_data'
for sz in size:
    files=[]
    string='output_'+str(sz)+'_'
    for f in os.listdir(dir_path):
        if string in f:
            files.append(os.path.join(dir_path, f))
    #print(files)
    print(len(files))
    MAX_EXECUTED_CYCLES_DATA_BIT=0
    count = 0
    for f in files:
        with open(f, "r") as open_f:
            seen_0_once=0
            seen_1_once = 0
            for line_num, line in enumerate(open_f, start=1):
                if(line_num < 3):
                    continue
                #print(line)
                # Checking if sender has executed once
                if(int(line.split(',')[5]) < 100 and seen_0_once == 0):
                    seen_0_once = 1
                    continue
                if(int(line.split(',')[5]) > 100 and seen_1_once == 0):
                    seen_1_once = 1
                    continue

                if(int(line.split(',')[2]) > MAX_EXECUTED_CYCLES_DATA_BIT):
                    MAX_EXECUTED_CYCLES_DATA_BIT = int(line.split(',')[2])
                    FILE_NAME_DATA_BIT = f
                    LINE_NUMBER_DATA_BIT = line_num
        count=count+1
        #print(count)
    number_of_bits_per_sec=4000000000/(MAX_EXECUTED_CYCLES_DATA_BIT)
    bandwidth=(number_of_bits_per_sec/1024)
    bandwidth=round(bandwidth,2)

    new_data="Bandwidth: "+str(bandwidth)
    output_file="bandwidth_"+str(sz)+".txt"
    # Open the file and write new data
    with open(output_file, 'w') as file:
        file.write(new_data+ '\n')  # Add a newline after the new data

    print("Bandwidth: ",bandwidth,"Kbps")
    print("MAX_EXECUTED_CYCLES_DATA_BIT: ",MAX_EXECUTED_CYCLES_DATA_BIT," FILE_NAME_DATA_BIT: ",FILE_NAME_DATA_BIT, " LINE_NUMBER_DATA_BIT: ",LINE_NUMBER_DATA_BIT)
    
