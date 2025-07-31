import os

#size=[3276, 6554, 9830, 13108, 16384]
size=[4915, 9830, 19660]
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
    MAX_EXECUTED_CYCLES_REF_BIT=0
    count = 0
    for f in files:
        with open(f, "r") as open_f:
            sender_executed_once=0
            for line_num, line in enumerate(open_f, start=1):
                if(line_num < 3):
                    continue
                #print(line)
                # Checking if sender has executed once
                if(int(line.split(',')[5]) < 1000 and sender_executed_once == 0):
                    continue
                if(int(line.split(',')[5]) > 1000 and sender_executed_once == 0):
                    sender_executed_once = 1
                    continue

                if(int(line.split(',')[2]) > MAX_EXECUTED_CYCLES_DATA_BIT and int(line.split(',')[0])%2 == 0):
                    MAX_EXECUTED_CYCLES_DATA_BIT = int(line.split(',')[2])
                    FILE_NAME_DATA_BIT = f
                    LINE_NUMBER_DATA_BIT = line_num
                if(int(line.split(',')[2]) > MAX_EXECUTED_CYCLES_REF_BIT and int(line.split(',')[0])%2 == 1):
                    MAX_EXECUTED_CYCLES_REF_BIT = int(line.split(',')[2])
                    FILE_NAME_REF_BIT = f
                    LINE_NUMBER_REF_BIT = line_num
        count=count+1
        #print(count)
    number_of_bits_per_sec=4000000000/(MAX_EXECUTED_CYCLES_DATA_BIT+MAX_EXECUTED_CYCLES_REF_BIT)
    bandwidth=(number_of_bits_per_sec/1024)
    bandwidth=round(bandwidth,2)

    new_data="Bandwidth: "+str(bandwidth)
    output_file="bandwidth_"+str(sz)+".txt"
    # Open the file and write new data
    with open(output_file, 'w') as file:
        file.write(new_data+ '\n')  # Add a newline after the new data

    print("Bandwidth: ",bandwidth,"Kbps")
    print("MAX_EXECUTED_CYCLES_DATA_BIT: ",MAX_EXECUTED_CYCLES_DATA_BIT," FILE_NAME_DATA_BIT: ",FILE_NAME_DATA_BIT, " LINE_NUMBER_DATA_BIT: ",LINE_NUMBER_DATA_BIT)
    print("MAX_EXECUTED_CYCLES_REF_BIT: ",MAX_EXECUTED_CYCLES_REF_BIT," FILE_NAME_REF_BIT: ",FILE_NAME_REF_BIT, " LINE_NUMBER_REF_BIT: ",LINE_NUMBER_REF_BIT)
    
