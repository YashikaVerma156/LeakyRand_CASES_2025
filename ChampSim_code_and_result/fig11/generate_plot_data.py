import os

# Specify the file name or path
file_path = "best_dyn.txt"

# Check if the file exists before deleting
if os.path.exists(file_path):
    os.remove(file_path)
    print(f"{file_path} has been deleted.")
else:
    print(f"{file_path} does not exist.")

with open(file_path, "w") as file:
    file.write("#array_size   estimated   experimented\n")

file1="estimated_occ.txt"
file2="simulated_occ.txt"
est_occ_per=[]
sim_occ_per=[]
all_values=[]

with open (file1,"r") as fl:
    for line in fl:
        line=line.strip()
        line=100*(int(line)/32768)
        line=round(line,2)
        est_occ_per.append(line)

with open (file2,"r") as fl:
    for line in fl:
        line=line.strip()
        line=100*(int(line)/32768)
        line=round(line,2)
        sim_occ_per.append(line)

for i in range(0,len(sim_occ_per)):
    all_values=[]
    if i == 0:
        all_values.append("1c")
    elif i > 0 and i < 3:
        all_values.append("1.1c")
    elif i > 2:
        all_values.append("1.2c")
    all_values.append(est_occ_per[i])
    all_values.append(sim_occ_per[i])
    with open(file_path, "a") as file:
        file.write(" ".join(str(val) for val in all_values) + "\n")
