import csv

def read_file_line_by_line(ob, UF, atp, msl, benchmark, HIT_MISS_THRESHOLD):
    if benchmark == "train":
        hit_file_path="hit_data_UF_"+str(ob)+"_"+str(UF)+"_"+str(msl)+"_"+str(atp)+"_train.csv"
        miss_file_path="miss_data_UF_"+str(ob)+"_"+str(UF)+"_"+str(msl)+"_"+str(atp)+"_train.csv"
    elif benchmark == "test":
        hit_file_path="hit_data_UF_"+str(ob)+"_"+str(UF)+"_"+str(msl)+"_"+str(atp)+"_test.csv"
        miss_file_path="miss_data_UF_"+str(ob)+"_"+str(UF)+"_"+str(msl)+"_"+str(atp)+"_test.csv"
    hit_latencies_freq=0
    miss_latencies_freq=0
    overlapping_miss_latencies_freq=0
    # Open the file and read the file line by line.
    print(hit_file_path)
    with open(hit_file_path) as csvfile:
        # Create a CSV reader objt
        csvreader = csv.reader(csvfile)

        # Skip the first line (header)
        next(csvreader)

        # Iterate over the remaining lines
        for row in csvreader:
            # Add the hit latencies.
            hit_latencies_freq+=int(row[1])
 
    # Open the file and read the file line by line.
    print(miss_file_path)
    with open(miss_file_path) as csvfile:
        # Create a CSV reader objt
        csvreader = csv.reader(csvfile)

        # Skip the first line (header)
        next(csvreader)

        # Iterate over the remaining lines
        for row in csvreader:
            # Add the miss latencies.
            miss_latencies_freq+=int(row[1])
            # Add the overlapping latencies.
            if int(row[0]) < HIT_MISS_THRESHOLD or int(row[0]) == HIT_MISS_THRESHOLD:
                overlapping_miss_latencies_freq+=int(row[1])
    print("hit_latencies_freq: ",hit_latencies_freq," miss_latencies_freq: ",miss_latencies_freq," overlapping_miss_latencies_freq: ",overlapping_miss_latencies_freq)

    # Calculate the % of overlapping miss latencies.
    print((overlapping_miss_latencies_freq/(hit_latencies_freq+miss_latencies_freq))*100)




#### Pass the parameters of the file to be read.
#UFs=[256]
UFs=[32, 64, 128, 256]
#algo_trigger_points=[8,16,32,64,128]
algo_trigger_points=[128]
#benchmarks=["test","train"] 
benchmarks=["train"]
msg_string_len=[512]
#other_blocks=["31","326"]  
other_blocks=["326"]  
for ob in other_blocks:
    for UF in UFs:
        if UF == 32:
            HIT_MISS_THRESHOLD=134
        elif UF == 64:
            HIT_MISS_THRESHOLD=233
        elif UF == 128:
            HIT_MISS_THRESHOLD=316
        elif UF == 256:
            HIT_MISS_THRESHOLD=629
        for atp in algo_trigger_points:
            for msl in msg_string_len:
                for benchmark in benchmarks:
                    read_file_line_by_line(ob, UF, atp, msl, benchmark, HIT_MISS_THRESHOLD)

