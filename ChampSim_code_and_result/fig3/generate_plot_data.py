import os

# Specify the file name or path
file_path = "region_99.txt"

# Check if the file exists before deleting
if os.path.exists(file_path):
    os.remove(file_path)
    print(f"{file_path} has been deleted.")
else:
    print(f"{file_path} does not exist.")

with open(file_path, "w") as file:
    file.write("#array_size  avg_occupancy  min_occupancy  max_occupancy\n")

file_name="latest_clear_cache_stats_1_new_5_itr.txt"
occ_per=[]
min_occ_per=[]
max_occ_per=[]
fc=[]
pc=[]
occ_per_1=0
occ_per_2=0
occ_per_3=0
occ_per_4=0
occ_per_5=0

min_occ_per_1=40000000000
min_occ_per_2=40000000000
min_occ_per_3=40000000000
min_occ_per_4=40000000000
min_occ_per_5=40000000000

max_occ_per_1=0
max_occ_per_2=0
max_occ_per_3=0
max_occ_per_4=0
max_occ_per_5=0

with open (file_name,"r") as fl:
    for line in fl:
        line=line.strip()
        line=line.split(",")
        if int(line[0]) == 1:
            occ_per_1 += int(line[3])
            if min_occ_per_1 > int(line[3]):
                min_occ_per_1 = int(line[3])
            if max_occ_per_1 < int(line[3]):
                max_occ_per_1 = int(line[3])

        elif int(line[0]) == 2:
            occ_per_2 += int(line[3])
            if min_occ_per_2 > int(line[3]):
                min_occ_per_2 = int(line[3])
            if max_occ_per_2 < int(line[3]):
                max_occ_per_2 = int(line[3])

        elif int(line[0]) == 3:
            occ_per_3 += int(line[3])
            if min_occ_per_3 > int(line[3]):
                min_occ_per_3 = int(line[3])
            if max_occ_per_3 < int(line[3]):
                max_occ_per_3 = int(line[3])

        elif int(line[0]) == 4:
            occ_per_4 += int(line[3])
            if min_occ_per_4 > int(line[3]):
                min_occ_per_4 = int(line[3])
            if max_occ_per_4 < int(line[3]):
                max_occ_per_4 = int(line[3])

        elif int(line[0]) == 5:
            occ_per_5 += int(line[3])
            if min_occ_per_5 > int(line[3]):
                min_occ_per_5 = int(line[3])
            if max_occ_per_5 < int(line[3]):
                max_occ_per_5 = int(line[3])


occ_per_1 /= 5
occ_per.append(occ_per_1)
min_occ_per.append(min_occ_per_1)
max_occ_per.append(max_occ_per_1)

occ_per_2 /= 5
occ_per.append(occ_per_2)
min_occ_per.append(min_occ_per_2)
max_occ_per.append(max_occ_per_2)

occ_per_3 /= 5
occ_per.append(occ_per_3)
min_occ_per.append(min_occ_per_3)
max_occ_per.append(max_occ_per_3)

occ_per_4 /= 5
occ_per.append(occ_per_4)
min_occ_per.append(min_occ_per_4)
max_occ_per.append(max_occ_per_4)

occ_per_5 /= 5
occ_per.append(occ_per_5)
min_occ_per.append(min_occ_per_5)
max_occ_per.append(max_occ_per_5)

for i in range(0,len(occ_per)):
    all_values=[]
    occ_per[i] /= 32768
    occ_per[i] *= 100
    min_occ_per[i] /= 32768
    min_occ_per[i] *= 100
    max_occ_per[i] /= 32768
    max_occ_per[i] *= 100
    all_values.append(i+1)
    all_values.append(int(occ_per[i]))
    all_values.append(int(min_occ_per[i]))
    all_values.append(int(max_occ_per[i]))

    with open(file_path, "a") as file:
        file.write(" ".join(str(val) for val in all_values) + "\n")



# Specify the file name or path
file_path = "temp.txt"

# Check if the file exists before deleting
if os.path.exists(file_path):
    os.remove(file_path)
    print(f"{file_path} has been deleted.")
else:
    print(f"{file_path} does not exist.")

file_name="latest_clear_cache_stats_1_new.txt"

with open (file_name,"r") as fl:
    for line in fl:
        all_values=[]
        line=line.strip()
        line=line.split(",")
        if int(line[0]) == 1:
            all_values.append(1)
            all_values.append(line[6])
            all_values.append(line[7])
        elif int(line[0]) == 2:
            all_values.append(2)
            all_values.append(line[6])
            all_values.append(line[7])
        elif int(line[0]) == 3:
            all_values.append(3)
            all_values.append(line[6])
            all_values.append(line[7])
        elif int(line[0]) == 4:
            all_values.append(4)
            all_values.append(line[6])
            all_values.append(line[7])
        elif int(line[0]) == 5:
            all_values.append(5)
            all_values.append(line[6])
            all_values.append(line[7])
        with open(file_path, "a") as file:
            file.write(" ".join(str(val) for val in all_values) + "\n")
