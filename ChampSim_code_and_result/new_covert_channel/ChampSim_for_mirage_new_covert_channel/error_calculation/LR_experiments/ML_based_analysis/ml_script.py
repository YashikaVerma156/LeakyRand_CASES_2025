import os
import sys
import pandas as pd
import numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report, accuracy_score

if len(sys.argv) != 3:
    print("Usage: python3 ml_script.py <ds> <uf>")
    print("Something is wrong.")
    exit(0)

ds=int(sys.argv[1])
uf=int(sys.argv[2])

errors=0
error_0to1=0
error_1to0=0
total_checked_bits=0

# Paths
parent_folder_train='misses_results_'+str(ds)+'_train'
parent_folder_test='misses_results_'+str(ds)+'_test'
inner_folder='misses_results_'+str(uf)
train_folder = os.path.join(parent_folder_train,inner_folder)
test_folder = os.path.join(parent_folder_test,inner_folder)

# -------------- Step 1: Load Training Data --------------

X_train = []
y_train = []

# Assuming features are stored in .txt in train_folder
for filename in os.listdir(train_folder):
    #filename_string=filename
    str_num=int(filename.split("_")[2])
    if filename.endswith('.txt'):
        file_path = os.path.join(train_folder, filename)
        #print(filename) 
        # Load features
        df = pd.read_csv(file_path)
        
        #print(f"df shape: {df.shape}")
        #exit(0)
        with open("benchmark_train.txt", "r") as f:
            line_number=0
            for line in f:
                line_number += 1
                if str_num == line_number:
                    label_string = line.strip()  # e.g., '11001010110'
                    labels = np.array([int(c) for c in label_string])  # convert to array of integers
                    #print(line_number) 

        # Check length
        if len(df) != len(labels):
            raise ValueError(f"Mismatch between feature rows and label length in {filename}")
        #XXX understand these instructions
        for i in range(0, len(df)):
            X_train.append(df.iloc[i])
            y_train.append(labels[i])
             
        #X_train.append(df.values)
        #y_train.append(labels)
        #exit(0)
print(len(X_train))
print(len(X_train[0]))
print(len(y_train))
#XXX understand these instructions
# Stack all training data
X_train = np.vstack(X_train)
y_train = np.hstack(y_train)
#print(X_train)
print(f"Training data shape: {X_train.shape}")
print(f"Training labels shape: {y_train.shape}")
#exit(0)
# -------------- Step 2: Train Classifier --------------

# You can use Logistic Regression, Random Forest, LightGBM, etc.
model = LogisticRegression(max_iter=1000)
model.fit(X_train, y_train)
#print(model.coef_)
#print(model.get_params())
#exit(0)
# -------------- Step 3: Load Test Data and Predict --------------

pred_dir='predicted_test_labels_'+str(ds)+'_'+str(uf)

# Make folder to save predictions
os.makedirs(pred_dir, exist_ok=True)
for filename in os.listdir(test_folder):
    str_num=int(filename.split("_")[2])
    if filename.endswith('.txt'):
        file_path = os.path.join(test_folder, filename)
        
        df_test = pd.read_csv(file_path)
        tot_preds=[]
        #print(str_num)
        #print(df_test.iloc[0])
        #print(df_test.iloc[0])
        #exit(0)
        # Predict
        #preds = model.predict(df_test.values)
        #preds = model.predict(df_test.iloc[0])
        for i in range(0, len(df_test)):
            X_test=[]
            X_test.append(df_test.iloc[i])
            #X_test=df_test.iloc[i]
            #print(len(df_test.iloc[i]))
            #print(df_test.iloc[i].shape)
            #print(len(X_test))
            #print(X_test.shape)
            #print(X_test)
            preds = model.predict(X_test)
            #print (str_num)
            #print(preds) 
            #print(preds[0]) 
            tot_preds.append(preds[0]) 
            #exit(0)
        # Save predictions
        output_file = os.path.join(pred_dir, filename.replace('.txt', '_preds.csv'))
        pd.DataFrame(tot_preds, columns=['Predicted_Label']).to_csv(output_file, index=False)
        #exit(0)

        #print(f"Predicted and saved for file: {filename}")


# -------------- Step 4: Errors calculation --------------
for filename in os.listdir(pred_dir):
    string=filename
    str_num=int(string.split('_')[2])
    with open("benchmark_test.txt", "r") as f:
        line_number=0
        for line in f:
            line_number += 1
            if str_num == line_number:
                label_string = line.strip()  # e.g., '11001010110'
                actual_labels = [int(c) for c in label_string]  # convert to array of integers
                #print(line_number)
                #print(actual_labels)
    file_path = os.path.join(pred_dir, filename)
    predicted_labels=[]
    with open(file_path,"r") as f:
        line_number=0
        for line in f:
            line_number += 1
            if(line_number > 1):
                line = line.strip()
                #print(line)
                predicted_labels.append(int(line))

    # Check length match
    if len(actual_labels) != len(predicted_labels):
        raise ValueError(f"Mismatch in size")
 
    #Error check
    for i in range(0,len(predicted_labels)):
        total_checked_bits += 1
        if actual_labels[i] != predicted_labels[i]:
            errors += 1
            if actual_labels[i] == 0:
                error_0to1 += 1
            if actual_labels[i] == 1:
                error_1to0 += 1

print("Errors:",errors,"error_1to0:",error_1to0,"error_0to1:",error_0to1,"total_checked_bits:",total_checked_bits)
