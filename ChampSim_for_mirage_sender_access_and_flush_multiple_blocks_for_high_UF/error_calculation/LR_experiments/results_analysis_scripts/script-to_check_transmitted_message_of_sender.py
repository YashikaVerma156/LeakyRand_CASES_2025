import os

def identify(file_path, string_number, train_data, string_len, atp, sen_dis):
## Read the actual message.
    original_msg=[]   ## Original message after including the marker bits.
    original_msg1=[]  ##Intermediate message read from benchmark_<test/train>.txt
    MAX_INC_IN_ARR=int(string_len/(atp-1))
    NUM_BITS_WITH_MARKER_BITS= int(string_len + (2*MAX_INC_IN_ARR))
    if train_data == 1:
        with open('benchmark/benchmark_train.txt', "r") as file:
            for line_number, line in enumerate(file, start=1):
                if(line_number == string_number):
                    line = line.strip()
                    # Split the string into single characters and assign to an array
                    original_msg1 = [char for char in line]
                    #print(original_msg1,' ',line_number)
    #print("length of org_msg1 ", len(original_msg1))
    if train_data == 0:
        with open('benchmark/benchmark_test.txt', "r") as file:
            for line_number, line in enumerate(file, start=1):
                if(line_number == string_number):
                    line = line.strip()
                    # Split the string into single characters and assign to an array
                    original_msg1 = [char for char in line]
                   # print(original_msg,' ',line_number)
    err=0
    i=0
    for num_bits in range(0,NUM_BITS_WITH_MARKER_BITS):
        if(err+1) % atp == 0:
            original_msg.append(1)
            err+=1
        elif((err+1) % atp == 1 and err > 1):
            original_msg.append(0)
            err = 0
        else:
            #print("2. length of org_msg1 ", len(original_msg1)," i is: ",i," err: ",err, " atp: ",atp," num_bits: ",num_bits)
            original_msg.append(original_msg1[i])
            i+=1
            err+=1

            ## Identify if the sent mesage is indeed the one meant to send.

    sender_sent_msg=[]
    set_up_done=0
    turn_cpu1=0
    bit_duration_start = 0
    bit_duration_end = 0
    pattern1=r'cpu 0 is awakened from sleep'
    pattern2=r'clflush called on cpu: 0'
    pattern3=r'cpu 0 is on wait'
    pattern4=r'=========cache fill is done========'
    with open(file_path, "r") as file:
        for line_number, line in enumerate(file, start=1):
            line=line.strip()
            if pattern1 in line:
                turn_cpu1 = 1
            if turn_cpu1 > 0 and pattern2 in line:
                turn_cpu1 += 1
            if pattern3 in line and turn_cpu1 > 0:
                if turn_cpu1 == 1:
                    sender_sent_msg.append(0)
                if turn_cpu1 > 1:
                    sender_sent_msg.append(1)
                    if (turn_cpu1 != 2*sen_dis+1):  # Pattern 2 is printed twice per flush of cpu.
                        print("issue sen_dis: " ,sen_dis," turn_cpu1: ",turn_cpu1, " atp: ",atp)
                        exit()
                turn_cpu1 = 0

    for i in range(0, NUM_BITS_WITH_MARKER_BITS):
        if(int(original_msg[i]) != int(sender_sent_msg[i])):
            #print("sender_sent_msg: ")
            #print(sender_sent_msg)
            #print("original_msg: ")
            #print(original_msg)
            print("i is: ",i," actual_msg_bit: ",original_msg[i]," sent_bit: ", sender_sent_msg[i])
            print("========== Something is not same between the sender_sent_msg and original_msg ============")
            #exit(0)
       
    #print("sender_sent_msg: ")
    #print(sender_sent_msg)
    #print("original_msg: ")
    #print(original_msg)
    
file_path='../results/result_random_1_champsim.trace_sender_512_62_19565_128_with_both_algo_4_access_train.gz.txt'

UFs=[16]
Algo_trigger_point=[128, 64, 32, 16, 8]
#Algo_trigger_point=[128]
sender_disturbances=[1,2,3,4]

directory_path='../results'

file_list = os.listdir(directory_path)

for UF in UFs:
    for atp in Algo_trigger_point:
        for sen_dis in sender_disturbances:
            filtered_files = [file for file in file_list if '_1_champsim.trace_sender_' in file and '_'+str(atp)+'_with_both_algo_' in file and '_algo_'+str(sen_dis)+'_access_' in file]
            if(len(filtered_files) != 1):
                print('issue: len(filtered_files) is: ',len(filtered_files),' ',atp,' ',sen_dis)
                print(len(filtered_files), filtered_files)
                exit()
            #print(len(filtered_files), filtered_files)
            for file_name in filtered_files:
                file_path = os.path.join(directory_path, file_name)
                print("atp: ",atp," sen_dis: ",sen_dis)
                identify(file_path, 64, 1, 512, atp, sen_dis)
