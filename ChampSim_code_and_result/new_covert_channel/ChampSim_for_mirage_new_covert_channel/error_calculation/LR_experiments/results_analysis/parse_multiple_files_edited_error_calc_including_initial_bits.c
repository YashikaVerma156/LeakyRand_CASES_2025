#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

#define MAX_PATH 1024

int extract_string_number(char input[])
{
    char *token;
    int target_index = 2; // Fixed location of the string_number in the filename.
    int current_index = 0;
    int extracted_number = 0;

    // Split the string by '_'
    token = strtok(input, "_"); // First call to the strtok should be with the complete string to be tokenized and the string to be used to split.
    while (token != NULL) {
        if (current_index == target_index) {
            extracted_number = atoi(token); // Convert substring to an integer
            break;
        }
        token = strtok(NULL, "_"); // Further calls to strtok should be with NULL and the string to be used to split.
        current_index++;
    }

    /*if (extracted_number != 0) {
        printf("Extracted number: %d\n", extracted_number);
    } else {
        printf("Number not found or invalid format.\n");
    }*/

    return extracted_number;
}

// Function to check if the file has a .txt extension
int is_text_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (ext == NULL)
    {
        printf("pointer ext is NULL.");
        _exit(1);
    }
    return ext != NULL && strcmp(ext, ".txt") == 0;
}

int main(int argc, char *argv[])
{
  FILE *fptr;
	if (argc != 10) {
		printf("Usage: ./parse <benchmark file train> <dir_for_latency_file_train_suite> <history_length> <dir_for_latency_file_test_suite> <benchmark file test> <sender_type: 0(naive), 1(version1), or 2(version2)>  <unrolling_factor> <threshold(n)> <less_miss: 0(more_miss), 1(less_miss)> .\nAborting...\n");
		exit(0);
	}
  // Open the directory
  DIR *dir = opendir(argv[2]);
  if (!dir) {
      perror("Error opening directory");
      return 1;
  }

  char filepath[MAX_PATH];
	int i, j, k;
	int histlen = atoi(argv[3]), sender_type = atoi(argv[6]), sender_ds = 0, uf = atoi(argv[7]), threshold_n = atoi(argv[8]), less_miss = atoi(argv[9]);
	assert(histlen >= 0);

    char sender[20];
    if(sender_type == 0)
    {
        sprintf(sender,"naive");
    }
    else if(sender_type == 1)
    {
        sprintf(sender,"version1");
    }
    else if(sender_type == 2)
    {
        sprintf(sender,"version2");
    }
    else
    {
        printf("Invalid sender_type is provided.");
        _exit(1);
    }


  // Use sscanf to extract the sender |DS|.
  if (sscanf(argv[2], "misses_results_%d_train", &sender_ds) == 1) {
      printf("Processing for sender |DS|: %d\n", sender_ds);
  } else {
        printf("Failed to extract the number.\n");
        _exit(1); 
  }
  char out_filename[200];

  // Construct the output filename.
  sprintf(out_filename, "DS_%d_histlen_%d_%d_%d_both_suite_%s_%d.txt", sender_ds, histlen, uf, threshold_n, sender, less_miss);
  printf("%s\n",out_filename);

  // Open the file in write mode
  fptr = fopen(out_filename, "w");
  if (fptr == NULL) {
      printf("Error opening file!\n");
      return 1;
  } 
  //_exit(1);
  // Define the initial reference bits to be used as history.
  //int INITIAL_REFERENCE_BITS = (histlen == 0) ? 1 : histlen;
  int INITIAL_REFERENCE_BITS = 0;

  // Allocate a buffer of size histlen+1.
	int *global_minAB = (int*)malloc((1 << (histlen+1))*sizeof(int));
	int *global_maxAB = (int*)malloc((1 << (histlen+1))*sizeof(int));
	assert(global_minAB);
	assert(global_maxAB);
  for (i=0; i<(1 << (histlen+1)); i++) 
  {
      global_minAB[i] = 1000000000;
      global_maxAB[i] = -1000000000;
  }
	int count = 0;
  struct dirent *entry;
  int file_count = 0;
  int files_processed = 0;

  // Iterate over files in the directory
  while ((entry = readdir(dir)) != NULL) 
  {
      // Skip directories
      if (entry->d_type == DT_DIR) 
      {
          //printf("%s",entry->d_name);
          //printf("continued\n");
          continue;
      }
  // Check if the file is a text file, write it to filepath.
  if (is_text_file(entry->d_name)) {
      snprintf(filepath, sizeof(filepath), "%s/%s", argv[2], entry->d_name);
  }

  char file_string[50];
  // intermediate copying is needed as the strtok() used in extract_string_number(), modifies the original string.
  strcpy(file_string, entry->d_name);
  file_count = extract_string_number(file_string);
  printf("%s\n",entry->d_name);
  //_exit(1);

	FILE *fp = fopen(argv[1], "r");
	assert(fp);
	char c;
	char bits[512 + INITIAL_REFERENCE_BITS];
	if (fseek(fp, (file_count-1)*513, SEEK_SET) != 0) {
			printf("Error seeking in file");
			fclose(fp);
			_exit(1);
	}

  for(i=0; i < INITIAL_REFERENCE_BITS; i++)
  {  
    bits[i] = 0; 
  }

	count = 0 + INITIAL_REFERENCE_BITS;
	while (1) {
		fscanf(fp, "%c", &c);
		if (c == '\n') break;
		else if (c == '0') bits[count] = 0;
		else if (c == '1') bits[count] = 1;
		else assert(0);
		count++;
	}
	assert(count == 512 + INITIAL_REFERENCE_BITS);
	fclose(fp);

  // Print the message string.
  /*for(i=0; i<count; i++)
  printf("%d ",bits[i]);
    printf("\n");
  _exit(1);*/ 

	//fp = fopen(argv[2], "r");
	fp = fopen(filepath, "r");
  assert(fp);

  int miss_count[uf+1];
	
  // Skip the header (a line of string)
  char buffer[4096];
  if (fgets(buffer, sizeof(buffer), fp) == NULL) {
      perror("Error reading header");
      fclose(fp);
      return 1;
  }

  // Read the receiver's latency in the first line of the text file, i.e. before starting communication.
		//fscanf(fp, "%d,%d,%d,%d,%d\n", &a1, &a2, &a3, &a4, &a5);
		//fscanf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10, &a11);
    //int init_bit_0 = a4;
    //printf("%d\n", init_bit_0);
  /*if (fgets(buffer, sizeof(buffer), fp) == NULL) {
      perror("Error reading first line of integers");
      fclose(fp);
      return 1;
  }*/
	int count2 = INITIAL_REFERENCE_BITS, count2_sender = 0;
	int latency[512 + INITIAL_REFERENCE_BITS];

  // Assign 0 to initial reference bits latency.
  for(i=0; i<INITIAL_REFERENCE_BITS; i++)
  {
    latency[i] = 0;
  }

  // Assign the value for receiver latency assigned for 51st probe i.e. latency of the one empty probe before starting communication.
  //latency[count2 - 1] = init_bit_0;

	//int sender_latency[512];
	//while (!feof(fp)) {
  char *token;
  while(fgets(buffer, sizeof(buffer), fp) != NULL)
  {
      //printf("Hi count2: %d\n",count2);
		  //fscanf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10, &a11);
		  //latency[count2] = a4;

      int iter = 0, miss_number[uf+1];
      token = strtok(buffer, ",");
      while (token != NULL)
      {
          miss_number[iter] = atoi(token);  // Convert token to integer
          //printf("%d\n", miss_number[iter]);  // Do whatever you need with the number
          token = strtok(NULL, ",");
          //printf("%s\n",token);
          iter++;
      }
      iter=0;
      latency[count2]=0;
      while(iter <= uf)
      {
          if(iter >= threshold_n)
          {    latency[count2] += miss_number[iter];
          }
          iter++;
      }

      // Sender latency is collected to compare it with the actual message later.
		  //sender_latency[count2_sender] = a5;
		count2++;
		//count2_sender++;
	}
	//assert(count2_sender == 512);
	assert(count2 == 512 + INITIAL_REFERENCE_BITS);
	fclose(fp);

  // Print the latency array.
  /*for(i=0; i<count2; i++)
    printf("%d ",latency[i]);
  printf("\n");
  _exit(1);*/

  // Verify the sender disturbance pattern with the actual message string.
  /*for(int i=0; i<512; i++)
  {
    if((sender_latency[i] < 1000) && (bits[i + INITIAL_REFERENCE_BITS] == 1))
    {    
        printf("There is a mismatch in the communicated message.\n");
        assert(0);
    }
  }*/

  // Allocate a buffer of size histlen+1.
	int *minAB = (int*)malloc((1 << (histlen+1))*sizeof(int));
	int *maxAB = (int*)malloc((1 << (histlen+1))*sizeof(int));
	assert(minAB);
	assert(maxAB);
  // Initialize the buffer.
	for (i=0; i<(1 << (histlen+1)); i++) {
		minAB[i] = 1000000000;
		maxAB[i] = -1000000000;
	}
  printf("%d\n",histlen);
	for (i=((histlen == 0) ? 1 : histlen); i<count; i++) {
		for (j=0; j<(1 << (histlen+1)); j++) {
			for (k=0; k<histlen+1; k++) {
				if (bits[k+i-histlen] != ((j >> (histlen - k)) & 0x1)) break;
			}
			if (k == (histlen+1)) break;
		}
		assert(j < (1 << (histlen+1)));
                if ((latency[i] - latency[i-1]) < minAB[j]) minAB[j] = latency[i] - latency[i-1];
                if ((latency[i] - latency[i-1]) > maxAB[j]) maxAB[j] = latency[i] - latency[i-1];
                printf("i: %d %d\n", i, bits[i]);
		for (j=0; j<(1 << histlen); j++) printf("min%d=%d, max%d=%d, ", j, minAB[j], j, maxAB[j]);
		printf("\n");
		for (j=(1 << histlen); j<(1 << (histlen+1)); j++) printf("min%d=%d, max%d=%d, ", j, minAB[j], j, maxAB[j]);
		printf("\n\n");
        }
	  for (j=0; j<(1 << histlen); j++) 
    {   
        if(global_minAB[j] > minAB[j])
            global_minAB[j] = minAB[j];
        if(global_maxAB[j] < maxAB[j])
            global_maxAB[j] = maxAB[j];
        //printf("==min%d=%d, max%d=%d, ", j, minAB[j], j, maxAB[j]);
        printf("== global_min%d=%d, gobal_max%d=%d, ", j, global_minAB[j], j, global_maxAB[j]);
        fprintf(fptr,"== global_min%d=%d, gobal_max%d=%d, ", j, global_minAB[j], j, global_maxAB[j]);
    }
    printf("\n");
    fprintf(fptr,"\n");
    for (j=(1 << histlen); j<(1 << (histlen+1)); j++) 
    {   
        if(global_minAB[j] > minAB[j])
            global_minAB[j] = minAB[j];
        if(global_maxAB[j] < maxAB[j])
            global_maxAB[j] = maxAB[j];
        //printf("==min%d=%d, max%d=%d, ", j, minAB[j], j, maxAB[j]);
        printf("== global_min%d=%d, gobal_max%d=%d, ", j, global_minAB[j], j, global_maxAB[j]);
        fprintf(fptr,"== global_min%d=%d, gobal_max%d=%d, ", j, global_minAB[j], j, global_maxAB[j]);
  
    }    
    printf("\n");
    fprintf(fptr,"\n");
    file_count ++;
    files_processed ++;
    printf("files_processed: %d\n",files_processed);
  }

  // Close the directory
  closedir(dir);
  //exit(0); 
 //=======The code below is for calculating best threshold and errors.\n");

	int *t = (int*)malloc((1 << histlen)*sizeof(int));
	int *best_t = (int*)malloc((1 << histlen)*sizeof(int));
	int error = 0, global_error = 0, global_error0to1 = 0, global_error1to0 = 0, error0to1 = 0, error1to0 = 0;
	int *min_error = (int*)malloc((1 << histlen)*sizeof(int));
  int *error_0to1 = (int*)malloc((1 << histlen)*sizeof(int));
  int *error_1to0 = (int*)malloc((1 << histlen)*sizeof(int));
	assert(t);
	assert(best_t);
	assert(min_error);
  assert(error_1to0);
  assert(error_0to1);

	for (i=0; i< (1 << histlen); i++) min_error[i] = 512*500;
  // The logic is to calculate error for one subspace across the complete directory.
  printf(" ======== Minimum errors and best threshold observed across train suite. ======== \n");
  fprintf(fptr," ======== Minimum errors and best threshold observed across train suite. ======== \n");
	for (j=0; j<(1 << histlen); j++) 
  {
    // Defines the range of thresholds to be tested for calculating the minimum number of errors.
    int limit, start;

    printf("Processing for history_length: %d\n",j);
    if(global_maxAB[2*j] < global_minAB[2*j+1])
    {
        // abs() is used because sometimes both the values are negative.
        start = global_maxAB[2*j] + abs((global_minAB[2*j+1] - global_maxAB[2*j]))/2;
        //printf("Start: %d\n",start);
        limit = start + 1;
        //printf("Limit: %d\n",limit);
    }
    else if(global_maxAB[2*j] >= global_minAB[2*j+1] && global_minAB[2*j+1] > global_minAB[2*j])
    {
        start = global_minAB[2*j+1]-1;
        limit = global_maxAB[2*j] + 1;
    }
    else
    {
        // Mostly any other case will/should not arise.
        printf("A different scenario is observed");
        assert(0);
    }
    // XXX New condition.
    for(t[j] = start; t[j] < limit; t[j]++)
    {
				// Open the directory
				dir = opendir(argv[2]);
				if (!dir) {
						perror("Error opening directory");
						return 1;
				} 
        global_error = 0, global_error0to1 = 0, global_error1to0 = 0; 
			  // Iterate over files in the directory to calculate errors.
				while ((entry = readdir(dir)) != NULL)
			  {
						// Skip directories "." and ".."
						if (entry->d_type == DT_DIR)
						{
								//printf("%s",entry->d_name);
								//printf("continued\n");
								continue;
						}
						// Check if the file is a text file
						if (is_text_file(entry->d_name)) {
								snprintf(filepath, sizeof(filepath), "%s/%s", argv[2], entry->d_name);
						}

						char file_string[100];
						strcpy(file_string, entry->d_name);
						file_count = extract_string_number(file_string);
						//printf("%s\n",entry->d_name);
						//_exit(1);

            count = 0 + INITIAL_REFERENCE_BITS;
						FILE *fp = fopen(argv[1], "r");
            //printf("line 267 count: %d\n",count);
						assert(fp);
						char c;
						char bits[512 + INITIAL_REFERENCE_BITS];
            //Initialize the reference bits as zero.
            for(i=0; i<INITIAL_REFERENCE_BITS; i++)
                bits[i] = 0;
            // Seek to the line in the benchmark_*.txt
						if (fseek(fp, (file_count-1)*513, SEEK_SET) != 0) {
								printf("Error seeking in file");
								fclose(fp);
								_exit(1);
						}
						while (1) {
							fscanf(fp, "%c", &c);
							if (c == '\n') break;
							else if (c == '0') bits[count] = 0;
							else if (c == '1') bits[count] = 1;
							else assert(0);
							count++;
						}
            // Print the message string.
            /*for(i=0; i<count; i++)
              printf("%d ",bits[i]);
            printf("\n");*/
            //_exit(1);
            //printf("count: %d\n",count);
						assert(count == 512 + INITIAL_REFERENCE_BITS);
						fclose(fp);
						fp = fopen(filepath, "r");
						assert(fp);
						// Skip the header (a line of string)
						char buffer[4096];
						if (fgets(buffer, sizeof(buffer), fp) == NULL) {
								perror("Error reading header");
								fclose(fp);
								return 1;
						}

						// int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11;
            // Read the receiver's latency in the first line of the text file, i.e. before starting communication.
            // fscanf(fp, "%d,%d,%d,%d,%d\n", &a1, &a2, &a3, &a4, &a5);
		        // fscanf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10, &a11);
            // int init_bit_0 = a4;

						int count2 = 0 + INITIAL_REFERENCE_BITS;
						int latency[512 + INITIAL_REFERENCE_BITS];

            //Initialize the receiver's latency for reference bits as zero.
            for(i=0; i<INITIAL_REFERENCE_BITS; i++)
            {
              latency[i] = 0;
            }

            // Assign the receiver latency observed for 51st probe.
            //latency[count2 - 1] = init_bit_0;

            //printf("Hi\n");
            char *token;
						//while (!feof(fp)) 
            while(fgets(buffer, sizeof(buffer), fp) != NULL)
            {
						    //fscanf(fp, "%d,%d,%d,%d,%d\n", &a1, &a2, &a3, &a4, &a5);
		            //fscanf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10, &a11);
							  //latency[count2] = a4;
                int iter = 0, miss_number[uf+1];
                token = strtok(buffer, ",");
                while (token != NULL)
                {
                    miss_number[iter] = atoi(token);  // Convert token to integer
                    //printf("%d\n", miss_number[iter]);  // Do whatever you need with the number
                    token = strtok(NULL, ",");
                    //printf("%s\n",token);
                    iter++;
                }
                iter=0;
                latency[count2]=0;
                while(iter <= uf)
                {
                    if(iter >= threshold_n)
                    {    latency[count2] += miss_number[iter];
                    }
                    iter++;
                }
							  count2++;
						}
            // Print the latency array.
            /*for(i=0; i<count2; i++)
            printf("%d ",latency[i]);
            printf("\n");
            _exit(1);*/
						assert(count2 == 512 + INITIAL_REFERENCE_BITS);
						fclose(fp);
            //printf("count2: %d\n",count2);

						error = 0, error0to1 = 0, error1to0 = 0;
						for (i=((histlen == 0) ? 1 : histlen); i<count; i++) // Looping over the complete 512 bits. 
						{
								for (k=0; k<histlen; k++) // This loop check for the matching pattern of the subspace.
								{
										if (bits[k+i-histlen] != ((j >> (histlen - k - 1)) & 0x1)) break;
								}
								if (k == histlen) 
								{
										//if((j == 3 && t[j] == -3339) || (j == 7 && t[j] == -3679) )
										//		printf("i is: %d, bit is:%d, latency difference: %d, threshold is: %d \n",i,bits[i], (latency[i] - latency[i-1]), t[j]); 
										if ((latency[i] - latency[i-1]) <= t[j]) 
										{
												if (bits[i] == 1)
                        { 
                            error++;
                            error1to0 ++; 
                            //if((j == 3 && t[j] == -3339) || (j == 7 && t[j] == -3679) )
                                //printf("bit position:%d, file_number:%d, error bit is:%d, latency difference: %d, threshold is: %d \n",i,file_count,bits[i], (latency[i] - latency[i-1]), t[j]); 
                        }
										}
										else if (bits[i] == 0)
                    {
                        error++;
                        error0to1 ++;
                        //if((j == 3 && t[j] == -3339) || (j == 7 && t[j] == -3679))
                            //printf("bit position:%d, file_number:%d, error bit is:%d, latency difference: %d, threshold is: %d \n",i,file_count,bits[i], (latency[i] - latency[i-1]), t[j]); 
                    }
								}
						}
						// Add the error to global_error.
						global_error += error;
            global_error0to1 += error0to1;
            global_error1to0 += error1to0;
        }
				closedir(dir);
				// End the loop for all the files in the directory here.
				if (global_error < min_error[j]) // This should be global_error 
				{
            min_error[j] = global_error;
            error_0to1[j] = global_error0to1;
            error_1to0[j] = global_error1to0;
            best_t[j] = t[j];
				}
    }
	}
	error = 0;
	for (j=0; j<(1 << histlen); j++) {
    printf("min_error%d: %d, best_t%d: %d, error_1to0[%d]: %d, error_0to1[%d]: %d\n",j,min_error[j],j,best_t[j],j,error_1to0[j],j,error_0to1[j]);
    fprintf(fptr,"min_error%d: %d, best_t%d: %d, error_1to0[%d]: %d, error_0to1[%d]: %d\n",j,min_error[j],j,best_t[j],j,error_1to0[j],j,error_0to1[j]);
    error += min_error[j];
    error1to0 += error_1to0[j];
    error0to1 += error_0to1[j];
	}
  printf("Total error: %d, Total 1->0 error: %d, Total 0->1 error: %d\n", error, error1to0, error0to1);
  fprintf(fptr,"Total error: %d, Total 1->0 error: %d, Total 0->1 error: %d\n", error, error1to0, error0to1);

  //exit(0);
 
  //XXX Calculate the errors in test suite, based on the best threshold chosen across the train_benchmark.
  for (i=0; i< (1 << histlen); i++) min_error[i] = 512*500;

  printf(" ======== Errors observed across Test suite, for best threshold observed across Train suite. =========\n");
  fprintf(fptr," ======== Errors observed across Test suite, for best threshold observed across Train suite. =========\n");
  for (j=0; j<(1 << histlen); j++)
  {
    int limit, start;
    printf("Processing for history_length: %d\n",j);

    t[j] = best_t[j];
    // XXX New condition.
    //for(t[j] = start; t[j] < limit; t[j]++)
    //{
        // Open the directory
        dir = opendir(argv[4]);
        if (!dir) {
            perror("Error opening directory");
            return 1;
        }
        global_error = 0, global_error0to1 = 0, global_error1to0 = 0;
        // Iterate over files in the directory to calculate errors.
        while ((entry = readdir(dir)) != NULL)
        {
            // Skip directories "." and ".."
            if (entry->d_type == DT_DIR)
            {
                //printf("%s",entry->d_name);
                //printf("continued\n");
                continue;
            }
            // Check if the file is a text file
            if (is_text_file(entry->d_name)) {
                snprintf(filepath, sizeof(filepath), "%s/%s", argv[4], entry->d_name);
            }

            char file_string[100];
            strcpy(file_string, entry->d_name);
            file_count = extract_string_number(file_string);
            //printf("%s\n",entry->d_name);
            //_exit(1);

            count = 0 + INITIAL_REFERENCE_BITS;
            FILE *fp = fopen(argv[5], "r");
            //printf("line 267 count: %d\n",count);
            assert(fp);
            char c;
            char bits[512 + INITIAL_REFERENCE_BITS];
            if (fseek(fp, (file_count-1)*513, SEEK_SET) != 0) {
                printf("Error seeking in file");
                fclose(fp);
                _exit(1);
            }
            for(i=0; i<INITIAL_REFERENCE_BITS; i++)
            {
              bits[i] = 0;
            }
            while (1) {
              fscanf(fp, "%c", &c);
              if (c == '\n') break;
              else if (c == '0') bits[count] = 0;
              else if (c == '1') bits[count] = 1;
              else assert(0);
              count++;
            }
            //printf("count: %d\n",count);
            assert(count == 512 +  INITIAL_REFERENCE_BITS);
            fclose(fp);
            fp = fopen(filepath, "r");
            assert(fp);
            // Skip the header (a line of string)
            char buffer[4096];
            if (fgets(buffer, sizeof(buffer), fp) == NULL) {
                perror("Error reading header");
                fclose(fp);
                return 1;
            }

            //int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11;

            // Read the receiver's latency in the first line of the text file, i.e. before starting communication.
            //fscanf(fp, "%d,%d,%d,%d,%d\n", &a1, &a2, &a3, &a4, &a5);
		        //fscanf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10, &a11);
            //int init_bit_0 = a4;

            int count2 = 0 + INITIAL_REFERENCE_BITS;
            int latency[512 +  INITIAL_REFERENCE_BITS];

            for(i=0; i<INITIAL_REFERENCE_BITS; i++)
            { 
              latency[i] = 0;
            }

            //latency[count2 - 1] = init_bit_0;
            char *token;
            while(fgets(buffer, sizeof(buffer), fp) != NULL)
            {
                //fscanf(fp, "%d,%d,%d,%d,%d\n", &a1, &a2, &a3, &a4, &a5);
		            //fscanf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10, &a11);
                //latency[count2] = a4;
                int iter = 0, miss_number[uf+1];
                token = strtok(buffer, ",");
                while (token != NULL)
                {
                    miss_number[iter] = atoi(token);  // Convert token to integer
                    //printf("%d\n", miss_number[iter]);  // Do whatever you need with the number
                    token = strtok(NULL, ",");
                    //printf("%s\n",token);
                    iter++;
                }
                iter=0;
                latency[count2]=0;
                while(iter <= uf)
                {
                    if(iter >= threshold_n)
                    {    latency[count2] += miss_number[iter];
                    }
                    iter++;
                }

                count2++;
            }
            assert(count2 == 512 + INITIAL_REFERENCE_BITS);
            fclose(fp);
            //printf("count2: %d\n",count2);

            error = 0, error0to1 = 0, error1to0 = 0;
            for (i=((histlen == 0) ? 1 : histlen); i<count; i++) // Looping over the complete 512 bits. 
            {
                for (k=0; k<histlen; k++) // This loop check for the pattern.
                {
                    if (bits[k+i-histlen] != ((j >> (histlen - k - 1)) & 0x1)) break;
                }
                if (k == histlen)
                {
                    //if((j == 3 && t[j] == -3339) || (j == 7 && t[j] == -3679) )
                    //    printf("i is: %d, bit is:%d, latency difference: %d, threshold is: %d \n",i,bits[i], (latency[i] - latency[i-1]), t[j]);
                    /*if(latency[i] == 0 || latency[i-1] == 0)
                    {
                        printf("%d %d %d %d\n",i, latency[i], i-1, latency[i-1]);
                        printf("%s\n",filepath);
                        printf("latency value 0 should not be used in error calculation.\n");
                        assert(0);
                    }*/ 
                    if ((latency[i] - latency[i-1]) <= t[j])
                    {
                        if (bits[i] == 1)
                        {
                            error++;
                            error1to0 ++;
                            //if((j == 3 && t[j] == -3339) || (j == 7 && t[j] == -3679) )
                                //printf("bit position: %d, file_number:%d, error bit is: %d, latency difference: %d, threshold is: %d \n",i,file_count,bits[i], (latency[i] - latency[i-1]), t[j]); 
                                //fprintf(fptr,"bit position: %d, file_number:%d, error bit is: %d, latency difference: %d, threshold is: %d \n",i,file_count,bits[i], (latency[i] - latency[i-1]), t[j]); 
                        }
                    }
                    else if (bits[i] == 0)
                    {
                        error++;
                        error0to1 ++;
                        //if((j == 3 && t[j] == -3339) || (j == 7 && t[j] == -3679))
                            //printf("bit position: %d, file_number:%d, error bit is:%d, latency difference: %d, threshold is: %d \n",i,file_count,bits[i], (latency[i] - latency[i-1]), t[j]); 
                            //fprintf(fptr,"bit position: %d, file_number:%d, error bit is:%d, latency difference: %d, threshold is: %d \n",i,file_count,bits[i], (latency[i] - latency[i-1]), t[j]); 
                    }
                }
            }
            // Add the error to global_error.
            global_error += error;
            global_error0to1 += error0to1;
            global_error1to0 += error1to0;
        }
        closedir(dir);
        // End the loop for all the files in the directory here.
        if (global_error < min_error[j]) // This should be global_error 
        {
            min_error[j] = global_error;
            error_0to1[j] = global_error0to1;
            error_1to0[j] = global_error1to0;
            best_t[j] = t[j];
        }
    //}
  }
  error = 0;
  for (j=0; j<(1 << histlen); j++) {
    printf("min_error%d: %d, best_t%d: %d, error_1to0[%d]: %d, error_0to1[%d]: %d\n",j,min_error[j],j,best_t[j],j,error_1to0[j],j,error_0to1[j]);
    fprintf(fptr,"min_error%d: %d, best_t%d: %d, error_1to0[%d]: %d, error_0to1[%d]: %d\n",j,min_error[j],j,best_t[j],j,error_1to0[j],j,error_0to1[j]);
    error += min_error[j];
    error1to0 += error_1to0[j];
    error0to1 += error_0to1[j];
  }
  
  printf("Total error: %d, Total 1->0 error: %d, Total 0->1 error: %d\n", error, error1to0, error0to1);
  fprintf(fptr, "Total error: %d, Total 1->0 error: %d, Total 0->1 error: %d\n", error, error1to0, error0to1);
  // Close the file
  fclose(fptr);
  return 0;

}
