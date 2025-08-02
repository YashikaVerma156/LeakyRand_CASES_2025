#ths=[-5000, -4000, -3000, -2000, -1500, -1000]
#ths=[-500, -200, -100, 0, 100]
ths=[-8000, -7000, -6000]
size=[3276, 6554, 9830, 13108, 16384]
for sz in size:
    print(sz)
    string=[]
    for th in ths:
        fl="error_count_sender_arr_size_"+str(sz)+"_msg_size_512_th_"+str(th)+".txt"
        fl_path='extracted_data/'+fl
        with open(fl_path, "r") as f:
            for line_number, line in enumerate(f, start=1):
                if "Total" in line:
                    line=line.strip()
                    line = line.split(' ')
                    strng=str(line[2])+' & '+str(line[5])+' & '+str(line[8])+' & '
                    string.append(strng)
                    break   # check nly the first occurrence
    print(*string)  # print list without brackets and comma.
