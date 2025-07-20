import os

# Specify the file name or path
file_path = "llc_miss_io_calls.txt"

# Check if the file exists before deleting
if os.path.exists(file_path):
    os.remove(file_path)
    print(f"{file_path} has been deleted.")
else:
    print(f"{file_path} does not exist.")

with open(file_path, "w") as file:
    file.write("#Arr_size Arr_size_in_MB compute_iter_2 compute_iter_4 compute_iter_6 compute_iter_8\n")

file_name="extracted_result_llc_misses.txt"
arr_2_0=[]
arr_2_1=[]
arr_4_0=[]
arr_4_1=[]
arr_6_0=[]
arr_6_1=[]
arr_8_0=[]
arr_8_1=[]

with open (file_name,"r") as fl:
    line_number=0
    for line in fl:
        line_number+=1
        line=line.strip()
        line=line.split(',')
        if line_number == 1:
            continue
        if int(line[3]) == 2 and int(line[0]) == 0 and int(line[1]) == 1:
            total=0
            for i in range(5,25):
                total+=int(line[i])
            arr_2_0.append(total)

        if int(line[3]) == 2 and int(line[0]) == 1 and int(line[1]) == 1:
            total=0
            for i in range(5,25):
                total+=int(line[i])
            arr_2_1.append(total)

        if int(line[3]) == 4 and int(line[0]) == 0 and int(line[1]) == 1:
            total=0
            for i in range(5,25):
                total+=int(line[i])
            arr_4_0.append(total)

        if int(line[3]) == 4 and int(line[0]) == 1 and int(line[1]) == 1:
            total=0
            for i in range(5,25):
                total+=int(line[i])
            arr_4_1.append(total)

        if int(line[3]) == 6 and int(line[0]) == 0 and int(line[1]) == 1:
            total=0
            for i in range(5,25):
                total+=int(line[i])
            arr_6_0.append(total)

        if int(line[3]) == 6 and int(line[0]) == 1 and int(line[1]) == 1:
            total=0
            for i in range(5,25):
                total+=int(line[i])
            arr_6_1.append(total)

        if int(line[3]) == 8 and int(line[0]) == 0 and int(line[1]) == 1:
            total=0
            for i in range(5,25):
                total+=int(line[i])
            arr_8_0.append(total)

        if int(line[3]) == 2 and int(line[0]) == 1 and int(line[1]) == 1:
            total=0
            for i in range(5,25):
                total+=int(line[i])
            arr_8_1.append(total)

if len(arr_2_0) != 5 or len(arr_2_1) != 5 or len(arr_4_0) != 5 or len(arr_4_1) != 5 or len(arr_6_0) != 5 or len(arr_6_1) != 5 or len(arr_8_0) != 5 or len(arr_8_1) != 5:
    print("Something is wrong, exiting...")

arr_2=[]
arr_4=[]
arr_6=[]
arr_8=[]
for i in range(0,len(arr_8_1)):
    num=arr_2_0[i]/arr_2_1[i]
    num=round(num,2)
    arr_2.append(num)

    num=arr_4_0[i]/arr_4_1[i]
    num=round(num,2)
    arr_4.append(num)

    num=arr_6_0[i]/arr_6_1[i]
    num=round(num,2)
    arr_6.append(num)

    num=arr_8_0[i]/arr_8_1[i]
    num=round(num,2)
    arr_8.append(num)

if len(arr_2) != 5 or len(arr_4) != 5 or len(arr_6) != 5 or len(arr_8) != 5:
    print("Something is wrong, exiting...")

for i in range(0,len(arr_8)):
    all_values=[]
    if i == 0:
        num=0.125
        num_str="0.25MB"
    elif i == 1:
        num=0.25
        num_str="0.5MB"
    elif i == 2:
        num=0.5
        num_str="1MB"
    elif i == 3:
        num=0.75
        num_str="1.5MB"
    elif i == 4:
        num=1
        num_str="2MB"
    all_values.append(num)
    all_values.append(num_str)
    all_values.append(arr_2[i])
    all_values.append(arr_4[i])
    all_values.append(arr_6[i])
    all_values.append(arr_8[i])
    with open(file_path, "a") as file:
        file.write(" ".join(str(val) for val in all_values) + "\n")



