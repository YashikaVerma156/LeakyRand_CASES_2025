import os

# This script calculates an optimal-threshold (that has the least number of errors).
threshold_dict={}
res_dir="extracted_data"
file_sizes=[20]

mas_file_list=[]
master_file_size=[8]
master_file_name="lr_like_controlled_probe_8_train.txt"
master_file_path = os.path.join(res_dir, master_file_name)
with open(master_file_path, "r") as mas_file:
    for (line_number, line) in enumerate(mas_file, start=1):
        if(line_number == 1):
            continue
        mas_file_list.append(int(line.strip()))
    mas_file_list.sort()
    #print(mas_file_list)
    print("Train suite, uf_16_controlled:")
    for file_size in file_sizes:
        #print(file_size)
        file_list=[]
        file_name="lr_like_controlled_probe_"+str(file_size)+"_train.txt"
        file_path = os.path.join(res_dir, file_name)
        with open(file_path, "r") as file1:
            for (line_number, line) in enumerate(file1, start=1):
                if(line_number == 1):
                    continue
                file_list.append(int(line.strip()))
        file_list.sort()
        #print(file_list)

        if(mas_file_list[-1] < file_list[0]):
            #print("non-overlapping")
            threshold=int((file_list[0] - mas_file_list[-1])/2) + mas_file_list[-1]
            smallest_err_cnt=0
            FS1_err_cnt=0
            FS2_err_cnt=0
            #print("threshold is: ", threshold," error: ",smallest_err_cnt)
        else:
            #print("Overlapping")
            smallest_err_cnt=0
            FS1_err_cnt=0
            FS2_err_cnt=0
            threshold=0
            for thr in range(file_list[0]-1 , mas_file_list[-1]+2): #Deciding the range to swipe over to find out the best threshold.
                err_cnt=0
                fs1_err_cnt=0
                fs2_err_cnt=0
                for itr in range(0,len(mas_file_list)):
                    if(mas_file_list[itr] > thr):
                        err_cnt+=1
                        fs1_err_cnt+=1
                for itr in range(0,len(file_list)):
                    if(file_list[itr] <= thr):
                        err_cnt+=1
                        fs2_err_cnt+=1
                if(smallest_err_cnt == 0):   #The first time assignment to variable smallest_err_cnt and threshold.
                    smallest_err_cnt = err_cnt
                    FS1_err_cnt=fs1_err_cnt
                    FS2_err_cnt=fs2_err_cnt
                    threshold = thr
                if(smallest_err_cnt > err_cnt): #Finding out the smallest_err_cnt and threshold with the least amount of errors.
                    smallest_err_cnt = err_cnt
                    FS1_err_cnt=fs1_err_cnt
                    FS2_err_cnt=fs2_err_cnt
                    threshold = thr
                print("8 vs ",file_size," least err count is ",smallest_err_cnt," threshold: ",threshold," err_cnt: ",err_cnt," thr: ",thr)
        print("8 vs ",file_size," least err count is ",smallest_err_cnt," threshold: ",threshold," ",FS1_err_cnt," ",FS2_err_cnt)
        probetype="16_uf_controlled"
        dict_mas_file_size=8
        threshold_dict[(probetype,dict_mas_file_size,file_size)]=threshold
#        print(mas_file_list)
#        print(file_list)

mas_file_list=[]
master_file_size=[8]
master_file_name="lrgen_like_probe_8_train.txt"
master_file_path = os.path.join(res_dir, master_file_name)
with open(master_file_path, "r") as mas_file:
    for (line_number, line) in enumerate(mas_file, start=1):
        if(line_number == 1):
            continue
        mas_file_list.append(int(line.strip()))
    mas_file_list.sort()
    #print("Error lr_gen: ")
    print("Train suite, lr_gen:")

    for file_size in file_sizes:
        #print(file_size)
        file_list=[]
        file_name="lrgen_like_probe_"+str(file_size)+"_train.txt"
        file_path = os.path.join(res_dir, file_name)
        with open(file_path, "r") as file1:
            for (line_number, line) in enumerate(file1, start=1):
                if(line_number == 1):
                    continue
                file_list.append(int(line.strip()))
        file_list.sort()

        if(mas_file_list[-1] < file_list[0]):
            #print("non-overlapping")
            threshold=int((file_list[0] - mas_file_list[-1])/2) + mas_file_list[-1]
            smallest_err_cnt=0
            FS1_err_cnt=0
            FS2_err_cnt=0
            #print("threshold is: ", threshold," error: ",smallest_err_cnt)
        else:
            #print("Overlapping")
            smallest_err_cnt=0
            FS1_err_cnt=0
            FS2_err_cnt=0
            threshold=0
            for thr in range(file_list[0]-1 , mas_file_list[-1]+2): #Deciding the range to swipe over to find out the best threshold.
                err_cnt=0
                fs1_err_cnt=0
                fs2_err_cnt=0
                for itr in range(0,len(mas_file_list)):
                    if(mas_file_list[itr] > thr):
                        err_cnt+=1
                        fs1_err_cnt+=1
                for itr in range(0,len(file_list)):
                    if(file_list[itr] <= thr):
                        err_cnt+=1
                        fs2_err_cnt+=1
                if(smallest_err_cnt == 0):   #The first time assignment to variable smallest_err_cnt and threshold.
                    smallest_err_cnt = err_cnt
                    FS1_err_cnt=fs1_err_cnt
                    FS2_err_cnt=fs2_err_cnt
                    threshold = thr
                if(smallest_err_cnt > err_cnt): #Finding out the smallest_err_cnt and threshold with the least amount of errors.
                    smallest_err_cnt = err_cnt
                    FS1_err_cnt=fs1_err_cnt
                    FS2_err_cnt=fs2_err_cnt
                    threshold = thr
        print("8 vs ",file_size," least err count is ",smallest_err_cnt," threshold: ",threshold," ",FS1_err_cnt," ",FS2_err_cnt)
        probetype="lrgen"
        dict_mas_file_size=8
        threshold_dict[(probetype,dict_mas_file_size,file_size)]=threshold

mas_file_list=[]
master_file_size=[8]
master_file_name="spp_like_probe_8_train.txt"
master_file_path = os.path.join(res_dir, master_file_name)
with open(master_file_path, "r") as mas_file:
    for (line_number, line) in enumerate(mas_file, start=1):
        if(line_number == 1):
            continue
        mas_file_list.append(int(line.strip()))
    mas_file_list.sort()
    print("Train suite, spp: ")

    for file_size in file_sizes:
        #print(file_size)
        file_list=[]
        file_name="spp_like_probe_"+str(file_size)+"_train.txt"
        file_path = os.path.join(res_dir, file_name)
        with open(file_path, "r") as file1:
            for (line_number, line) in enumerate(file1, start=1):
                if(line_number == 1):
                    continue
                file_list.append(int(line.strip()))
        file_list.sort()

        if(mas_file_list[-1] < file_list[0]):
            #print("non-overlapping")
            threshold=int((file_list[0] - mas_file_list[-1])/2) + mas_file_list[-1]
            smallest_err_cnt=0
            FS1_err_cnt=0
            FS2_err_cnt=0
            #print("threshold is: ", threshold," error: ",smallest_err_cnt)
        else:
            #print("Overlapping")
            smallest_err_cnt=0
            threshold=0
            FS1_err_cnt=0
            FS2_err_cnt=0
            for thr in range(file_list[0]-1 , mas_file_list[-1]+2): #Deciding the range to swipe over to find out the best threshold.
                err_cnt=0
                fs1_err_cnt=0
                fs2_err_cnt=0
                for itr in range(0,len(mas_file_list)):
                    if(mas_file_list[itr] > thr):
                        err_cnt+=1
                        fs1_err_cnt+=1
                for itr in range(0,len(file_list)):
                    if(file_list[itr] <= thr):
                        err_cnt+=1
                        fs2_err_cnt+=1
                if(smallest_err_cnt == 0):   #The first time assignment to variable smallest_err_cnt and threshold.
                    smallest_err_cnt = err_cnt
                    FS1_err_cnt=fs1_err_cnt
                    FS2_err_cnt=fs2_err_cnt
                    threshold = thr
                if(smallest_err_cnt > err_cnt): #Finding out the smallest_err_cnt and threshold with the least amount of errors.
                    smallest_err_cnt = err_cnt
                    FS1_err_cnt=fs1_err_cnt
                    FS2_err_cnt=fs2_err_cnt
                    threshold = thr
        print("8 vs ",file_size," least err count is ",smallest_err_cnt," threshold: ",threshold," ",FS1_err_cnt," ",FS2_err_cnt)
        probetype="spp"
        dict_mas_file_size=8
        threshold_dict[(probetype,dict_mas_file_size,file_size)]=threshold
#        print(mas_file_list)
#        print(file_list)
#print(threshold_dict)

print("============= Test suite error results =============")

mas_file_list=[]
master_file_name="lr_like_controlled_probe_8_test.txt"
master_file_path = os.path.join(res_dir, master_file_name)
with open(master_file_path, "r") as mas_file:
    for (line_number, line) in enumerate(mas_file, start=1):
        if(line_number == 1):
            continue
        mas_file_list.append(int(line.strip()))
    mas_file_list.sort()
    #print(mas_file_list)

    for file_size in file_sizes:
        #print(file_size)
        file_list=[]
        file_name="lr_like_controlled_probe_"+str(file_size)+"_test.txt"
        file_path = os.path.join(res_dir, file_name)
        with open(file_path, "r") as file1:
            for (line_number, line) in enumerate(file1, start=1):
                if(line_number == 1):
                    continue
                file_list.append(int(line.strip()))
        file_list.sort()
        #print(file_list)
        thr=threshold_dict.get(("16_uf_controlled",8,file_size))
        #print("threshold: ",thr)
        err_cnt=0
        err_cnt_fs1=0
        err_cnt_fs2=0
        for itr in range(0,len(mas_file_list)):
            if(mas_file_list[itr] > thr):
                err_cnt+=1
                err_cnt_fs1+=1
        for itr in range(0,len(file_list)):
            if(file_list[itr] <= thr):
                err_cnt+=1
                err_cnt_fs2+=1
        print("Errors for lr_uf_16_controlled: ",err_cnt," err_cnt_fs1: ",err_cnt_fs1," err_cnt_fs2: ",err_cnt_fs2)

mas_file_list=[]
master_file_name="lrgen_like_probe_8_test.txt"
master_file_path = os.path.join(res_dir, master_file_name)
with open(master_file_path, "r") as mas_file:
    for (line_number, line) in enumerate(mas_file, start=1):
        if(line_number == 1):
            continue
        mas_file_list.append(int(line.strip()))
    mas_file_list.sort()
    #print(mas_file_list)

    for file_size in file_sizes:
        #print(file_size)
        file_list=[]
        file_name="lrgen_like_probe_"+str(file_size)+"_test.txt"
        file_path = os.path.join(res_dir, file_name)
        with open(file_path, "r") as file1:
            for (line_number, line) in enumerate(file1, start=1):
                if(line_number == 1):
                    continue
                file_list.append(int(line.strip()))
        file_list.sort()
        #print(file_list)
        thr=threshold_dict.get(("lrgen",8,file_size))
        #print("threshold: ",thr)
        err_cnt=0
        err_cnt_fs1=0
        err_cnt_fs2=0
        for itr in range(0,len(mas_file_list)):
            if(mas_file_list[itr] > thr):
                err_cnt+=1
                err_cnt_fs1+=1
        for itr in range(0,len(file_list)):
            if(file_list[itr] <= thr):
                err_cnt+=1
                err_cnt_fs2+=1
        print("Errors for lrgen: ",err_cnt," err_cnt_fs1: ",err_cnt_fs1," err_cnt_fs2: ",err_cnt_fs2)

mas_file_list=[]
master_file_name="spp_like_probe_8_test.txt"
master_file_path = os.path.join(res_dir, master_file_name)
with open(master_file_path, "r") as mas_file:
    for (line_number, line) in enumerate(mas_file, start=1):
        if(line_number == 1):
            continue
        mas_file_list.append(int(line.strip()))
    mas_file_list.sort()
    #print(mas_file_list)

    for file_size in file_sizes:
        #print(file_size)
        file_list=[]
        file_name="spp_like_probe_"+str(file_size)+"_test.txt"
        file_path = os.path.join(res_dir, file_name)
        with open(file_path, "r") as file1:
            for (line_number, line) in enumerate(file1, start=1):
                if(line_number == 1):
                    continue
                file_list.append(int(line.strip()))
        file_list.sort()
        #print(file_list)
        thr=threshold_dict.get(("spp",8,file_size))
        #print("threshold: ",thr)
        err_cnt=0
        err_cnt_fs1=0
        err_cnt_fs2=0
        for itr in range(0,len(mas_file_list)):
            if(mas_file_list[itr] > thr):
                err_cnt+=1
                err_cnt_fs1+=1
        for itr in range(0,len(file_list)):
            if(file_list[itr] <= thr):
                err_cnt+=1
                err_cnt_fs2+=1
        print("Errors for spp: ",err_cnt," err_cnt_fs1: ",err_cnt_fs1," err_cnt_fs2: ",err_cnt_fs2)


