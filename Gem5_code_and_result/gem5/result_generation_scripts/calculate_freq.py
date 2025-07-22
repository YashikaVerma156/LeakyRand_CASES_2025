import pandas as pd
from collections import Counter
import sys

if(len(sys.argv) != 3):
    print("Enter ds and buffer")
    exit(0)

ds=int(sys.argv[1])
buffer=int(sys.argv[2])

file="output_"+str(ds)+"_"+str(buffer)+".csv"

print(file)

arr=[]

with open(file, "r") as fl:
    line_number=0
    for line in fl:
        line_number+=1
        if line_number == 1:
            continue
        line=line.strip()
        arr.append(int(line.split(",")[1]))
arr.sort()
unique_arr = list(set(arr))
unique_arr.sort()
print(unique_arr)
#freq = Counter(arr)
#print(freq)
unique_arr_freq=[]

for i in range(0,len(unique_arr)):
    count=0
    for j in range(0, len(arr)):
        if int(unique_arr[i]) == int(arr[j]):
            count+=1
    unique_arr_freq.append(int(count))

unique_arr_cumulative_prob=[]

for i in range(0,len(unique_arr)):
    print(unique_arr[i],unique_arr_freq[i])
for i in range(0,len(unique_arr)):
    print(unique_arr[i],unique_arr_freq[i]/len(arr))
    if i>0:
        unique_arr_cumulative_prob.append( (unique_arr_freq[i]/len(arr) + unique_arr_cumulative_prob[i-1]) )
    else:
        unique_arr_cumulative_prob.append( unique_arr_freq[i]/len(arr) )

for i in range(0,len(unique_arr)):
    print(unique_arr[i],unique_arr_cumulative_prob[i])

# Create the DataFrame
df = pd.DataFrame({
    'Miss_count': unique_arr,
    'miss_count_freq': unique_arr_cumulative_prob
})  

df.to_csv("miss_cumu_prob_"+str(ds)+"_"+str(buffer)+".csv", index=False)

