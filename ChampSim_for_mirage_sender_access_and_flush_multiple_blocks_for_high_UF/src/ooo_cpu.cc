#include "ooo_cpu.h"
#include "set.h"
#include "config.h"

// out-of-order core
O3_CPU ooo_cpu[NUM_CPUS]; 
uint64_t current_core_cycle[NUM_CPUS], stall_cycle[NUM_CPUS];
uint32_t SCHEDULING_LATENCY = 0, EXEC_LATENCY = 0;

void O3_CPU::initialize_core()
{
}
void O3_CPU::fseek_manual()
{
    int i=0;
    // Resetting files to beginning.
    pclose(trace_file_inc);
    trace_file_inc = popen(gunzip_command_inc, "r");
    pclose(trace_file_fl);
    trace_file_fl = popen(gunzip_command_fl, "r");
    // shifting trace_file_inc (a)before accessing a new element in LLC and (b)before filling a hole in LLC.
    cout << "Inside fseek_manual() "<<endl;
    while(i < INC_ARR_LEN*NUM_INSTR_INC_BLOCK)
    {    if (!fread(&dummy_current_instr, sizeof(input_instr), 1, trace_file_inc))
         {
            cout << "i is: "<<i<<" INC_ARR_LEN: "<<INC_ARR_LEN<<endl;
            assert(0);
         }
         i++;
         cout << "inside fseek i is: "<<i<<" INC_ARR_LEN: "<<INC_ARR_LEN<<" seen_error_correction: "<<seen_error_correction<<endl;
    }
    // shifting trace_file_fl before filling hole in LLC.
    i=0;
    if(seen_error_correction == 2)
    {
        //cout << "Hiiii"<<endl;
        while(i < INC_ARR_LEN)
	{   
		 if (!fread(&dummy_current_instr, sizeof(input_instr), 1, trace_file_fl))
		 {
				cout << "i is: "<<i<<" INC_ARR_LEN: "<<INC_ARR_LEN<<endl;
				assert(0);
		 }
		 i++;
		 cout << "inside fseek, incrementing fl file, i is: "<<i<<" INC_ARR_LEN: "<<INC_ARR_LEN<<endl;
	}
    }
}
void O3_CPU::memorize_num_instrs_read()
{
        if(FILL_HOLE > 0)
				{
        		if(ins_read_from_arr < NUM_INSTR_INC_BLOCK)
			{   
					 ins_read_from_arr ++;
					 cout << "Filling hole received : Read from arr file " << "ins_read_from_arr: "<< ins_read_from_arr<< endl;
			}
        } 
        else if(ERROR_CORRECTION_PHASE == 1)
        {
            if(ins_read_from_arr < NUM_INSTR_INC_BLOCK)
            {   
                 ins_read_from_arr ++;
                 cout << "Read from arr file ins_read_from_arr: "<< ins_read_from_arr<< endl;
            }
            else if(ins_read_from_arr == NUM_INSTR_INC_BLOCK && (ins_read_from_fen == 0 || ins_read_from_fl == 1))
            { 
                ins_read_from_fen ++;
                cout << "Read from fen file ins_read_from_fen: "<< ins_read_from_fen<< endl;
            }
            else if(ins_read_from_arr == NUM_INSTR_INC_BLOCK && ins_read_from_fen == 1 && ins_read_from_fl == 0)
            { 
                ins_read_from_fl ++;
                cout << "Read from fl file ins_read_from_fl: "<< ins_read_from_fl<< endl;
            }
        }
        /*else if(LLC_OCCUPANCY_CHECK_ONGOING == 0 && INC_ARR_LEN > 0 && read_increased_arr_instr == 0 && ( (HOLE_FILLING_PROCESS_ONGOING == 1 && DOUBLE_PROBE == 1) || !(HOLE_FILLING_PROCESS_ONGOING) ) )
        {
            if(ins_read_from_fen_rd < 3)
            {
                 ins_read_from_fen_rd ++;
                 cout << "INC_ARR_LEN: Read from fen_rd file " << "ins_read_from_fen_rd: "<< ins_read_from_fen_rd<< endl;
            }
            else if(ins_read_from_fen_rd == 3 && ins_read_from_arr < instr_to_read_this_time) //TODO: We can read more than 1 in one go.
            { 
                ins_read_from_arr ++;
                cout << "INC_ARR_LEN: Read from arr file" << " ins_read_from_arr: "<< ins_read_from_arr<< endl;
            }
            else if(ins_read_from_fen_rd < 6 && ins_read_from_arr == instr_to_read_this_time)
            { 
                ins_read_from_fen_rd ++;
                cout << "INC_ARR_LEN: Read from fen_rd file" << " ins_read_from_fen_rd: "<< ins_read_from_fen_rd<< endl;
            } 
        } */
        else if(LLC_OCC_CHK_PHASE == 1)
        {
            if(instr_flushed_beginning != /*UNROLLING_FACTOR_FOR_LLC_OCC_CHK*/ MAX_EXTRA_BLOCKS_POSSIBLE )
            {   // XXX Below lines are commented as we are not probing EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK. XXX
            //    if(/*EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK*/ INC_ARR_LEN != 0 && instr_flushed_beginning != /*EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK*/ MAX_EXTRA_BLOCKS_POSSIBLE)
             //   {
             //       if(seen_rdtsc == 1 && ins_read_from_arr < /*2*EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK*/ 2*1) // Assuming we flush only one element in the beginning.
            //        {    ins_read_from_arr ++;  
            //             cout << "In if Inside memorize_read_instr, ins_read_from_arr: " <<ins_read_from_arr<< endl;
            //        }
            //        else if(seen_rdtsc == 2 && ins_read_from_arr == /*2*EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK*/ 2*1 )
            //        {    instr_flushed_beginning ++; 
             //            cout << "In if Inside memorize_read_instr, instr_flushed_beginning: " <<instr_flushed_beginning<< endl;
             //       }
             //   }
             //   else
             //   {  
                    if(seen_rdtsc == 1 && seen_fence == 1 && ins_read_from_arr < 2*MAX_EXTRA_BLOCKS_POSSIBLE)
                    {    ins_read_from_arr ++ ;
                         //cout << "In else Inside memorize_read_instr, ins_read_from_arr: " <<ins_read_from_arr<< endl;
                    }
                    else if(seen_rdtsc == 2 && seen_fence == 2 && ins_read_from_arr == /* 2*UNROLLING_FACTOR_FOR_LLC_OCC_CHK */ 2*MAX_EXTRA_BLOCKS_POSSIBLE)
                    {    instr_flushed_beginning ++;
                         //cout << "In else Inside memorize_read_instr, instr_flushed_beginning: " <<instr_flushed_beginning<< endl;
                    }
               // }
            }
            else if(instr_flushed_beginning == /*UNROLLING_FACTOR_FOR_LLC_OCC_CHK*/ MAX_EXTRA_BLOCKS_POSSIBLE)
            {
                if(flush_more_address != 0)
								{		flush_more_address --;
                    //cout <<"In else if flush_more_address: " << flush_more_address<<endl;
                }
                else if(flush_more_address == 0)
                {
                    if(seen_rdtsc == 1 && seen_fence == 1 && ins_read_from_arr < 2*UNROLLING_FACTOR_FOR_LLC_OCC_CHK)
                    {   ins_read_from_arr ++ ;
                        //cout <<"In else if ins_read_from_arr: " << ins_read_from_arr<<endl;
                    }
                }
            }
        }
       /* else if(LLC_OCC_CHK_PHASE == 1 && ((seen_rdtsc == 2 && instr_flushed_beginning < EXTRA_ARR_SIZE) || seen_rdtsc == 4) && instr_flushed_beginning != UNROLLING_FACTOR)
        {
            instr_flushed_beginning ++;
            cout <<"instr_flushed_beginning: "<<instr_flushed_beginning<<" LLC_OCC_CHK: REDUCED_ARR_SIZE: "<<REDUCED_ARR_SIZE<<" ARR_MAX_LIMIT: "<<ARR_MAX_LIMIT<<endl;
        }
        else if(LLC_OCC_CHK_PHASE == 1 && instr_flushed_beginning == UNROLLING_FACTOR && flush_more_address != 0)
        {
            cout <<" LLC_OCC_CHK: REDUCED_ARR_SIZE: "<<REDUCED_ARR_SIZE<<" ARR_MAX_LIMIT: "<<ARR_MAX_LIMIT<< " Reduced flush_more_address, from "<<flush_more_address <<" to ";
            flush_more_address --;
            cout <<flush_more_address<<endl;
        } */
}
void O3_CPU::update_trace_file()
{
        // Code to switch between different trace files.
        if(FILL_HOLE > 0)
        {
            if(ins_read_from_arr < NUM_INSTR_INC_BLOCK)
            {
                 fptr = trace_file_inc;
                 cout << "Filling hole: fptr has trace_file_inc" <<endl;
            }
            else if(ins_read_from_arr == NUM_INSTR_INC_BLOCK)
            {
                cout <<" closing and opening file" << endl;
                // Close and open the file so the pointer is at the beginning.
                pclose(trace_file_inc);
                trace_file_inc = popen(gunzip_command_inc, "r");
                if (trace_file_inc == NULL) {
                    cerr << endl << "*** CANNOT REOPEN TRACE FILE: " << trace_string_inc << " ***" << endl;
                    assert(0);
                }
                fptr = trace_file;
                FILL_HOLE = 0;
                ins_read_from_arr = 0;
            }
        }
        /*else if(LLC_OCCUPANCY_CHECK_ONGOING == 0 && INC_ARR_LEN > 0 && read_increased_arr_instr == 0 && cpu == 0 && ( (HOLE_FILLING_PROCESS_ONGOING == 1 && DOUBLE_PROBE == 1) || !(HOLE_FILLING_PROCESS_ONGOING) ) )
        {
            cout << "INC_ARR_LEN: ins_read_from_fen_rd: "<< ins_read_from_fen_rd << endl;
            if(ins_read_from_fen_rd < 3)
            {
                 if(ins_read_from_fen_rd == 0)
                 {
										 pclose(trace_file_fen_rd);
										 trace_file_fen_rd = popen(gunzip_command_fen_rd, "r");
										 if (trace_file_fen_rd == NULL) 
                     {
										 		cerr << endl << "*** CANNOT REOPEN TRACE FILE: " << trace_string_fen_rd << " ***" << endl;
										 		assert(0);
										 }
                     if(ins_read_from_arr == 0)
                     {
                         pclose(trace_file_inc);          // Apparently, this is not needed.
                         trace_file_inc = popen(gunzip_command_inc, "r");
                         if (trace_file_inc == NULL) 
                         {
                             cerr << endl << "*** CANNOT REOPEN TRACE FILE: " << trace_string_inc << " ***" << endl;
                             assert(0);
                         }
                     }
                    //XXX Major change to read only one block at a time i.e. Unrolling_factor == 1 for the incremented array length. The below code can be used with the ellaborative latency cycles later.
										 //Update instr_to_read_this_time
										 //instr_to_read_this_time += UNROLLING_FACTOR < INC_ARR_LEN ? 
                     //                 ((INC_ARR_LEN * NUM_INSTR_INC_BLOCK - instr_to_read_this_time)/(UNROLLING_FACTOR * NUM_INSTR_INC_BLOCK)) > 0 ? 
                     //                 (UNROLLING_FACTOR * NUM_INSTR_INC_BLOCK) : 
                     //                 (INC_ARR_LEN * NUM_INSTR_INC_BLOCK - instr_to_read_this_time) % (UNROLLING_FACTOR * NUM_INSTR_INC_BLOCK) : 
                     //                 (INC_ARR_LEN * NUM_INSTR_INC_BLOCK); 
                 
                      instr_to_read_this_time += NUM_INSTR_INC_BLOCK; // Read one array block at a time.
                 }
                 fptr = trace_file_fen_rd;
                 cout << "INC_ARR_LEN: fptr has trace_file_fen_rd, fptr: " <<fptr<<" Unrolling factor:"<< UNROLLING_FACTOR<<" instr_to_read_this_time: " << instr_to_read_this_time<<" INC_ARR_LEN: "<<INC_ARR_LEN<<endl;
            }
            else if(ins_read_from_fen_rd == 3 && ins_read_from_arr < instr_to_read_this_time) //TODO: We can read more than 1 in one go.
            { 
                fptr = trace_file_inc;
                cout << "INC_ARR_LEN: fptr has trace_file_inc, fptr: " <<fptr <<" Unrolling factor:"<< UNROLLING_FACTOR<<" instr_to_read_this_time: " << instr_to_read_this_time<<" INC_ARR_LEN: "<<INC_ARR_LEN<<endl;
            }
            else if(ins_read_from_fen_rd < 6 && ins_read_from_arr == instr_to_read_this_time)
            { 
                fptr = trace_file_fen_rd;
                cout << "INC_ARR_LEN: fptr has trace_file_fen_rd, fptr: " <<fptr <<" Unrolling factor:"<< UNROLLING_FACTOR<<" instr_to_read_this_time: " << instr_to_read_this_time<<" INC_ARR_LEN: "<<INC_ARR_LEN<<endl;
            }
            else if(ins_read_from_fen_rd == 6 && ins_read_from_arr == instr_to_read_this_time)
            { 
                if(INC_ARR_LEN * NUM_INSTR_INC_BLOCK > instr_to_read_this_time)
                {
											ins_read_from_fen_rd = 0;
                      cout<<"INC_ARR_LEN * NUM_INSTR_INC_BLOCK : "<<INC_ARR_LEN * NUM_INSTR_INC_BLOCK<<"instr_to_read_this_time: "<< 
                             instr_to_read_this_time<<endl;
                }
                else
                {
											cout <<"INC_ARR_LEN: closing and opening file" << endl; 
											pclose(trace_file_inc);  
											trace_file_inc = popen(gunzip_command_inc, "r");
											if (trace_file_inc == NULL) {
													cerr << endl << "*** CANNOT REOPEN TRACE FILE: " << trace_string_inc << " ***" << endl;
													assert(0);
											}
											pclose(trace_file_fen_rd);
											trace_file_fen_rd = popen(gunzip_command_fen_rd, "r");
											if (trace_file_fen_rd == NULL) {
													cerr << endl << "*** CANNOT REOPEN TRACE FILE: " << trace_string_fen_rd << " ***" << endl;
													assert(0);
											}
											fptr = trace_file;
											ins_read_from_arr = 0;
											ins_read_from_fen_rd = 0;
											cout << "Completed reading extra instructions." <<endl;
											read_increased_arr_instr = 1;
                      instr_to_read_this_time = 0;
                }
            }
        } */
        else if(ERROR_CORRECTION_PHASE == 1)
        {
            if(ins_read_from_arr != NUM_INSTR_INC_BLOCK)
            {   
                 fptr = trace_file_inc;
                 //cout << "fptr has trace_file_inc, ins_read_from_arr "<<ins_read_from_arr<<endl;
            }
            else if(ins_read_from_arr == NUM_INSTR_INC_BLOCK && (ins_read_from_fen == 0 || ins_read_from_fl == 1))
            {
                 fptr = trace_file_fen;
                 //cout << "fptr has trace_file_fen" <<endl;
            }
            else if(ins_read_from_arr == NUM_INSTR_INC_BLOCK && ins_read_from_fen == 1 && ins_read_from_fl == 0)
            {
                 fptr = trace_file_fl;
                 //cout << "fptr has trace_file_fl" <<endl;
            }
            if(ins_read_from_arr == NUM_INSTR_INC_BLOCK && ins_read_from_fen == 2 && ins_read_from_fl == 1)
            {
                //cout <<" closing and opening file" << endl; 
                // Close and open the file so the pointer is at the beginning.
                pclose(trace_file_inc);
                trace_file_inc = popen(gunzip_command_inc, "r");
                if (trace_file_inc == NULL) {
                    cerr << endl << "*** CANNOT REOPEN TRACE FILE: " << trace_string_inc << " ***" << endl;
                    assert(0);
                }
                pclose(trace_file_fen);
                trace_file_fen = popen(gunzip_command_fen, "r");
                if (trace_file_fen == NULL) {
                    cerr << endl << "*** CANNOT REOPEN TRACE FILE: " << trace_string_fen << " ***" << endl;
                    assert(0);
                }
                pclose(trace_file_fl);
                trace_file_fl = popen(gunzip_command_fl, "r");
                if (trace_file_fl == NULL) {
                    cerr << endl << "*** CANNOT REOPEN TRACE FILE: " << trace_string_fl << " ***" << endl;
                    assert(0);
                }
                // Switch-off error-correction phase.
                fptr = trace_file;
                ERROR_CORRECTION_PHASE = 0;
                ins_read_from_arr = 0;
                ins_read_from_fl = 0;
                ins_read_from_fen = 0;
                cout << "Error_correction phase switched off" <<endl;
            }
        }
        else if(LLC_OCC_CHK_PHASE == 1)
        {
            if(instr_flushed_beginning != MAX_EXTRA_BLOCKS_POSSIBLE /*UNROLLING_FACTOR_FOR_LLC_OCC_CHK*/) //XXX Keeping it 1 helps as we might not be able to create UNROLLING_FACTOR_FOR_LLC_OCC_CHK holes if all of these are missing from LLC. In such case we will have only one hole in LLC.
            {
               // if(/*EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK*/ INC_ARR_LEN != 0 /*&& instr_flushed_beginning != MAX_EXTRA_BLOCKS_POSSIBLE */ /*EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK*/)
              //  {   cout << "in if" << endl;
              //      if(seen_rdtsc == 0)
              //          fptr = trace_file_fen_rd;
              //      else if(seen_rdtsc == 1 && ins_read_from_arr < 2*MAX_EXTRA_BLOCKS_POSSIBLE /*2*EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK*/)
              //      {   fptr = trace_file_arr_access;
              //          if(turn_to_traverse_arr == 0)
              //              turn_to_traverse_arr = MAX_EXTRA_BLOCKS_POSSIBLE /*EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK*/;
              //      }
              //      else if(seen_rdtsc == 1 && ins_read_from_arr == 2*MAX_EXTRA_BLOCKS_POSSIBLE /*2*EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK*/)
              //          fptr = trace_file_fen_rd;
              //      else if(seen_rdtsc == 2 && ins_read_from_arr == 2*MAX_EXTRA_BLOCKS_POSSIBLE /*2*EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK*/)
              //          fptr = trace_file_fl_llc_occ;
              //  }
              // else
              //  {   
                    /*if(seen_rdtsc == 2 && ins_read_from_arr == 2*EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK)
                    {   seen_rdtsc = 0;
                        ins_read_from_arr = 0;
                        cout << "Resetting seen_rdtsc to: "<<seen_rdtsc<<" ins_read_from_arr: "<<ins_read_from_arr<<endl;
                    } */
                    if(seen_rdtsc == 0)
                        fptr = trace_file_fen_rd;
                    else if(seen_rdtsc == 1 && seen_fence == 0)
                        fptr = trace_file_fen;
                    else if(seen_rdtsc == 1 && ins_read_from_arr < 2*MAX_EXTRA_BLOCKS_POSSIBLE && seen_fence == 1)
                    {   fptr = trace_file_arr_access;
                        if(turn_to_traverse_arr == 0)
                        {   turn_to_traverse_arr = MAX_EXTRA_BLOCKS_POSSIBLE ;
                            //cout << "turn_to_traverse_arr: "<< turn_to_traverse_arr << endl;
                        }
                    }
                    else if(seen_rdtsc == 1 && ins_read_from_arr == 2*MAX_EXTRA_BLOCKS_POSSIBLE && seen_fence == 1)
                        fptr = trace_file_fen_rd;
                   else if(seen_rdtsc == 2 && ins_read_from_arr == 2*MAX_EXTRA_BLOCKS_POSSIBLE && seen_fence == 1)  // CHeck where to reset seen_fence
                        fptr = trace_file_fen;
                    else if(seen_rdtsc == 2 && ins_read_from_arr == 2*MAX_EXTRA_BLOCKS_POSSIBLE && seen_fence == 2)
                        fptr = trace_file_fl_llc_occ;
              //  }
            }
            else if(instr_flushed_beginning == MAX_EXTRA_BLOCKS_POSSIBLE )
            {
                if(flush_more_address != 0)
                    fptr = trace_file_fl_llc_occ;
                else if(flush_more_address == 0)
                {
                    if(seen_rdtsc == 2 && seen_fence == 2 && still_have_array_elements_to_traverse)
                    {    
                        seen_rdtsc = 0;
                        ins_read_from_arr = 0;
                        seen_fence = 0;
                    }
                    if(seen_rdtsc == 0)
                        fptr = trace_file_fen_rd;
                    else if(seen_rdtsc == 1 && seen_fence == 0 && ins_read_from_arr < 2*UNROLLING_FACTOR_FOR_LLC_OCC_CHK)
                    {
                        fptr = trace_file_fen;
                    }
                    else if(seen_rdtsc == 1 && seen_fence == 1 && ins_read_from_arr < 2*UNROLLING_FACTOR_FOR_LLC_OCC_CHK)
                    {  
                        if(turn_to_traverse_arr == 0)
                            turn_to_traverse_arr = UNROLLING_FACTOR_FOR_LLC_OCC_CHK /*- EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK*/; 
                        fptr = trace_file_arr_access;
                    }
                    else if(seen_rdtsc == 1 && seen_fence == 1 && ins_read_from_arr == 2*UNROLLING_FACTOR_FOR_LLC_OCC_CHK)
                        fptr = trace_file_fen_rd;
                    else if(seen_rdtsc == 2 && seen_fence == 1 && ins_read_from_arr == 2*UNROLLING_FACTOR_FOR_LLC_OCC_CHK)
                        fptr = trace_file_fen;
                    else if(seen_rdtsc == 2 && seen_fence == 2 && !still_have_array_elements_to_traverse)
                    {
                        pclose(trace_file_fl);
												trace_file_fl = popen(gunzip_command_fl, "r");
												if (trace_file_fl == NULL) 
                        {
														cerr << endl << "*** CANNOT REOPEN TRACE FILE: " << trace_string_fl << " ***" << endl;
														assert(0);
												}
                        pclose(trace_file_fen_rd);
                        trace_file_fen_rd = popen(gunzip_command_fen_rd, "r");
                        if (trace_file_fen_rd == NULL) 
                        {
                            cerr << endl << "*** CANNOT REOPEN TRACE FILE: " << trace_string_fen_rd << " ***" << endl;
                            assert(0);
                        }
                        pclose(trace_file_fen);
                        trace_file_fen = popen(gunzip_command_fen, "r");
                        if (trace_file_fen == NULL) 
                        {
                            cerr << endl << "*** CANNOT REOPEN TRACE FILE: " << trace_string_fen << " ***" << endl;
                            assert(0);
                        }
                        fptr = trace_file;
                        //cout <<"line 374"<<endl;
                        ins_read_from_arr = 0;
                    }
                }
            }
        }
       /* else if(LLC_OCC_CHK_PHASE == 1 && ((seen_rdtsc == 2 && instr_flushed_beginning < EXTRA_ARR_SIZE) || seen_rdtsc == 4) && instr_flushed_beginning != UNROLLING_FACTOR)
        { 
            fptr = trace_file_fl_llc_occ;
            cout << "fptr has trace_file_fl "<< "instr_flushed_beginning: "<< instr_flushed_beginning <<" EXTRA_ARR_SIZE: " << EXTRA_ARR_SIZE <<" seen_rdtsc: " << seen_rdtsc <<" UNROLLING_FACTOR: "<< UNROLLING_FACTOR <<" LLC_OCC_CHK: REDUCED_ARR_SIZE: "<<REDUCED_ARR_SIZE<<" ARR_MAX_LIMIT: "<<ARR_MAX_LIMIT<<endl;
        } */
       /* else if(LLC_OCC_CHK_PHASE == 1 && instr_flushed_beginning == UNROLLING_FACTOR && flush_more_address != 0)
        {
            fptr = trace_file_fl_llc_occ;
            cout<<"LLC_OCC_CHK: Updated fptr to flush cache blocks further"<<" LLC_OCC_CHK: REDUCED_ARR_SIZE: "<<REDUCED_ARR_SIZE<<" ARR_MAX_LIMIT: "<<ARR_MAX_LIMIT<<endl;
        }*/
        else if(access_to_fill_empty_slots > 0)
        {
            cout<<" filling empty slots"<<endl;
            fptr = trace_file_arr_access;
        }
        else
        {
            fptr = trace_file;
        }
}

void O3_CPU::handle_branch()
{
    // actual processors do not work like this but for easier implementation,
    // we read instruction traces and virtually add them in the ROB
    // note that these traces are not yet translated and fetched 

    uint8_t continue_reading = 1;
    uint32_t num_reads = 0;
    instrs_to_read_this_cycle = FETCH_WIDTH;

    // first, read PIN trace
    while (continue_reading) {

        size_t instr_size = knob_cloudsuite ? sizeof(cloudsuite_instr) : sizeof(input_instr);
        

        // This is to switch between the trace-files for the receiver.
        if(!knob_cloudsuite)
            update_trace_file(); 

        if (knob_cloudsuite) {
            if (!fread(&current_cloudsuite_instr, instr_size, 1, trace_file)) {
                // reached end of file for this trace
                cout << "*** Reached end of trace for Core: " << cpu << " Repeating trace: " << trace_string << endl; 

                // close the trace file and re-open it
                pclose(trace_file);
                trace_file = popen(gunzip_command, "r");
                if (trace_file == NULL) {
                    cerr << endl << "*** CANNOT REOPEN TRACE FILE: " << trace_string << " ***" << endl;
                    assert(0);
                }
            } else { // successfully read the trace

                // copy the instruction into the performance model's instruction format
                ooo_model_instr arch_instr;
                int num_reg_ops = 0, num_mem_ops = 0;

                arch_instr.instr_id = instr_unique_id;
                arch_instr.ip = current_cloudsuite_instr.ip;
                arch_instr.is_branch = current_cloudsuite_instr.is_branch;
                arch_instr.branch_taken = current_cloudsuite_instr.branch_taken;

                arch_instr.asid[0] = current_cloudsuite_instr.asid[0];
                arch_instr.asid[1] = current_cloudsuite_instr.asid[1];

                for (uint32_t i=0; i<MAX_INSTR_DESTINATIONS; i++) {
                    arch_instr.destination_registers[i] = current_cloudsuite_instr.destination_registers[i];
                    arch_instr.destination_memory[i] = current_cloudsuite_instr.destination_memory[i];
                    arch_instr.destination_virtual_address[i] = current_cloudsuite_instr.destination_memory[i];

                    if (arch_instr.destination_registers[i])
                        num_reg_ops++;
                    if (arch_instr.destination_memory[i]) {
                        num_mem_ops++;

                        // update STA, this structure is required to execute store instructios properly without deadlock
                        if (num_mem_ops > 0) {
#ifdef SANITY_CHECK
                            if (STA[STA_tail] < UINT64_MAX) {
                                if (STA_head != STA_tail)
                                    assert(0);
                            }
#endif
                            STA[STA_tail] = instr_unique_id;
                            STA_tail++;

                            if (STA_tail == STA_SIZE)
                                STA_tail = 0;
                        }
                    }
                }

                for (int i=0; i<NUM_INSTR_SOURCES; i++) {
                    arch_instr.source_registers[i] = current_cloudsuite_instr.source_registers[i];
                    arch_instr.source_memory[i] = current_cloudsuite_instr.source_memory[i];
                    arch_instr.source_virtual_address[i] = current_cloudsuite_instr.source_memory[i];

                    if (arch_instr.source_registers[i])
                        num_reg_ops++;
                    if (arch_instr.source_memory[i])
                        num_mem_ops++;
                }

                arch_instr.num_reg_ops = num_reg_ops;
                arch_instr.num_mem_ops = num_mem_ops;
                if (num_mem_ops > 0) 
                    arch_instr.is_memory = 1;

                // virtually add this instruction to the ROB
                if (ROB.occupancy < ROB.SIZE) {
                    uint32_t rob_index = add_to_rob(&arch_instr);
                    num_reads++;

                    // branch prediction
                    if (arch_instr.is_branch) {

                        DP( if (warmup_complete[cpu]) {
                        cout << "[BRANCH] instr_id: " << instr_unique_id << " ip: " << hex << arch_instr.ip << dec << " taken: " << +arch_instr.branch_taken << endl; });

                        num_branch++;

                        /*
                        uint8_t branch_prediction;
                        // for faster simulation, force perfect prediction during the warmup
                        // note that branch predictor is still learning with real branch results
                        if (all_warmup_complete == 0)
                            branch_prediction = arch_instr.branch_taken; 
                        else
                            branch_prediction = predict_branch(arch_instr.ip);
                        */
                        uint8_t branch_prediction = predict_branch(arch_instr.ip);
                        
                        if (arch_instr.branch_taken != branch_prediction) {
                            branch_mispredictions++;

                            DP( if (warmup_complete[cpu]) {
                            cout << "[BRANCH] MISPREDICTED instr_id: " << instr_unique_id << " ip: " << hex << arch_instr.ip << dec;
                            cout << " taken: " << +arch_instr.branch_taken << " predicted: " << +branch_prediction << endl; });

                            // halt any further fetch this cycle
                            instrs_to_read_this_cycle = 0;

                            // and stall any additional fetches until the branch is executed
                            fetch_stall = 1; 

                            ROB.entry[rob_index].branch_mispredicted = 1;
                        }
                        else {
                            if (branch_prediction == 1) {
                                // if we are accurately predicting a branch to be taken, then we can't possibly fetch down that path this cycle,
                                // so we have to wait until the next cycle to fetch those
                                instrs_to_read_this_cycle = 0;
                            }

                            DP( if (warmup_complete[cpu]) {
                            cout << "[BRANCH] PREDICTED    instr_id: " << instr_unique_id << " ip: " << hex << arch_instr.ip << dec;
                            cout << " taken: " << +arch_instr.branch_taken << " predicted: " << +branch_prediction << endl; });
                        }

                        last_branch_result(arch_instr.ip, arch_instr.branch_taken);
                    }

                    //if ((num_reads == FETCH_WIDTH) || (ROB.occupancy == ROB.SIZE))
                    if ((num_reads >= instrs_to_read_this_cycle) || (ROB.occupancy == ROB.SIZE))
                        continue_reading = 0;
                }
                instr_unique_id++;
            }
        } else {
            if (!fread(&current_instr, instr_size, 1, fptr)) {
                // reached end of file for this trace
               
                if(fptr == trace_file) // Print only for the main trace file on repeat.
                    cout << "*** Reached end of trace for Core: " << cpu <<" fptr: "<<fptr<< " Repeating trace: " << trace_string <<" unique instr ID: "<< instr_unique_id<<" num_retired:  "<<num_retired << endl; 

                // close the trace file and re-open it
                //cout << "closed trace file of pointer : "<<fptr<<endl;
                pclose(fptr);
								if(ERROR_CORRECTION_PHASE == 1)
                { 
                    if(ins_read_from_arr != NUM_INSTR_INC_BLOCK)
										 {   trace_file_inc = popen(gunzip_command_inc, "r");
                         //cout << " opened arr inc file "<< gunzip_command_inc<< endl;
                     }
								     else if(ins_read_from_arr == NUM_INSTR_INC_BLOCK /*&& ins_read_from_fen != 4*/)
										 {
                         trace_file_fen = popen(gunzip_command_fen, "r");
                         //cout << " line 556: opened fence file" <<endl;
                     }
                }
               // else if(LLC_OCCUPANCY_CHECK_ONGOING == 0 && INC_ARR_LEN > 0 && read_increased_arr_instr == 0 && cpu == 0)
                //{
								//		if(ins_read_from_fen_rd < 3)
								//		{
                 //        trace_file_fen_rd = popen(gunzip_command_fen_rd, "r");
                  //       cout << " line 564: opened fence rdtsc file" <<endl;
									//	}
								//		else if(ins_read_from_fen_rd == 3 && ins_read_from_arr < instr_to_read_this_time /*NUM_INSTR_INC_BLOCK*/) //TODO: We can read more than 1 in one go.
							//			{ 
							//			     trace_file_inc = popen(gunzip_command_inc, "r");
               //          cout << " opened arr inc file "<< gunzip_command_inc<< endl;
							//			}
							//			else if(ins_read_from_fen_rd < 6 && ins_read_from_arr == instr_to_read_this_time/*NUM_INSTR_INC_BLOCK*/)
							//			{ 
               //          trace_file_fen_rd = popen(gunzip_command_fen_rd, "r");
               //          cout << " line 574: opened fence rdtsc file" <<endl;
							//			} 
							//	}
                else if(LLC_OCC_CHK_PHASE == 1)
                {
										if(instr_flushed_beginning != MAX_EXTRA_BLOCKS_POSSIBLE /* UNROLLING_FACTOR_FOR_LLC_OCC_CHK*/)
										{
												/*if(EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK != 0 && instr_flushed_beginning != EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK)
												{
														if(seen_rdtsc == 0)
														{
																 trace_file_fen_rd = popen(gunzip_command_fen_rd, "r");
																 cout << " line 586: opened fence rdtsc file" <<endl;
														}
														else if(seen_rdtsc == 1 && ins_read_from_arr < 2*EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK)
														{
                                trace_file_arr_access = popen(gunzip_command_arr_access, "r");
																cout << " opened arr_access file for LLC_OCC_CHK." <<endl;
														}
														else if(seen_rdtsc == 1 && ins_read_from_arr == 2*EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK)
														{
                                 trace_file_fen_rd = popen(gunzip_command_fen_rd, "r");
                                 cout << " line 596: opened fence rdtsc file" <<endl;
                            }
														else if(seen_rdtsc == 2 && ins_read_from_arr == 2*EXTRA_ARR_SIZE_FOR_LLC_OCC_CHK)
														{
																 trace_file_fl_llc_occ = popen(gunzip_command_fl_llc_occ, "r");
																 cout << " opened flush file llc occ" <<endl;
														}
												}	
												else
												{ */
														if(seen_rdtsc == 0)
														{
                                 trace_file_fen_rd = popen(gunzip_command_fen_rd, "r");
                                 //cout << " line 609: opened fence rdtsc file" <<endl;
                            }
														else if(seen_rdtsc == 1 && seen_fence == 0)
														{
                                 trace_file_fen = popen(gunzip_command_fen, "r");
                            }
														else if(seen_rdtsc == 1 && seen_fence == 1 && ins_read_from_arr < 2*MAX_EXTRA_BLOCKS_POSSIBLE /*2*UNROLLING_FACTOR_FOR_LLC_OCC_CHK*/)
														{
                                 trace_file_arr_access = popen(gunzip_command_arr_access, "r");
                                 //cout << " opened arr_access file for LLC_OCC_CHK." <<endl;
														}
														else if(seen_rdtsc == 1 && seen_fence == 1 && ins_read_from_arr == 2*MAX_EXTRA_BLOCKS_POSSIBLE /*2*UNROLLING_FACTOR_FOR_LLC_OCC_CHK*/)
														{
                                 trace_file_fen_rd = popen(gunzip_command_fen_rd, "r");
                                 //cout << " line 619: opened fence rdtsc file" <<endl;
                            }
														else if(seen_rdtsc == 2 && seen_fence == 1 && ins_read_from_arr == 2*MAX_EXTRA_BLOCKS_POSSIBLE)
														{
                                 trace_file_fen = popen(gunzip_command_fen, "r");
                            }
														else if(seen_rdtsc == 2 && seen_fence == 2 && ins_read_from_arr == 2*MAX_EXTRA_BLOCKS_POSSIBLE /*2*UNROLLING_FACTOR_FOR_LLC_OCC_CHK*/)
														{
                                 trace_file_fl_llc_occ = popen(gunzip_command_fl_llc_occ, "r");
                                 //cout << " opened flush file llc occ" <<endl;
                            }
											//	}
                    }
                    else if(instr_flushed_beginning == MAX_EXTRA_BLOCKS_POSSIBLE /*UNROLLING_FACTOR_FOR_LLC_OCC_CHK*/)
										{
												if(flush_more_address != 0)
												{
														 trace_file_fl_llc_occ = popen(gunzip_command_fl_llc_occ, "r");
														 //cout << " opened flush file llc occ" <<endl;
												}
												else if(flush_more_address == 0)
												{
														if(seen_rdtsc == 0)
														{
                                 trace_file_fen_rd = popen(gunzip_command_fen_rd, "r");
                                 //cout << " line 640: opened fence rdtsc file" <<endl;
                            }
														else if(seen_rdtsc == 1 && seen_fence == 0 && ins_read_from_arr < 2*UNROLLING_FACTOR_FOR_LLC_OCC_CHK)
														{
                                 trace_file_fen = popen(gunzip_command_fen, "r");
                            }
														else if(seen_rdtsc == 1 && seen_fence == 1 && ins_read_from_arr < 2*UNROLLING_FACTOR_FOR_LLC_OCC_CHK)
														{
																trace_file_arr_access = popen(gunzip_command_arr_access, "r");
                                //cout << " opened arr_access file for LLC_OCC_CHK." <<endl;
														}
														else if(seen_rdtsc == 1 && seen_fence == 1 && ins_read_from_arr == 2*UNROLLING_FACTOR_FOR_LLC_OCC_CHK)
														{
                                 trace_file_fen_rd = popen(gunzip_command_fen_rd, "r");
                                 //cout << " line 650: opened fence rdtsc file" <<endl;
                            }
														else if(seen_rdtsc == 2 && seen_fence == 1 && ins_read_from_arr == 2*UNROLLING_FACTOR_FOR_LLC_OCC_CHK)
														{
                                 trace_file_fen = popen(gunzip_command_fen, "r");
                            }
												}
										}
                }   
        /*        else if(LLC_OCC_CHK_PHASE == 1 && ((seen_rdtsc == 2 && instr_flushed_beginning < EXTRA_ARR_SIZE) || seen_rdtsc == 4) && instr_flushed_beginning != UNROLLING_FACTOR) 
                {
                     trace_file_fl_llc_occ = popen(gunzip_command_fl_llc_occ, "r");
                     cout << " opened flush file llc occ" <<endl;
                }
                else if(flush_more_address != 0 && LLC_OCC_CHK_PHASE == 1 && instr_flushed_beginning != UNROLLING_FACTOR)
                {
                     trace_file_fl_llc_occ = popen(gunzip_command_fl_llc_occ, "r");
                     cout << " opened flush file for flush_more_address" <<endl;
                }*/
								else if(access_to_fill_empty_slots > 0)
								{
                     trace_file_arr_access = popen(gunzip_command_arr_access, "r");
                     //cout << " opened arr_access file for filling up emptied slots." <<endl;
								}
                else if(flush_more_address != 0)
                {
                     trace_file_fl_llc_occ = popen(gunzip_command_fl_llc_occ, "r");
                     //cout << " Reopened flush file to flush more address. " <<endl;
                }
                else
                {
                     trace_file = popen(gunzip_command, "r");
                     //cout << "Reopened base trace file." << endl;
                }

                // cpu == 0 is for receiver and cpu == 1 is for sender.
                if(cpu == 0)
                {
                    if (trace_file == NULL ||trace_file_inc == NULL || trace_file_fl_llc_occ == NULL || trace_file_fl == NULL || trace_file_fen_rd == NULL || trace_file_arr_access == NULL ) 
                    {
                        cerr << endl << "*** CANNOT REOPEN TRACE FILE "<< " ***" << endl;
                        assert(0);
                    }
                }
                else if(cpu == 1)
                {
                    if (trace_file == NULL ) 
                    {
                        cerr << endl << "*** CANNOT REOPEN TRACE FILE "<< " ***" << endl;
                        assert(0);
                    }
                }
            } else { // successfully read the trace
                // copy the instruction into the performance model's instruction format
                memorize_num_instrs_read();
                ooo_model_instr arch_instr;
                int num_reg_ops = 0, num_mem_ops = 0;

                arch_instr.warmup = current_instr.warmup; // read warmup flag
                arch_instr.wait = current_instr.wait; // read wait flag
                arch_instr.fence = current_instr.fence; // read fence flag
                arch_instr.rdtsc = current_instr.rdtsc; // read rdtsc flag
                arch_instr.error_correction = current_instr.error_correction; // read error_correction flag
                arch_instr.double_probe_end = current_instr.double_probe_end;
                arch_instr.llc_occ_chk_end = current_instr.llc_occ_chk_end;
                arch_instr.llc_occ_chk = current_instr.llc_occ_chk;
                
                // Identifying active blocks ip range for sender.
                if(TURN == 1 && cpu == 1 && sender_seen_wait_once == 1)
                {
                    if(SENDER_ACTIVE_IP_START == 0 && SENDER_ACTIVE_IP_END == 0)
                    {    
                        SENDER_ACTIVE_IP_START = current_instr.ip;
                        SENDER_ACTIVE_IP_END = current_instr.ip;
                    }
                    else
                    {
                        if(current_instr.ip < SENDER_ACTIVE_IP_START)
                        {
                            SENDER_ACTIVE_IP_START = current_instr.ip;
                        }
                        if(current_instr.ip > SENDER_ACTIVE_IP_END)
                        {
                            SENDER_ACTIVE_IP_END = current_instr.ip;
                        }
                    }
                }

                if(current_instr.double_probe_start == 1)
                { 
                    if(HOLE_FILLING_PROCESS_ONGOING == 1)
                        DOUBLE_PROBE = 1;
                    cout << "Inside double_probe start, DOUBLE_PROBE is: "<<(int)DOUBLE_PROBE<<" bit_revealed_is: "<<bit_revealed_is<<endl;
                }
                if(current_instr.llc_occ_chk_end == 1)
                {  
                    if(LLC_OCC_CHK_PHASE == 1) 
                    {
                        LLC_OCC_CHK_PHASE = 0;
                        seen_rdtsc = 0;
                        seen_fence = 0;
                        instr_flushed_beginning = 0;
                        if( REDUCED_ARR_INC_ADDR != LAST_ARR_INC_ADDR && dont_fill_empty_slot == 0 )
                            access_to_fill_empty_slots = MAX_EXTRA_BLOCKS_POSSIBLE;
                        else if(REDUCED_ARR_INC_ADDR != LAST_ARR_INC_ADDR && dont_fill_empty_slot == 1)
                        {
                            access_to_fill_empty_slots = 0;
                            LAST_ARR_INC_ADDR = REDUCED_ARR_INC_ADDR;
                            cout << "updated LAST_ARR_INC_ADDR: "<<LAST_ARR_INC_ADDR <<endl;
                        }

                    }
                    cout <<" cycle: "<<current_core_cycle[cpu]<< "received, LLC_OCC_CHK: switched Off seen_rdtsc and LLC_OCC_CHK_PHASE "<<" INC_ARR_LEN: "<<INC_ARR_LEN<<" REDUCED_ARR_SIZE: " <<REDUCED_ARR_SIZE <<" ARR_MAX_LIMIT: "<<ARR_MAX_LIMIT<<" FIRST_ARR_INC_ADDR: "<<FIRST_ARR_INC_ADDR<<" LAST_ARR_INC_ADDR: "<<LAST_ARR_INC_ADDR<<endl;
                    cout <<"received access_to_fill_empty_slots: " << access_to_fill_empty_slots <<" REDUCED_ARR_INC_ADDR: "<<REDUCED_ARR_INC_ADDR << endl;
                }
                if(current_instr.llc_occ_chk != 0)
                {
                    if((INC_ARR_LEN > 0) && (CURR_LLC_OCC_CHK_ITR == 0 || (CURR_LLC_OCC_CHK_ITR >0 && dont_fill_empty_slot == 1))) // Considering there is possibility of error if array lenght is incremented and it is first iteration of llc_occ_chk or array lenght is reduced in first iteration of llc_occ_chk.
                    {
                        seen_llc_occ_chk ++;
                        continue_reading = 0;
                        traversed_element = REDUCED_ARR_SIZE;
                        traversed_incremented_element = LAST_ARR_INC_ADDR;
                        still_have_array_elements_to_traverse = 1;
                        // Resetting dont_fill_empty_slot here so that this variable can be reset between the multiple iterations of llc_occ_chk.
                        dont_fill_empty_slot = 0;
                        cout<<"cycle: "<<current_core_cycle[cpu]<<" received, starting LLC_OCC_CHK REDUCED_ARR_SIZE: "<<REDUCED_ARR_SIZE<<" ARR_MAX_LIMIT: "<<ARR_MAX_LIMIT<<" llc_occ_chk has started,"<<" llc_occ_chk: "<<current_instr.llc_occ_chk<<" FIRST_ARR_INC_ADDR: "<<FIRST_ARR_INC_ADDR<<" LAST_ARR_INC_ADDR: "<<LAST_ARR_INC_ADDR<<endl;
                   }
                   else
                   {
                        cout<<"cycle: "<<current_core_cycle[cpu]<<" received,No need to switch ON llc_occ_chk. REDUCED_ARR_SIZE: "<<REDUCED_ARR_SIZE<<" ARR_MAX_LIMIT: "<<ARR_MAX_LIMIT<<" FIRST_ARR_INC_ADDR: "<<FIRST_ARR_INC_ADDR<<" LAST_ARR_INC_ADDR: "<<LAST_ARR_INC_ADDR<<endl;
                   }
                }
                if(current_instr.error_correction != 0)
                {
                    seen_error_correction ++;
                    continue_reading = 0;
                    cout << "error_correction has started," <<" error_correction: "<<current_instr.error_correction << endl;
                    fseek_manual(); // Updating the location of pointer in file. 
                }
                if(current_instr.clflush != 0) 
                { 
                    // Flushes while identifying the LLC occupancy.
                    if(LLC_OCC_CHK_PHASE == 1)
                    {  
                        /*if(INC_ARR_LEN > 0)
                        {*/
                            cout<<"received, in clflush, LLC_OCC_CHK_PHASE is 1"<<" LLC_OCC_CHK: FIRST_ARR_INC_ADDR: "<<FIRST_ARR_INC_ADDR<<" LAST_ARR_INC_ADDR: "<<LAST_ARR_INC_ADDR<<" REDUCED_ARR_INC_ADDR: "<<REDUCED_ARR_INC_ADDR<< endl;
                            current_instr.clflush = REDUCED_ARR_INC_ADDR; //ARR_MAX_LIMIT - (instr_flushed_beginning -1)*64 ;
                            REDUCED_ARR_INC_ADDR += 64;
                            INC_ARR_LEN --;
                            cout<<"received, in clflush after flush, LLC_OCC_CHK_PHASE is 1"<<" LLC_OCC_CHK: FIRST_ARR_INC_ADDR: "<<FIRST_ARR_INC_ADDR<<" LAST_ARR_INC_ADDR: "<<LAST_ARR_INC_ADDR<<" REDUCED_ARR_INC_ADDR: "<<REDUCED_ARR_INC_ADDR<<" INC_ARR_LEN: "<<INC_ARR_LEN<< endl;
                       /* }
                        else
                        { 
                            cout << "LLC_OCC_CHK_PHASE is 1" <<" LLC_OCC_CHK: REDUCED_ARR_SIZE: "<<REDUCED_ARR_SIZE<<" ARR_MAX_LIMIT: "<<ARR_MAX_LIMIT<< endl; 
                            current_instr.clflush = REDUCED_ARR_SIZE; //ARR_MAX_LIMIT - (instr_flushed_beginning -1)*64 ;
                            REDUCED_ARR_SIZE -= 64;
                        }*/
                    }
                    arch_instr.clflush = current_instr.clflush; // read clflush flag
                    int num = 6;
                    cout << "clflush is ON on address: "<< arch_instr.clflush << " instr_unique_id: "<<instr_unique_id<<" cycle: "<<current_core_cycle[cpu]<<" max_clflush_possible: "<< MAX_CLFLUSH_POSSIBLE<< endl;
                    for(num=0; num < MAX_CLFLUSH_POSSIBLE; num++)
                    {  
                        if(clflush_on_cpu[cpu][num] == 0)
                        {   
                            clflush_on_cpu[cpu][num] = current_instr.clflush; 
                            cout <<"cpu: "<<cpu<<" num: "<<num<< " after updation: "<<clflush_on_cpu[cpu][num]<<endl;
                            break;
                        }
                    } 
                    /*if( (num >= NUM_ACCESS_FOR_BIT1) && (NUM_CPUS > 1) && cpu == 1) // Designates check for sender core.
                    { 
                        int rounds = 0;

                        while(rounds <= num)
                        {    cout<<"address: "<<clflush_on_cpu[cpu][rounds]<<endl;
                             rounds ++;
                        }
                        cout<<"num is: "<<num <<endl; 
                        //assert(0);  //XXX We don't need this assert as long as the noise is triggered in the sender code, internally.
                    }*/
                    cpu_cycle_to_trigger_clflush[cpu][num] = current_core_cycle[cpu] + cycles_to_wait_for_before_triggering_clflush; //can these extra cycle be removed?
                }
                if(current_instr.double_probe_end == 1)
                { 
                    cout << "Inside double_probe end"<< " bit_revealed_is: "<<bit_revealed_is <<endl;
                    if(DOUBLE_PROBE == 1 && cpu != 0)
                        assert(0);
                    // DOUBLE_PROBE is used to decide whether to fill hole or not.
                    if(DOUBLE_PROBE == 1)
                    {
                        if(bit_revealed_is == 1)
                            cout <<" cycle: "<<current_core_cycle[cpu]<< " no increase in array length as miss received in double probe is: "<<bit_revealed_is<< endl;
                        else if(bit_revealed_is ==0 && DONT_INC_ARR == 0)
                        {
                            cout <<" cycle: "<<current_core_cycle[cpu]<< " increase array length, as miss received in double probe is: "<<bit_revealed_is<< endl;
                            continue_reading = 0;
                            FILL_HOLE = 1; // Two instructions are read
                            fseek_manual(); // Updating the location of pointer in file. 
                        }
                    }
                }
                if(current_instr.repeat_hole_fill == 1)
                {
                    cout << "Inside repeat_hole_fill." <<endl;
                    // Check to read extra added instructions.
                    /*if(NUM_CPUS > 1 && cpu == 0 && (INC_ARR_LEN > 0))
                    {
                        if(read_increased_arr_instr == 0 )
                        {
                            cout <<"Double probe: "<<(int)DOUBLE_PROBE<<endl;
                            assert(0);
                        }
                        else
                            read_increased_arr_instr = 0; //Reset this variable.       
                    }*/
                    //DOUBLE_PROBE is used to decide whether to increase the array length or not.
                    if(DOUBLE_PROBE == 1 && bit_revealed_is == 0 && DONT_INC_ARR == 0)
                    {
                        INC_ARR_LEN ++;
                        // Remembering the range for the incremented array addresses.
                        if(INC_ARR_LEN == 1)
                        {   FIRST_ARR_INC_ADDR = EXTRA_ARR_MAX_ADDR;  
                            LAST_ARR_INC_ADDR = EXTRA_ARR_MAX_ADDR;  
                            REDUCED_ARR_INC_ADDR = LAST_ARR_INC_ADDR;
                        }
                        else
                        {   LAST_ARR_INC_ADDR -= 64 ; //The addresses of array accessed for hole filling increase in descending order.
                            REDUCED_ARR_INC_ADDR = LAST_ARR_INC_ADDR;
                        }
                        cout<<" Received: Increased arr length."<<" INC_ARR_LEN: "<<INC_ARR_LEN<<" FIRST_ARR_INC_ADDR: "<<FIRST_ARR_INC_ADDR<<" LAST_ARR_INC_ADDR: "<<LAST_ARR_INC_ADDR<<endl;
                    }
                    DOUBLE_PROBE = 0;
                    DONT_INC_ARR = 0;

                }
                if(current_instr.wait == 1)
                {
                    #if !(WAIT_IMPLEMENTATION)
                        if(NUM_CPUS > 1 && (SETUP_DONE[0] != 1 ||SETUP_DONE[1] != 2))
                            SETUP_DONE[cpu]++;
                    #endif      
                    //XXX I feel this is needed for read_increased_arr_instr. 
                    /* // Check to read extra added instructions.
                    if(NUM_CPUS > 1 && cpu == 0 && (INC_ARR_LEN > 0))
                    {
                        if(read_increased_arr_instr == 0 )
                        {
                            cout <<"Double probe: "<<(int)DOUBLE_PROBE<<endl;
                            assert(0);    
                        }  
                        else
                            read_increased_arr_instr = 0; //Reset this variable.       
                    }
                    // DOUBLE_PROBE is used to decide whether to increase the array length or not.
                    if(DOUBLE_PROBE == 1 && bit_revealed_is == 0)
                    {
                        INC_ARR_LEN ++;
                        cout << " Received: Increased arr length."<<" INC_ARR_LEN: "<<INC_ARR_LEN <<endl;
                    } */
 
		    if(cpu == 0)
                        TURN = 1;
		    else
		        TURN = 0;

                    cpu_on_wait[cpu] = 1;

		    if(cpu == 0 && NUM_CPUS == 2)
	            {
			   if(DO_ERR_CORR_AT == 0)
				   assert(0);
			   bit_number_received ++;
		           HOLE_FILLING_BIT = 0; 
		           LLC_OCC_CHK_BIT = 0; 
			   if(DO_ERR_CORR_AT != 0 && ( (bit_number_received + 1) % DO_ERR_CORR_AT == 0 ) )
		           {
				  HOLE_FILLING_BIT = 1; 
			   }
			   else if( DO_ERR_CORR_AT != 0 && (bit_number_received % DO_ERR_CORR_AT == 0)  && (bit_number_received > 0 ) )
			   {
				  bit_number_received = -1;
				  LLC_OCC_CHK_BIT = 1; 
			   }
			   cout<<"bit_number_received is:"<<bit_number_received<<" HOLE_FILLING_BIT: "<<HOLE_FILLING_BIT<<" LLC_OCC_CHK_BIT: "<<LLC_OCC_CHK_BIT<< endl;
		    }
                    cout<<"SSSSSSSS cpu "<<cpu<<" is on wait, TURN: "<<TURN<<" current core cycle: "<<current_core_cycle[cpu]<<" instr_unique_id: "<<instr_unique_id;

                    continue_reading = 0;
                    read_increased_arr_instr = 0;
		    cout<< " num_retired:  "<<num_retired<< endl;

                    // Updating the variable "sender_seen_wait_once" for capturing sender active blocks range.
                    if(cpu == 1 && sender_seen_wait_once == 0)
                        sender_seen_wait_once = 1;
                    
                    if(cpu == 1) // Printing the active block IPs for sender.
                     {   cout << "SENDER_ACTIVE_IP_START: "<<hex<<SENDER_ACTIVE_IP_START<<" SENDER_ACTIVE_IP_END: "<< SENDER_ACTIVE_IP_END <<dec<< endl;
                         cout << "EVICTED_IP: "<<hex<<EVICTED_IP<<" EVICTED_CPU: "<<EVICTED_CPU<< " IS_EVICTED_BLOCK_INSTR: "<<IS_EVICTED_BLOCK_INSTR<<dec << endl;

                         // Incrementing stats for errors.
                         if(EVICTED_CPU == 1)
                         {
                             if(EVICTED_IP <= SENDER_ACTIVE_IP_START && EVICTED_IP <= SENDER_ACTIVE_IP_END)
                             {
                                 sender_evicted_own_active_block += 1;
                             }
                             else
                             { 
                                 sender_evicted_own_inactive_block += 1;
                             }
                         }
                     }
                }
                if(current_instr.fence == 1)
                {
                    if(cpu == 0) {
                    if(FIRST_FENCE == 0)
                        FIRST_FENCE = instr_unique_id;
                    else
                        Calculate_LLC_occupancy ++;  //This variable is printed only once i.e. when its value is 1.
                    }
                    if(LLC_OCC_CHK_PHASE == 1  && ( ( seen_rdtsc == 1 && seen_fence == 0 ) || ( seen_rdtsc == 2 && seen_fence == 1 ) ) ) 
                    {    
                        seen_fence ++;  
                        //cout << "seen_fence incremented to: "<<seen_fence<<endl;
                    }
                    fence_called_on_cpu[cpu] = 1;
                    fence_instr[cpu] = instr_unique_id;
		    if(DETAILED_PRINT)
		        cout<<"FFFFFFF cpu "<<cpu<<" encountered fence, current core cycle: "<<current_core_cycle[cpu]<<" instr_unique_id: "<<instr_unique_id<<endl;
                    continue_reading = 0;
                }
                if(current_instr.rdtsc == 1)
                {
                    if(LLC_OCC_CHK_PHASE == 1 ) 
                    {    
                        seen_rdtsc ++;  
                        //cout << "seen_rdtsc incremented to: "<<seen_rdtsc<<endl;
                    }
                    if(FIRST_TIME_CACHE_SETUP_DONE)
                    {
                       // The below condition is used in recording the error stats.
                        if(cpu == 0) 
                        {
                            if(rdtsc_timer_is_on)                
                                rdtsc_timer_is_on = 0;
                            else
                                rdtsc_timer_is_on = 1;
                        }
                       
                        if(prev_rdtsc_counter != 0)
                        {
				//HOLE_FILLING_BIT = 0, LLC_OCC_CHK_BIT
                            if( ( ( (current_core_cycle[cpu] - prev_rdtsc_counter) > HIT_MISS_THRESHOLD) && HOLE_FILLING_BIT == 0 && LLC_OCC_CHK_BIT == 0 //|| 
  //Assuming HIT_MISS_THRESHOLD for UNROLLING_FACTOR == 1 and HIT_MISS_THRESHOLD_LLC_OCC_CHK are the same.
                              /*(INC_ARR_LEN > 0 && read_increased_arr_instr != 1 && (current_core_cycle[cpu] - prev_rdtsc_counter) > HIT_MISS_THRESHOLD_LLC_OCC_CHK) */ ) || 
                              (LLC_OCC_CHK_BIT == 1 && (current_core_cycle[cpu] - prev_rdtsc_counter) > HIT_MISS_THRESHOLD_LLC_OCC_CHK) ||
                               (HOLE_FILLING_BIT == 1 && (current_core_cycle[cpu] - prev_rdtsc_counter) > HIT_MISS_THRESHOLD_LLC_OCC_CHK ) //Assumption: unrolling_factor for llc_occ_chk and error_correction is same.
                             )
                            { 
                                MISSES_OBSERVED++; // This stat is used in err_corr_algo triggering decision.
                               /* if(bit_revealed_is == 1)
                                { cout <<"RDTSC_subtraction: "<<(current_core_cycle[cpu] - prev_rdtsc_counter) <<" core_cycle value: "<<current_core_cycle[cpu]<< endl; 
                                      //assert(0); 
                                }*/  //This is to check, as per current implementation, there should be exactly one miss for the receiver.
                                bit_revealed_is = 1;
                       // Checking for multiple misses during LLC_OCC_CHK_PHASE  //XXX can be modified later to identify >1 misses and inc flush_more_address by that.
                                if(LLC_OCC_CHK_PHASE == 1 && instr_flushed_beginning == MAX_EXTRA_BLOCKS_POSSIBLE ) 
                                {    
                                    dont_fill_empty_slot = 1;
                                   // flush_more_address ++; // Not considering presence of multiple blocks in the current implementation.
                                    //cout << "Flush more address: "<<flush_more_address<<endl;
                                }
                            }
                            #if(IEEE_SEED)
                                if(differ_sig[0] != 0) 
                                {  
                                    for(int coding = 0; coding < REPETETION_CODING; coding++)
                                    {    if(repeat_code[coding] == 0)
                                        {
                                            repeat_code[coding] = (current_core_cycle[cpu] - prev_rdtsc_counter) - differ_sig[0];
                                            cout << "differ_sig, repeat_code[coding] "<< repeat_code[coding]<< " differ_sig[0] : "<<differ_sig[0]<<" current_core_cycle[cpu]: "<<current_core_cycle[cpu]<<" prev_rdtsc_counter: "<<prev_rdtsc_counter <<endl;
                                            break;
                                        } 
                                    }
                                    differ_sig[0] = 0;
                                   if(repeat_code[REPETETION_CODING -1] != 0)
                                   {   int total=0;
                                       for(int coding = 0; coding < REPETETION_CODING; coding++)
                                       {
                                            total += repeat_code[coding];
                                            repeat_code[coding] = 0;
                                       }
                                       cout << "differ_sig, after repetetion_coding, cycles spent are: "<<total/REPETETION_CODING <<endl;
                                   } 
                                }
                                else
                                {
                                    differ_sig[0] = current_core_cycle[cpu] - prev_rdtsc_counter;
                                    cout << "differ_sig[0] : "<<differ_sig[0]<<" current_core_cycle[cpu]: "<<current_core_cycle[cpu]<<" prev_rdtsc_counter: "<<prev_rdtsc_counter<<endl;
                                }
                            #endif
                            cout<<"RDTSC, sub: "<<(current_core_cycle[cpu] - prev_rdtsc_counter)<<" prev: "<<prev_rdtsc_counter<<" cycle: "<<current_core_cycle[cpu]<<" id: "<<prev_rdtsc_id<<" - "<<instr_unique_id<</*" repetetion_coding: "<<REPETETION_CODING<<" differential_signalling: "<<DIFFERENTIAL_SIGNALLING<<*/endl;
                            prev_rdtsc_counter = 0;  //reset rdtsc_counter
                            prev_rdtsc_id = 0;
                        }
                        else 
                        { prev_rdtsc_counter = current_core_cycle[cpu];
                          prev_rdtsc_id = instr_unique_id; 
                          //cout << "RDTSC start cycle: "<<current_core_cycle[cpu]<<endl; 
                        }
                    }
								    if(DETAILED_PRINT)  
								        cout << "rdtsc encountered cpu "<<cpu <<"current core cycle: "<<current_core_cycle[cpu] << " instr_unique_id: "<<instr_unique_id << endl;
                    if(recorded_rdtsc_value[cpu] != 0)
                    { 
                        #if(CLEAR_CACHE)
                            if(current_core_cycle[cpu]-recorded_rdtsc_value[cpu] < HIT_MISS_THRESHOLD_SINGLE_ARR_ACCESS)
                                hit_block ++;
								            cout << "rdtsc subtraction "<<current_core_cycle[cpu]-recorded_rdtsc_value[cpu] << endl;
                        #endif
                        recorded_rdtsc_value[cpu] = 0; 
                    }
                    else
                    {    recorded_rdtsc_value[cpu] = current_core_cycle[cpu]; }
                }
                arch_instr.instr_id = instr_unique_id;
		if(DETAILED_PRINT /*|| LLC_OCC_CHK_PHASE || CHANNEL_BW_ERR_PRINT*/)
		    cout<<"fptr: "<<fptr<<" IP is: "<<hex<< current_instr.ip<<dec<<" fence: "<<current_instr.fence<<" rdtsc: "<<current_instr.rdtsc<< " instr_id: "<<instr_unique_id<<" cpu: "<<cpu <<" cycle[cpu]: " <<current_core_cycle[cpu] <<" branch: " <<(int)current_instr.is_branch <<" taken: "<<(int)current_instr.branch_taken;
                arch_instr.ip = current_instr.ip;
                arch_instr.is_branch = current_instr.is_branch;
                arch_instr.branch_taken = current_instr.branch_taken;
                //cout <<"ip: "<< arch_instr.ip<<" wait: "<<arch_instr.wait<<" warmup: "<<arch_instr.warmup<<" instr_id: "<<arch_instr.instr_id<<" is_branch: "<<arch_instr.is_branch<<" branch_taken: "<<arch_instr.branch_taken << endl;
                arch_instr.asid[0] = cpu;
                arch_instr.asid[1] = cpu;

                for (uint32_t i=0; i<MAX_INSTR_DESTINATIONS; i++) {
                    arch_instr.destination_registers[i] = current_instr.destination_registers[i];
                    arch_instr.destination_memory[i] = current_instr.destination_memory[i];
                    arch_instr.destination_virtual_address[i] = current_instr.destination_memory[i];

                    if (arch_instr.destination_registers[i])
                    {
		 	                if(DETAILED_PRINT /*|| LLC_OCC_CHK_PHASE *//*|| CHANNEL_BW_ERR_PRINT*/)
                  		  cout <<" i: "<<i<<" dest_rgst: "<<(int) arch_instr.destination_registers[i];
				              num_reg_ops++;
		                }

                    if (arch_instr.destination_memory[i]) {
                        num_mem_ops++;

                        if(DETAILED_PRINT /*|| LLC_OCC_CHK_PHASE *//*|| CHANNEL_BW_ERR_PRINT*/)
                            cout <<" dest_mem: "<<hex<< arch_instr.destination_memory[i]<<dec;

                        // update STA, this structure is required to execute store instructios properly without deadlock
                        if (num_mem_ops > 0) {
#ifdef SANITY_CHECK
                            if (STA[STA_tail] < UINT64_MAX) {
                                if (STA_head != STA_tail)
                                    assert(0);
                            }
#endif
                            STA[STA_tail] = instr_unique_id;
                            STA_tail++;

                            if (STA_tail == STA_SIZE)
                                STA_tail = 0;
                        }
                    }
                }

                int skip = 0;

                for (int i=0; i<NUM_INSTR_SOURCES; i++) 
                {
                    arch_instr.source_registers[i] = current_instr.source_registers[i];
                    arch_instr.source_memory[i] = current_instr.source_memory[i];
                    arch_instr.source_virtual_address[i] = current_instr.source_memory[i];
									
                    if (arch_instr.source_registers[i])
                       { 
                           num_reg_ops++;
			                     if(DETAILED_PRINT /*|| LLC_OCC_CHK_PHASE */ /*|| CHANNEL_BW_ERR_PRINT*/)
                    		      cout <<" i: "<<i<<" src_rgst: "<<(int) arch_instr.source_registers[i];
                       }
                    if (arch_instr.source_memory[i])
                       {
			       	// Flushes while identifying the LLC occupancy.
				       if(access_to_fill_empty_slots > 0)
				      {  
                                    REDUCED_ARR_INC_ADDR -= 64;
                                    LAST_ARR_INC_ADDR = REDUCED_ARR_INC_ADDR;
                                    INC_ARR_LEN ++;
                                    arch_instr.source_memory[i] = REDUCED_ARR_INC_ADDR; 
                                    access_to_fill_empty_slots --;
                                    if(access_to_fill_empty_slots == 0)
                                        cout<<" received, final Reduced_arr_size is: "<<REDUCED_ARR_SIZE<<" ARR_MAX_LIMIT: "<<ARR_MAX_LIMIT<<" REDUCED_ARR_INC_ADDR: "<<REDUCED_ARR_INC_ADDR<<endl;
				      }
				          if(DETAILED_PRINT /*|| LLC_OCC_CHK_PHASE *//*|| CHANNEL_BW_ERR_PRINT*/) 
				                  cout<<hex<<" src_mem: "<< arch_instr.source_memory[i]<<dec;
				          num_mem_ops++;
                            //Regulating probing while LLC_OCC_CHK.
                            if(LLC_OCC_CHK_PHASE == 1 && turn_to_traverse_arr > 0)
                            {
                                if(instr_flushed_beginning != MAX_EXTRA_BLOCKS_POSSIBLE )
                                {
                                    arch_instr.source_memory[i] = traversed_incremented_element;
                                    cout << " edited source_memory to incremented array address: "<< traversed_incremented_element;
                                    turn_to_traverse_arr --;
                                    //traversed_incremented_element += 64;
                                }
                                else
                                {
                                    arch_instr.source_memory[i] = traversed_element;
                                    //cout <<"turn_to_traverse_arr:" <<turn_to_traverse_arr<< " edited source_memory to: "<< traversed_element;
                                    turn_to_traverse_arr --;
                                    traversed_element -= 64;
                                    if(traversed_element < ARR_START_ADDR)
				    {
					still_have_array_elements_to_traverse = 0;
                                        //cout<<endl<<"turn_to_traverse_arr:" <<turn_to_traverse_arr<<" still_have_array_elements_to_traverse: "<<still_have_array_elements_to_traverse<<endl;
				    }
                                }
                            }
                       }
                }
                if(DETAILED_PRINT /*|| LLC_OCC_CHK_PHASE *//*|| CHANNEL_BW_ERR_PRINT*/)
			               cout << endl;

                if(skip == 1)
                {
                    cout << "Instruction skipped."<<endl;
                    skip = 0;
                    continue;
                }

                //Next instruction should fill-up the empty slots created in LLC during LLC_OCC_CHK_PHASE.
                
                /*if(current_instr.llc_occ_chk_end == 1) // @Yashika, to be tested.
                {
                    // This check ensures access_to_fill_empty_slots is not set on 1st acces to llc_occ_chk_end.
                    if( REDUCED_ARR_INC_ADDR != LAST_ARR_INC_ADDR && dont_fill_empty_slot == 0 )
                        access_to_fill_empty_slots = MAX_EXTRA_BLOCKS_POSSIBLE;  
                   
                    cout <<"received access_to_fill_empty_slots: " << access_to_fill_empty_slots <<" LLC_OCC_CHK: REDUCED_ARR_SIZE: "<<REDUCED_ARR_SIZE<<" ARR_MAX_LIMIT: "<<ARR_MAX_LIMIT<<" cycle[cpu]: " <<current_core_cycle[cpu] << endl; 
                }*/
                arch_instr.num_reg_ops = num_reg_ops;
                arch_instr.num_mem_ops = num_mem_ops;
                if (num_mem_ops > 0) 
                    arch_instr.is_memory = 1;

                // virtually add this instruction to the ROB
                if (ROB.occupancy < ROB.SIZE) {
                    uint32_t rob_index = add_to_rob(&arch_instr);
                    num_reads++;

                    // branch prediction
                    if (arch_instr.is_branch) {

                        DP( if (warmup_complete[cpu]) {
                        cout << "[BRANCH] instr_id: " << instr_unique_id << " ip: " << hex << arch_instr.ip << dec << " taken: " << +arch_instr.branch_taken << endl; });

                        num_branch++;

                        /*
                        uint8_t branch_prediction;
                        // for faster simulation, force perfect prediction during the warmup
                        // note that branch predictor is still learning with real branch results
                        if (all_warmup_complete == 0)
                            branch_prediction = arch_instr.branch_taken; 
                        else
                            branch_prediction = predict_branch(arch_instr.ip);
                        */
                        uint8_t branch_prediction = predict_branch(arch_instr.ip);
                        
                        if (arch_instr.branch_taken != branch_prediction) {
                            branch_mispredictions++;

                            DP( if (warmup_complete[cpu]) {
                            cout << "[BRANCH] MISPREDICTED instr_id: " << instr_unique_id << " ip: " << hex << arch_instr.ip << dec;
                            cout << " taken: " << +arch_instr.branch_taken << " predicted: " << +branch_prediction << endl; });

                            // halt any further fetch this cycle
                            instrs_to_read_this_cycle = 0;

                            // and stall any additional fetches until the branch is executed
                            fetch_stall = 1; 

                            ROB.entry[rob_index].branch_mispredicted = 1;
                        }
                        else {
                            if (branch_prediction == 1) {
                                // if we are accurately predicting a branch to be taken, then we can't possibly fetch down that path this cycle,
                                // so we have to wait until the next cycle to fetch those
                                instrs_to_read_this_cycle = 0;
                            }

                            DP( if (warmup_complete[cpu]) {
                            cout << "[BRANCH] PREDICTED    instr_id: " << instr_unique_id << " ip: " << hex << arch_instr.ip << dec;
                            cout << " taken: " << +arch_instr.branch_taken << " predicted: " << +branch_prediction << endl; });
                        }

                        last_branch_result(arch_instr.ip, arch_instr.branch_taken);
                    }

                    //if ((num_reads == FETCH_WIDTH) || (ROB.occupancy == ROB.SIZE))
                    if ((num_reads >= instrs_to_read_this_cycle) || (ROB.occupancy == ROB.SIZE))
                        continue_reading = 0;
                }
                instr_unique_id++;
            }
        }
    }


    //instrs_to_fetch_this_cycle = num_reads;
}

uint32_t O3_CPU::add_to_rob(ooo_model_instr *arch_instr)
{
    uint32_t index = ROB.tail;

    // sanity check
    if (ROB.entry[index].instr_id != 0) {
        cerr << "[ROB_ERROR] " << __func__ << " is not empty index: " << index;
        cerr << " instr_id: " << ROB.entry[index].instr_id << endl;
        assert(0);
    }

    ROB.entry[index] = *arch_instr;
    ROB.entry[index].event_cycle = current_core_cycle[cpu];
    ROB.entry[index].retire_clflush = current_core_cycle[cpu];

    ROB.occupancy++;
    ROB.tail++;
    if (ROB.tail >= ROB.SIZE)
        ROB.tail = 0;

    DP ( if (warmup_complete[cpu]) {
    cout << "[ROB] " <<  __func__ << " instr_id: " << ROB.entry[index].instr_id;
    cout << " ip: " << hex << ROB.entry[index].ip << dec;
    cout << " head: " << ROB.head << " tail: " << ROB.tail << " occupancy: " << ROB.occupancy;
    cout << " event: " << ROB.entry[index].event_cycle << " current: " << current_core_cycle[cpu] << endl; });

#ifdef SANITY_CHECK
    if (ROB.entry[index].ip == 0) {
        cerr << "[ROB_ERROR] " << __func__ << " ip is zero index: " << index;
        cerr << " instr_id: " << ROB.entry[index].instr_id << " ip: " << ROB.entry[index].ip << endl;
        assert(0);
    }
#endif

    return index;
}

uint32_t O3_CPU::check_rob(uint64_t instr_id)
{
    if ((ROB.head == ROB.tail) && ROB.occupancy == 0)
        return ROB.SIZE;

    if (ROB.head < ROB.tail) {
        for (uint32_t i=ROB.head; i<ROB.tail; i++) {
            if (ROB.entry[i].instr_id == instr_id) {
                DP ( if (warmup_complete[ROB.cpu]) {
                cout << "[ROB] " << __func__ << " same instr_id: " << ROB.entry[i].instr_id;
                cout << " rob_index: " << i << endl; });
                return i;
            }
        }
    }
    else {
        for (uint32_t i=ROB.head; i<ROB.SIZE; i++) {
            if (ROB.entry[i].instr_id == instr_id) {
                DP ( if (warmup_complete[cpu]) {
                cout << "[ROB] " << __func__ << " same instr_id: " << ROB.entry[i].instr_id;
                cout << " rob_index: " << i << endl; });
                return i;
            }
        }
        for (uint32_t i=0; i<ROB.tail; i++) {
            if (ROB.entry[i].instr_id == instr_id) {
                DP ( if (warmup_complete[cpu]) {
                cout << "[ROB] " << __func__ << " same instr_id: " << ROB.entry[i].instr_id;
                cout << " rob_index: " << i << endl; });
                return i;
            }
        }
    }

    cerr << "[ROB_ERROR] " << __func__ << " does not have any matching index! ";
    cerr << " instr_id: " << instr_id << endl;
    assert(0);

    return ROB.SIZE;
}

void O3_CPU::fetch_instruction()
{
    // TODO: can we model wrong path execusion?

    // add this request to ITLB
    uint32_t read_index = (ROB.last_read == (ROB.SIZE-1)) ? 0 : (ROB.last_read + 1);
    for (uint32_t i=0; i<FETCH_WIDTH; i++) {

        if (ROB.entry[read_index].ip == 0)
            break;

#ifdef SANITY_CHECK
        // sanity check
        if (ROB.entry[read_index].translated) {
            if (read_index == ROB.head)
                break;
            else {
                cout << "read_index: " << read_index << " ROB.head: " << ROB.head << " ROB.tail: " << ROB.tail << endl;
                assert(0);
            }
        }
#endif

        PACKET trace_packet;
        trace_packet.instruction = 1;
        trace_packet.tlb_access = 1;
        trace_packet.fill_level = FILL_L1;
        trace_packet.cpu = cpu;
        trace_packet.address = ROB.entry[read_index].ip >> LOG2_PAGE_SIZE;
        if (knob_cloudsuite)
            trace_packet.address = ((ROB.entry[read_index].ip >> LOG2_PAGE_SIZE) << 9) | ( 256 + ROB.entry[read_index].asid[0]);
        else
            trace_packet.address = ROB.entry[read_index].ip >> LOG2_PAGE_SIZE;
        trace_packet.full_addr = ROB.entry[read_index].ip;
        trace_packet.instr_id = ROB.entry[read_index].instr_id;
        trace_packet.rob_index = read_index;
        trace_packet.producer = 0; // TODO: check if this guy gets used or not
        trace_packet.ip = ROB.entry[read_index].ip;
        trace_packet.type = LOAD; 
        trace_packet.asid[0] = ROB.entry[read_index].asid[0];
        trace_packet.asid[1] = ROB.entry[read_index].asid[1];
        trace_packet.event_cycle = current_core_cycle[cpu];

			  if(DETAILED_PRINT || (ROB.entry[read_index].instr_id >= range_start && ROB.entry[read_index].instr_id <= range_end))
            cout <<ROB.entry[read_index].ip << " I entered inside ITLB rq at cycle: " << current_core_cycle[0] <<" ROB.entry[read_index].instr_id: " <<ROB.entry[read_index].instr_id <<endl;

        int rq_index = ITLB.add_rq(&trace_packet);

        if (rq_index == -2)
            break;
        else {
            /*
            if (rq_index >= 0) {
                uint32_t producer = ITLB.RQ.entry[rq_index].rob_index;
                ROB.entry[read_index].fetch_producer = producer;
                ROB.entry[read_index].is_consumer = 1;

                ROB.entry[producer].memory_instrs_depend_on_me[read_index] = 1;
                ROB.entry[producer].is_producer = 1; // producer for fetch
            }
            */

            ROB.last_read = read_index;
            read_index++;
            if (read_index == ROB.SIZE)
                read_index = 0;
        }
    }
    
    uint32_t fetch_index = (ROB.last_fetch == (ROB.SIZE-1)) ? 0 : (ROB.last_fetch + 1);


    for (uint32_t i=0; i<FETCH_WIDTH; i++) {

        // fetch is in-order so it should break
        if ((ROB.entry[fetch_index].translated != COMPLETED) || (ROB.entry[fetch_index].event_cycle > current_core_cycle[cpu])) 
            {
                //cout<<"Translation is not complete yet"<<" current_core_cycle: "<< current_core_cycle[0] << endl;
             break;
            }
        
        if(DETAILED_PRINT || (ROB.entry[fetch_index].instr_id >= range_start && ROB.entry[fetch_index].instr_id <= range_end))
            cout << "Inside fetch_instr: "<<"instr_id: "<<ROB.entry[fetch_index].instr_id <<" translated: "<<(int)ROB.entry[fetch_index].translated <<" current_core_cycle: "<< current_core_cycle[0] <<" event_cycle: "<< ROB.entry[fetch_index].event_cycle<< endl;

        // sanity check
        if (ROB.entry[fetch_index].fetched) {
            if (fetch_index == ROB.head)
                break;
            else {
                cout << "fetch_index: " << fetch_index << " ROB.head: " << ROB.head << " ROB.tail: " << ROB.tail << endl;
                assert(0);
            }
        }

        // add it to L1I
        PACKET fetch_packet;
        fetch_packet.instruction = 1;
        fetch_packet.fill_level = FILL_L1;
        fetch_packet.cpu = cpu;
        fetch_packet.address = ROB.entry[fetch_index].instruction_pa >> 6;
        fetch_packet.instruction_pa = ROB.entry[fetch_index].instruction_pa;
        fetch_packet.full_addr = ROB.entry[fetch_index].instruction_pa;
        fetch_packet.instr_id = ROB.entry[fetch_index].instr_id;
        fetch_packet.rob_index = fetch_index;
        fetch_packet.producer = 0;
        fetch_packet.ip = ROB.entry[fetch_index].ip;
        fetch_packet.type = LOAD; 
        fetch_packet.asid[0] = ROB.entry[fetch_index].asid[0];
        fetch_packet.asid[1] = ROB.entry[fetch_index].asid[1];
        fetch_packet.event_cycle = current_core_cycle[cpu];


        int rq_index = L1I.add_rq(&fetch_packet);

        if (rq_index == -2)
            break;
        else {
            /*
            if (rq_index >= 0) {
                uint32_t producer = L1I.RQ.entry[rq_index].rob_index;
                ROB.entry[fetch_index].fetch_producer = producer;
                ROB.entry[fetch_index].is_consumer = 1;

                ROB.entry[producer].memory_instrs_depend_on_me[fetch_index] = 1;
                ROB.entry[producer].is_producer = 1;
            }
            */
						if(DETAILED_PRINT || (ROB.entry[fetch_index].instr_id >= range_start && ROB.entry[fetch_index].instr_id <= range_end))
            cout << ROB.entry[fetch_index].ip<< " I entered inside L1I rq at cycle: " << current_core_cycle[0] <<" ROB.entry[fetch_index].instr_id: " <<ROB.entry[fetch_index].instr_id <<endl;
					
            ROB.entry[fetch_index].fetched = INFLIGHT;
            ROB.last_fetch = fetch_index;
            fetch_index++;
            if (fetch_index == ROB.SIZE)
                fetch_index = 0;
        }
    }
}

// TODO: When should we update ROB.schedule_event_cycle?
// I. Instruction is fetched
// II. Instruction is completed
// III. Instruction is retired
void O3_CPU::schedule_instruction()
{
    if ((ROB.head == ROB.tail) && ROB.occupancy == 0)
        return;
   
    // execution is out-of-order but we have an in-order scheduling algorithm to detect all RAW dependencies
    uint32_t limit = ROB.tail;//ROB.next_fetch[1];
    num_searched = 0;
    if (ROB.head < limit) { //Q. is this condition ever true ?
        for (uint32_t i=ROB.head; i<limit; i++) { 
            if ((ROB.entry[i].fetched != COMPLETED) || (ROB.entry[i].event_cycle > current_core_cycle[cpu]) || (num_searched >= SCHEDULER_SIZE))
                {
										//if(DETAILED_PRINT)
                    //cout <<"Inside sched_inst return here I am "<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " ROB.entry[i].ip:  " << ROB.entry[i].ip<<endl;

                   return;
                }

            if (ROB.entry[i].scheduled == 0)
                {    
										if(DETAILED_PRINT || (ROB.entry[i].instr_id >= range_start && ROB.entry[i].instr_id <= range_end))
                     cout <<"Inside schedule_inst do_sched here I am"<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " ROB.entry[i].ip:  " << ROB.entry[i].ip<<endl;

                  do_scheduling(i);
                }

            num_searched++;
        }
    }
    else { 
        for (uint32_t i=ROB.head; i<ROB.SIZE; i++) {  
            if ((ROB.entry[i].fetched != COMPLETED) || (ROB.entry[i].event_cycle > current_core_cycle[cpu]) || (num_searched >= SCHEDULER_SIZE))
                {
										//if(DETAILED_PRINT)
                    // cout <<"Inside schedule_inst return "<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " ROB.entry[i].ip:  " << ROB.entry[i].ip<<endl;

                   return; 
                }

            if (ROB.entry[i].scheduled == 0)
                { 
									if(DETAILED_PRINT || (ROB.entry[i].instr_id >= range_start && ROB.entry[i].instr_id <= range_start))
                 cout<<"Inside schedule_inst do_sched, I am printed "<<" current_core_cycle[0]: "<<current_core_cycle[0]<<" ROB.entry[i].ip:  "<< ROB.entry[i].ip<<endl;

                  do_scheduling(i);
                }

            num_searched++;
        }
        for (uint32_t i=0; i<limit; i++) { 
            if ((ROB.entry[i].fetched != COMPLETED) || (ROB.entry[i].event_cycle > current_core_cycle[cpu]) || (num_searched >= SCHEDULER_SIZE))
                {
									//if(DETAILED_PRINT)
                     //cout <<"Inside schedule_inst return "<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " ROB.entry[i].ip:  " << ROB.entry[i].ip<<endl;
                 return;
                }

            if (ROB.entry[i].scheduled == 0)
                {   
										if(DETAILED_PRINT || (ROB.entry[i].instr_id >= range_start && ROB.entry[i].instr_id <= range_end))
                     cout <<"Inside schedule_inst do_sched "<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " ROB.entry[i].ip:  " << ROB.entry[i].ip<<endl;

									do_scheduling(i);
                }

            num_searched++;
        }
    }
}

void O3_CPU::do_scheduling(uint32_t rob_index)
{
    ROB.entry[rob_index].reg_ready = 1; // reg_ready will be reset to 0 if there is RAW dependency 

    reg_dependency(rob_index);
    ROB.next_schedule = (rob_index == (ROB.SIZE - 1)) ? 0 : (rob_index + 1);

    if (ROB.entry[rob_index].is_memory)
        {
					if(DETAILED_PRINT || (ROB.entry[rob_index].instr_id >= range_start && ROB.entry[rob_index].instr_id <= range_end))
            cout<< ROB.entry[rob_index].ip<<" I am a memory instr. entered inside do_scheduling at cycle: " << current_core_cycle[0] <<"  my old event cycle is: " <<ROB.entry[rob_index].event_cycle<<" scheduled is inflight." <<endl;

					ROB.entry[rob_index].scheduled = INFLIGHT;
				}
    else {
        ROB.entry[rob_index].scheduled = COMPLETED; // Scheduled status for non-memory instructions is completed straight away.
 				
				if(DETAILED_PRINT || (ROB.entry[rob_index].instr_id >= range_start && ROB.entry[rob_index].instr_id <= range_end))
				cout<< ROB.entry[rob_index].ip<<" I entered inside do_scheduling at cycle: " << current_core_cycle[0] <<"  my old event cycle is: " <<ROB.entry[rob_index].event_cycle <<" scheduled is completed." <<endl;

        // ADD LATENCY
        if (ROB.entry[rob_index].event_cycle < current_core_cycle[cpu])
            ROB.entry[rob_index].event_cycle = current_core_cycle[cpu] + SCHEDULING_LATENCY;
        else
            ROB.entry[rob_index].event_cycle += SCHEDULING_LATENCY;

				if(DETAILED_PRINT || (ROB.entry[rob_index].instr_id >= range_start && ROB.entry[rob_index].instr_id <= range_end))
        cout<<ROB.entry[rob_index].ip<<" inside do_scheduling at cycle: " << current_core_cycle[0] <<"  new event cycle of entry in ROB is: " << ROB.entry[rob_index].event_cycle<< endl;

        if (ROB.entry[rob_index].reg_ready) {

#ifdef SANITY_CHECK
            if (RTE1[RTE1_tail] < ROB_SIZE)
                assert(0);
#endif
            // remember this rob_index in the Ready-To-Execute array 1
            RTE1[RTE1_tail] = rob_index;

						if(DETAILED_PRINT || (ROB.entry[rob_index].instr_id >= range_start && ROB.entry[rob_index].instr_id <= range_end))
						 cout<<ROB.entry[rob_index].ip<<" I am a non-memory instruction and is register ready at cycle: " << current_core_cycle[0] <<endl;

            DP (if (warmup_complete[cpu]) {
            cout << "[RTE1] " << __func__ << " instr_id: " << ROB.entry[rob_index].instr_id << " rob_index: " << rob_index << " is added to RTE1";
            cout << " head: " << RTE1_head << " tail: " << RTE1_tail << endl; }); 

            RTE1_tail++;
            if (RTE1_tail == ROB_SIZE)
                RTE1_tail = 0;
        }
    }
}

void O3_CPU::reg_dependency(uint32_t rob_index)
{
    // print out source/destination registers
    DP (if (warmup_complete[cpu]) {
    for (uint32_t i=0; i<NUM_INSTR_SOURCES; i++) {
        if (ROB.entry[rob_index].source_registers[i]) {
            cout << "[ROB] " << __func__ << " instr_id: " << ROB.entry[rob_index].instr_id << " is_memory: " << +ROB.entry[rob_index].is_memory;
            cout << " load  reg_index: " << +ROB.entry[rob_index].source_registers[i] << endl;
        }
    }
    for (uint32_t i=0; i<MAX_INSTR_DESTINATIONS; i++) {
        if (ROB.entry[rob_index].destination_registers[i]) {
            cout << "[ROB] " << __func__ << " instr_id: " << ROB.entry[rob_index].instr_id << " is_memory: " << +ROB.entry[rob_index].is_memory;
            cout << " store reg_index: " << +ROB.entry[rob_index].destination_registers[i] << endl;
        }
    } }); 

    // check RAW dependency
    int prior = rob_index - 1;
    if (prior < 0)
        prior = ROB.SIZE - 1;

    if (rob_index != ROB.head) {
        if ((int)ROB.head <= prior) {
            for (int i=prior; i>=(int)ROB.head; i--) if (ROB.entry[i].executed != COMPLETED) {
		for (uint32_t j=0; j<NUM_INSTR_SOURCES; j++) {
			if (ROB.entry[rob_index].source_registers[j] && (ROB.entry[rob_index].reg_RAW_checked[j] == 0))
				reg_RAW_dependency(i, rob_index, j);  
		}
	    }
        } else {
            for (int i=prior; i>=0; i--) if (ROB.entry[i].executed != COMPLETED) {
		for (uint32_t j=0; j<NUM_INSTR_SOURCES; j++) {
			if (ROB.entry[rob_index].source_registers[j] && (ROB.entry[rob_index].reg_RAW_checked[j] == 0))
				reg_RAW_dependency(i, rob_index, j);
		}
	    }
            for (int i=ROB.SIZE-1; i>=(int)ROB.head; i--) if (ROB.entry[i].executed != COMPLETED) {
		for (uint32_t j=0; j<NUM_INSTR_SOURCES; j++) {
			if (ROB.entry[rob_index].source_registers[j] && (ROB.entry[rob_index].reg_RAW_checked[j] == 0))
				reg_RAW_dependency(i, rob_index, j);
		}
	    }
        }
    }
}

void O3_CPU::reg_RAW_dependency(uint32_t prior, uint32_t current, uint32_t source_index)
{
    for (uint32_t i=0; i<MAX_INSTR_DESTINATIONS; i++) {
        if (ROB.entry[prior].destination_registers[i] == 0)
            continue;

        if (ROB.entry[prior].destination_registers[i] == ROB.entry[current].source_registers[source_index]) {
								if(DETAILED_PRINT || (ROB.entry[current].instr_id >= range_start && ROB.entry[current].instr_id <= range_end))
                {cout <<" Inside reg_RAW_dep "<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " ROB.entry[current].ip:  " << ROB.entry[current].ip << " ROB.entry[current].source_registers[source_index]: "<<(unsigned) ROB.entry[current].source_registers[source_index] <<endl; 
                cout <<" dependent on "<< " ROB.entry[prior].ip:  " << ROB.entry[prior].ip<<" ROB.entry[prior].destination_registers[i]: "<< (unsigned) ROB.entry[prior].destination_registers[i] <<endl; }
           
            // we need to mark this dependency in the ROB since the producer might not be added in the store queue yet
            ROB.entry[prior].registers_instrs_depend_on_me.insert (current);   // this load cannot be executed until the prior store gets executed
            ROB.entry[prior].registers_index_depend_on_me[source_index].insert (current);   // this load cannot be executed until the prior store gets executed
            ROB.entry[prior].reg_RAW_producer = 1;

            ROB.entry[current].reg_ready = 0;
            ROB.entry[current].producer_id = ROB.entry[prior].instr_id; 
            ROB.entry[current].num_reg_dependent++;
            ROB.entry[current].reg_RAW_checked[source_index] = 1;

            DP (if(warmup_complete[cpu]) {
            cout << "[ROB] " << __func__ << " instr_id: " << ROB.entry[current].instr_id << " is_memory: " << +ROB.entry[current].is_memory;
            cout << " RAW reg_index: " << +ROB.entry[current].source_registers[source_index];
            cout << " producer_id: " << ROB.entry[prior].instr_id << endl; });

            return;
        }
    }
}

void O3_CPU::execute_instruction()
{
    if ((ROB.head == ROB.tail) && ROB.occupancy == 0)
        return;

    // out-of-order execution for non-memory instructions
    // memory instructions are handled by memory_instruction()
    uint32_t exec_issued = 0, num_iteration = 0;

    while (exec_issued < EXEC_WIDTH) {
        if (RTE0[RTE0_head] < ROB_SIZE) {
            uint32_t exec_index = RTE0[RTE0_head];
            if (ROB.entry[exec_index].event_cycle <= current_core_cycle[cpu]) {

								if(DETAILED_PRINT || (ROB.entry[exec_index].instr_id >= range_start && ROB.entry[exec_index].instr_id <= range_end))
                cout<<ROB.entry[exec_index].ip<<" Inside execute_instruction for RTE0 : "<< " current_core_cycle[0] " << current_core_cycle[0] << endl;
								

                do_execution(exec_index);

                RTE0[RTE0_head] = ROB_SIZE;
                RTE0_head++;
                if (RTE0_head == ROB_SIZE)
                    RTE0_head = 0;
                exec_issued++;

            }
        }
        else {
            //DP (if (warmup_complete[cpu]) {
            //cout << "[RTE0] is empty head: " << RTE0_head << " tail: " << RTE0_tail << endl; });
            break;
        }


        num_iteration++;
        if (num_iteration == (ROB_SIZE-1))
            break;
    }

    num_iteration = 0;
    while (exec_issued < EXEC_WIDTH) {
        if (RTE1[RTE1_head] < ROB_SIZE) {
            uint32_t exec_index = RTE1[RTE1_head];
            if (ROB.entry[exec_index].event_cycle <= current_core_cycle[cpu]) {
                do_execution(exec_index);

								if(DETAILED_PRINT || (ROB.entry[exec_index].instr_id >= range_start && ROB.entry[exec_index].instr_id <= range_end))
                cout<<ROB.entry[exec_index].ip<<"inside execute_instruction for RTE1 : " << " current_core_cycle[0] " << current_core_cycle[0] << endl;

                RTE1[RTE1_head] = ROB_SIZE;
                RTE1_head++;
                if (RTE1_head == ROB_SIZE)
                    RTE1_head = 0;
                exec_issued++;

		if(DETAILED_PRINT || (ROB.entry[exec_index].instr_id >= range_start && ROB.entry[exec_index].instr_id <= range_end))
      cout <<" Inside execute_instruction "<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " ROB.entry[exec_index].ip:  " << ROB.entry[exec_index].ip<<endl;		
						
            }
        }
        else {
            //DP (if (warmup_complete[cpu]) {
            //cout << "[RTE1] is empty head: " << RTE1_head << " tail: " << RTE1_tail << endl; });
            break;
        }

        num_iteration++;
        if (num_iteration == (ROB_SIZE-1))
            break;
    }
}

void O3_CPU::do_execution(uint32_t rob_index)
{
    //if (ROB.entry[rob_index].reg_ready && (ROB.entry[rob_index].scheduled == COMPLETED) && (ROB.entry[rob_index].event_cycle <= current_core_cycle[cpu])) {

        ROB.entry[rob_index].executed = INFLIGHT;

						if(DETAILED_PRINT || (ROB.entry[rob_index].instr_id >= range_start && ROB.entry[rob_index].instr_id <= range_end))
            cout <<"Inside do_execution, execution is inflight "<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " ROB.entry[rob_index].ip:  " << ROB.entry[rob_index].ip<<endl;

        // ADD LATENCY
        if (ROB.entry[rob_index].event_cycle < current_core_cycle[cpu])
            ROB.entry[rob_index].event_cycle = current_core_cycle[cpu] + EXEC_LATENCY;
        else
            ROB.entry[rob_index].event_cycle += EXEC_LATENCY;

        inflight_reg_executions++;

				if(DETAILED_PRINT || (ROB.entry[rob_index].instr_id >= range_start && ROB.entry[rob_index].instr_id <= range_end))
              cout <<"Inside do_execution "<<" event cycle changed to: " << ROB.entry[rob_index].event_cycle<< " for ip: "<<ROB.entry[rob_index].ip<<endl;


        DP (if (warmup_complete[cpu]) {
        cout << "[ROB] " << __func__ << " non-memory instr_id: " << ROB.entry[rob_index].instr_id; 
        cout << " event_cycle: " << ROB.entry[rob_index].event_cycle << endl;});
    //}
}

void O3_CPU::schedule_memory_instruction()
{
    if ((ROB.head == ROB.tail) && ROB.occupancy == 0)
        return;

    // execution is out-of-order but we have an in-order scheduling algorithm to detect all RAW dependencies
    uint32_t limit = ROB.next_schedule;
    num_searched = 0;
    if (ROB.head < limit) {
        for (uint32_t i=ROB.head; i<limit; i++) {

            if (ROB.entry[i].is_memory == 0)
                continue;

            if ((ROB.entry[i].fetched != COMPLETED) || (ROB.entry[i].event_cycle > current_core_cycle[cpu]) || (num_searched >= SCHEDULER_SIZE))
                break;

            if (ROB.entry[i].is_memory && ROB.entry[i].reg_ready && (ROB.entry[i].scheduled == INFLIGHT))
                do_memory_scheduling(i);
        }
    }
    else {
        for (uint32_t i=ROB.head; i<ROB.SIZE; i++) {

            if (ROB.entry[i].is_memory == 0)
                continue;

            if ((ROB.entry[i].fetched != COMPLETED) || (ROB.entry[i].event_cycle > current_core_cycle[cpu]) || (num_searched >= SCHEDULER_SIZE))
                break;

            if (ROB.entry[i].is_memory && ROB.entry[i].reg_ready && (ROB.entry[i].scheduled == INFLIGHT))
                do_memory_scheduling(i);
        }
        for (uint32_t i=0; i<limit; i++) {

            if (ROB.entry[i].is_memory == 0)
                continue;

            if ((ROB.entry[i].fetched != COMPLETED) || (ROB.entry[i].event_cycle > current_core_cycle[cpu]) || (num_searched >= SCHEDULER_SIZE))
                break;

            if (ROB.entry[i].is_memory && ROB.entry[i].reg_ready && (ROB.entry[i].scheduled == INFLIGHT))
                do_memory_scheduling(i);
        }
    }
}

void O3_CPU::execute_memory_instruction()
{
    operate_lsq();
    operate_cache();
}

void O3_CPU::do_memory_scheduling(uint32_t rob_index)
{
    uint32_t not_available = check_and_add_lsq(rob_index);
    if (not_available == 0) {
        ROB.entry[rob_index].scheduled = COMPLETED;
        if (ROB.entry[rob_index].executed == 0) // it could be already set to COMPLETED due to store-to-load forwarding
            ROB.entry[rob_index].executed  = INFLIGHT;

        DP (if (warmup_complete[cpu]) {
        cout << "[ROB] " << __func__ << " instr_id: " << ROB.entry[rob_index].instr_id << " rob_index: " << rob_index;
        cout << " scheduled all num_mem_ops: " << ROB.entry[rob_index].num_mem_ops << endl; });
				
	      if(DETAILED_PRINT || (ROB.entry[rob_index].instr_id >= range_start && ROB.entry[rob_index].instr_id <= range_end))
          cout << ".scheduled completed after check_and_add_lsq returned 0";

    }

			if(DETAILED_PRINT || (ROB.entry[rob_index].instr_id >= range_start && ROB.entry[rob_index].instr_id <= range_end))
      cout <<" Inside do_memory_scheduling "<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " ROB.entry[rob_index].ip:  " << ROB.entry[rob_index].ip<<endl;


    num_searched++;
}

uint32_t O3_CPU::check_and_add_lsq(uint32_t rob_index) 
{
    uint32_t num_mem_ops = 0, num_added = 0;

    // load
    for (uint32_t i=0; i<NUM_INSTR_SOURCES; i++) {
        if (ROB.entry[rob_index].source_memory[i]) {
            num_mem_ops++;
            if (ROB.entry[rob_index].source_added[i])
                num_added++;
            else if (LQ.occupancy < LQ.SIZE) {
                add_load_queue(rob_index, i);
                num_added++;
            }
            else {
                DP(if(warmup_complete[cpu]) {
                cout << "[LQ] " << __func__ << " instr_id: " << ROB.entry[rob_index].instr_id;
                cout << " cannot be added in the load queue occupancy: " << LQ.occupancy << " cycle: " << current_core_cycle[cpu] << endl; });
            }
        }
    }

    // store
    for (uint32_t i=0; i<MAX_INSTR_DESTINATIONS; i++) {
        if (ROB.entry[rob_index].destination_memory[i]) {
            num_mem_ops++;
            if (ROB.entry[rob_index].destination_added[i])
                num_added++;
            else if (SQ.occupancy < SQ.SIZE) {
                if (STA[STA_head] == ROB.entry[rob_index].instr_id) {
                    add_store_queue(rob_index, i);
                    num_added++;
                }
                //add_store_queue(rob_index, i);
                //num_added++;
            }
            else {
                DP(if(warmup_complete[cpu]) {
                cout << "[SQ] " << __func__ << " instr_id: " << ROB.entry[rob_index].instr_id;
                cout << " cannot be added in the store queue occupancy: " << SQ.occupancy << " cycle: " << current_core_cycle[cpu] << endl; });
            }
        }
    }

    if (num_added == num_mem_ops)
        return 0;

    uint32_t not_available = num_mem_ops - num_added;
    if (not_available > num_mem_ops) {
        cerr << "instr_id: " << ROB.entry[rob_index].instr_id << endl;
        assert(0);
    }

    return not_available;
}

void O3_CPU::add_load_queue(uint32_t rob_index, uint32_t data_index)
{
    // search for an empty slot 
    uint32_t lq_index = LQ.SIZE;
    for (uint32_t i=0; i<LQ.SIZE; i++) {
        if (LQ.entry[i].virtual_address == 0) {
            lq_index = i;
            break;
        }
    }

    // sanity check
    if (lq_index == LQ.SIZE) {
        cerr << "instr_id: " << ROB.entry[rob_index].instr_id << " no empty slot in the load queue!!!" << endl;
        assert(0);
    }

    // add it to the load queue
    ROB.entry[rob_index].lq_index[data_index] = lq_index;
    LQ.entry[lq_index].instr_id = ROB.entry[rob_index].instr_id;
    LQ.entry[lq_index].virtual_address = ROB.entry[rob_index].source_memory[data_index];
    LQ.entry[lq_index].ip = ROB.entry[rob_index].ip;
    LQ.entry[lq_index].data_index = data_index;
    LQ.entry[lq_index].rob_index = rob_index;
    LQ.entry[lq_index].asid[0] = ROB.entry[rob_index].asid[0];
    LQ.entry[lq_index].asid[1] = ROB.entry[rob_index].asid[1];
    LQ.entry[lq_index].event_cycle = current_core_cycle[cpu] + SCHEDULING_LATENCY;
    LQ.occupancy++;

		 if(DETAILED_PRINT || (LQ.entry[lq_index].instr_id >= range_start && LQ.entry[lq_index].instr_id <= range_end))
        cout <<" Added data_index to LQ,"<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " event_cycle: "<<LQ.entry[lq_index].event_cycle<< " SCHEDULING_LATENCY: "<<SCHEDULING_LATENCY<<endl;


    // check RAW dependency
    int prior = rob_index - 1;
    if (prior < 0)
        prior = ROB.SIZE - 1;

    if (rob_index != ROB.head) {
        if ((int)ROB.head <= prior) {
            for (int i=prior; i>=(int)ROB.head; i--) {
                if (LQ.entry[lq_index].producer_id != UINT64_MAX)
                    break;

                mem_RAW_dependency(i, rob_index, data_index, lq_index);
            }
        }
        else {
            for (int i=prior; i>=0; i--) {
                if (LQ.entry[lq_index].producer_id != UINT64_MAX)
                    break;

                mem_RAW_dependency(i, rob_index, data_index, lq_index);
            }
            for (int i=ROB.SIZE-1; i>=(int)ROB.head; i--) { 
                if (LQ.entry[lq_index].producer_id != UINT64_MAX)
                    break;

                mem_RAW_dependency(i, rob_index, data_index, lq_index);
            }
        }
    }

    // check
    // 1) if store-to-load forwarding is possible
    // 2) if there is WAR that are not correctly executed
    uint32_t forwarding_index = SQ.SIZE;
    for (uint32_t i=0; i<SQ.SIZE; i++) {

        // skip empty slot
        if (SQ.entry[i].virtual_address == 0)
            continue;

        // forwarding should be done by the SQ entry that holds the same producer_id from RAW dependency check
        if (SQ.entry[i].virtual_address == LQ.entry[lq_index].virtual_address) { // store-to-load forwarding check

            // forwarding store is in the SQ
            if ((rob_index != ROB.head) && (LQ.entry[lq_index].producer_id == SQ.entry[i].instr_id)) { // RAW
                forwarding_index = i;
                break; // should be break
            }

            if ((LQ.entry[lq_index].producer_id == UINT64_MAX) && (LQ.entry[lq_index].instr_id <= SQ.entry[i].instr_id)) { // WAR 
                // a load is about to be added in the load queue and we found a store that is 
                // "logically later in the program order but already executed" => this is not correctly executed WAR
                // due to out-of-order execution, this case is possible, for example
                // 1) application is load intensive and load queue is full
                // 2) we have loads that can't be added in the load queue
                // 3) subsequent stores logically behind in the program order are added in the store queue first

                // thanks to the store buffer, data is not written back to the memory system until retirement
                // also due to in-order retirement, this "already executed store" cannot be retired until we finish the prior load instruction 
                // if we detect WAR when a load is added in the load queue, just let the load instruction to access the memory system
                // no need to mark any dependency because this is actually WAR not RAW

                // do not forward data from the store queue since this is WAR
                // just read correct data from data cache

                LQ.entry[lq_index].physical_address = 0;
                LQ.entry[lq_index].translated = 0;
                LQ.entry[lq_index].fetched = 0;
                
                DP(if(warmup_complete[cpu]) {
                cout << "[LQ] " << __func__ << " instr_id: " << LQ.entry[lq_index].instr_id << " reset fetched: " << +LQ.entry[lq_index].fetched;
                cout << " to obey WAR store instr_id: " << SQ.entry[i].instr_id << " cycle: " << current_core_cycle[cpu] << endl; });
            }
        }
    }

    if (forwarding_index != SQ.SIZE) { // we have a store-to-load forwarding

        if ((SQ.entry[forwarding_index].fetched == COMPLETED) && (SQ.entry[forwarding_index].event_cycle <= current_core_cycle[cpu])) {
            LQ.entry[lq_index].physical_address = (SQ.entry[forwarding_index].physical_address & ~(uint64_t) ((1 << LOG2_BLOCK_SIZE) - 1)) | (LQ.entry[lq_index].virtual_address & ((1 << LOG2_BLOCK_SIZE) - 1));
            LQ.entry[lq_index].translated = COMPLETED;
            LQ.entry[lq_index].fetched = COMPLETED;

            uint32_t fwr_rob_index = LQ.entry[lq_index].rob_index;
            ROB.entry[fwr_rob_index].num_mem_ops--;
            ROB.entry[fwr_rob_index].event_cycle = current_core_cycle[cpu];
            if (ROB.entry[fwr_rob_index].num_mem_ops < 0) {
                cerr << "instr_id: " << ROB.entry[fwr_rob_index].instr_id << endl;
                assert(0);
            }
            if (ROB.entry[fwr_rob_index].num_mem_ops == 0)
                inflight_mem_executions++;

            DP(if(warmup_complete[cpu]) {
            cout << "[LQ] " << __func__ << " instr_id: " << LQ.entry[lq_index].instr_id << hex;
            cout << " full_addr: " << LQ.entry[lq_index].physical_address << dec << " is forwarded by store instr_id: ";
            cout << SQ.entry[forwarding_index].instr_id << " remain_num_ops: " << ROB.entry[fwr_rob_index].num_mem_ops << " cycle: " << current_core_cycle[cpu] << endl; });

            release_load_queue(lq_index);
        }
        else
            ; // store is not executed yet, forwarding will be handled by execute_store()
    }

    // succesfully added to the load queue
    ROB.entry[rob_index].source_added[data_index] = 1;

    if (LQ.entry[lq_index].virtual_address && (LQ.entry[lq_index].producer_id == UINT64_MAX)) { // not released and no forwarding
        RTL0[RTL0_tail] = lq_index;
        RTL0_tail++;
				 if(DETAILED_PRINT || (LQ.entry[lq_index].instr_id >= range_start && LQ.entry[lq_index].instr_id <= range_end))
         cout <<LQ.entry[lq_index].ip<<" Inserted into RTL0 "<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " event_cycle: "<<LQ.entry[lq_index].event_cycle<<endl;

        if (RTL0_tail == LQ_SIZE)
            RTL0_tail = 0;

        DP (if (warmup_complete[cpu]) {
        cout << "[RTL0] " << __func__ << " instr_id: " << LQ.entry[lq_index].instr_id << " rob_index: " << LQ.entry[lq_index].rob_index << " is added to RTL0";
        cout << " head: " << RTL0_head << " tail: " << RTL0_tail << endl; }); 
    }

    DP(if(warmup_complete[cpu]) {
    cout << "[LQ] " << __func__ << " instr_id: " << LQ.entry[lq_index].instr_id;
    cout << " is added in the LQ address: " << hex << LQ.entry[lq_index].virtual_address << dec << " translated: " << +LQ.entry[lq_index].translated;
    cout << " fetched: " << +LQ.entry[lq_index].fetched << " index: " << lq_index << " occupancy: " << LQ.occupancy << " cycle: " << current_core_cycle[cpu] << endl; });
}

void O3_CPU::mem_RAW_dependency(uint32_t prior, uint32_t current, uint32_t data_index, uint32_t lq_index)
{
    for (uint32_t i=0; i<MAX_INSTR_DESTINATIONS; i++) {
        if (ROB.entry[prior].destination_memory[i] == 0)
            continue;

        if (ROB.entry[prior].destination_memory[i] == ROB.entry[current].source_memory[data_index]) { //  store-to-load forwarding check

            // we need to mark this dependency in the ROB since the producer might not be added in the store queue yet
            ROB.entry[prior].memory_instrs_depend_on_me.insert (current);  // this load cannot be executed until the prior store gets executed
            ROB.entry[prior].is_producer = 1;
            LQ.entry[lq_index].producer_id = ROB.entry[prior].instr_id; 
            LQ.entry[lq_index].translated = INFLIGHT;

            DP (if(warmup_complete[cpu]) {
            cout << "[LQ] " << __func__ << " RAW producer instr_id: " << ROB.entry[prior].instr_id << " consumer_id: " << ROB.entry[current].instr_id << " lq_index: " << lq_index;
            cout << hex << " address: " << ROB.entry[prior].destination_memory[i] << dec << endl; });

            return;
        }
    }
}

void O3_CPU::add_store_queue(uint32_t rob_index, uint32_t data_index)
{
    uint32_t sq_index = SQ.tail;
#ifdef SANITY_CHECK
    if (SQ.entry[sq_index].virtual_address)
        assert(0);
#endif

    /*
    // search for an empty slot 
    uint32_t sq_index = SQ.SIZE;
    for (uint32_t i=0; i<SQ.SIZE; i++) {
        if (SQ.entry[i].virtual_address == 0) {
            sq_index = i;
            break;
        }
    }

    // sanity check
    if (sq_index == SQ.SIZE) {
        cerr << "instr_id: " << ROB.entry[rob_index].instr_id << " no empty slot in the store queue!!!" << endl;
        assert(0);
    }
    */

    // add it to the store queue
    ROB.entry[rob_index].sq_index[data_index] = sq_index;
    SQ.entry[sq_index].instr_id = ROB.entry[rob_index].instr_id;
    SQ.entry[sq_index].virtual_address = ROB.entry[rob_index].destination_memory[data_index];
    SQ.entry[sq_index].ip = ROB.entry[rob_index].ip;
    SQ.entry[sq_index].data_index = data_index;
    SQ.entry[sq_index].rob_index = rob_index;
    SQ.entry[sq_index].asid[0] = ROB.entry[rob_index].asid[0];
    SQ.entry[sq_index].asid[1] = ROB.entry[rob_index].asid[1];
    SQ.entry[sq_index].event_cycle = current_core_cycle[cpu] + SCHEDULING_LATENCY;

    SQ.occupancy++;
    SQ.tail++;
    if (SQ.tail == SQ.SIZE)
        SQ.tail = 0;

    // succesfully added to the store queue
    ROB.entry[rob_index].destination_added[data_index] = 1;

		if(DETAILED_PRINT || (ROB.entry[rob_index].instr_id >= range_start && ROB.entry[rob_index].instr_id <= range_end))
		cout <<SQ.entry[sq_index].ip << " is added in the SQ translated: " << "current_core_cycle[cpu]: "<<current_core_cycle[cpu] << endl;  
 
    STA[STA_head] = UINT64_MAX;
    STA_head++;
    if (STA_head == STA_SIZE)
        STA_head = 0;

    RTS0[RTS0_tail] = sq_index;
    RTS0_tail++;
    if (RTS0_tail == SQ_SIZE)
        RTS0_tail = 0;

    DP(if(warmup_complete[cpu]) {
    cout << "[SQ] " << __func__ << " instr_id: " << SQ.entry[sq_index].instr_id;
    cout << " is added in the SQ translated: " << +SQ.entry[sq_index].translated << " fetched: " << +SQ.entry[sq_index].fetched << " is_producer: " << +ROB.entry[rob_index].is_producer;
    cout << " cycle: " << current_core_cycle[cpu] << endl; });
}

void O3_CPU::operate_lsq()
{
    // handle store
    uint32_t store_issued = 0, num_iteration = 0;

    while (store_issued < SQ_WIDTH) {
        if (RTS0[RTS0_head] < SQ_SIZE) {
            uint32_t sq_index = RTS0[RTS0_head];
            if (SQ.entry[sq_index].event_cycle <= current_core_cycle[cpu]) {

                // add it to DTLB
                PACKET data_packet;

                data_packet.tlb_access = 1;
                data_packet.fill_level = FILL_L1;
                data_packet.cpu = cpu;
                data_packet.data_index = SQ.entry[sq_index].data_index;
                data_packet.sq_index = sq_index;
                if (knob_cloudsuite)
                    data_packet.address = ((SQ.entry[sq_index].virtual_address >> LOG2_PAGE_SIZE) << 9) | SQ.entry[sq_index].asid[1];
                else
                    data_packet.address = SQ.entry[sq_index].virtual_address >> LOG2_PAGE_SIZE;
                data_packet.full_addr = SQ.entry[sq_index].virtual_address;
                data_packet.instr_id = SQ.entry[sq_index].instr_id;
                data_packet.rob_index = SQ.entry[sq_index].rob_index;
                data_packet.ip = SQ.entry[sq_index].ip;
                data_packet.type = RFO;
                data_packet.asid[0] = SQ.entry[sq_index].asid[0];
                data_packet.asid[1] = SQ.entry[sq_index].asid[1];
                data_packet.event_cycle = SQ.entry[sq_index].event_cycle;

                DP (if (warmup_complete[cpu]) {
                cout << "[RTS0] " << __func__ << " instr_id: " << SQ.entry[sq_index].instr_id << " rob_index: " << SQ.entry[sq_index].rob_index << " is popped from to RTS0";
                cout << " head: " << RTS0_head << " tail: " << RTS0_tail << endl; }); 

                int rq_index = DTLB.add_rq(&data_packet);


                if (rq_index == -2)
                    break; 
                else 
                    {SQ.entry[sq_index].translated = INFLIGHT;  
								if(DETAILED_PRINT || (SQ.entry[sq_index].instr_id >= range_start && SQ.entry[sq_index].instr_id <= range_end))
                cout << "added_to DTLB rq, .translated is inflight: "<<SQ.entry[sq_index].ip << " current_core-cycle[cpu]: " <<current_core_cycle[cpu]<<endl; }

                RTS0[RTS0_head] = SQ_SIZE;
                RTS0_head++;
                if (RTS0_head == SQ_SIZE)
                    RTS0_head = 0;

                store_issued++;
            }
        }
        else {
            //DP (if (warmup_complete[cpu]) {
            //cout << "[RTS0] is empty head: " << RTS0_head << " tail: " << RTS0_tail << endl; });
            break;
        }

        num_iteration++;
        if (num_iteration == (SQ_SIZE-1))
            break;
    }

    num_iteration = 0;
    while (store_issued < SQ_WIDTH) {
        if (RTS1[RTS1_head] < SQ_SIZE) {
            uint32_t sq_index = RTS1[RTS1_head];
            if (SQ.entry[sq_index].event_cycle <= current_core_cycle[cpu]) {
                execute_store(SQ.entry[sq_index].rob_index, sq_index, SQ.entry[sq_index].data_index);

								if(DETAILED_PRINT || (SQ.entry[sq_index].instr_id >= range_start && SQ.entry[sq_index].instr_id <= range_end)) 
								cout<<SQ.entry[sq_index].ip <<" Issued execute_store "<<" current_core_cycle[0]: " <<current_core_cycle[0]<<endl;
                RTS1[RTS1_head] = SQ_SIZE;
                RTS1_head++;
                if (RTS1_head == SQ_SIZE)
                    RTS1_head = 0;

                store_issued++;
            }
        }
        else {
            //DP (if (warmup_complete[cpu]) {
            //cout << "[RTS1] is empty head: " << RTS1_head << " tail: " << RTS1_tail << endl; });
            break;
        }

        num_iteration++;
        if (num_iteration == (SQ_SIZE-1))
            break;
    }

    unsigned load_issued = 0;
    num_iteration = 0;
    while (load_issued < LQ_WIDTH) {
        if (RTL0[RTL0_head] < LQ_SIZE) {
            uint32_t lq_index = RTL0[RTL0_head];
            if (LQ.entry[lq_index].event_cycle <= current_core_cycle[cpu]) {
						  if(DETAILED_PRINT || (LQ.entry[lq_index].instr_id >= range_start && LQ.entry[lq_index].instr_id <= range_end))
              cout <<" Inserted into DTLB read queue "<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " event_cycle: "<<LQ.entry[lq_index].event_cycle<<endl;

                // add it to DTLB
                PACKET data_packet;
                data_packet.fill_level = FILL_L1;
                data_packet.cpu = cpu;
                data_packet.data_index = LQ.entry[lq_index].data_index;
                data_packet.lq_index = lq_index;
                if (knob_cloudsuite)
                    data_packet.address = ((LQ.entry[lq_index].virtual_address >> LOG2_PAGE_SIZE) << 9) | LQ.entry[lq_index].asid[1];
                else
                    data_packet.address = LQ.entry[lq_index].virtual_address >> LOG2_PAGE_SIZE;
                data_packet.full_addr = LQ.entry[lq_index].virtual_address;
                data_packet.instr_id = LQ.entry[lq_index].instr_id;
                data_packet.rob_index = LQ.entry[lq_index].rob_index;
                data_packet.ip = LQ.entry[lq_index].ip;
                data_packet.type = LOAD;
                data_packet.asid[0] = LQ.entry[lq_index].asid[0];
                data_packet.asid[1] = LQ.entry[lq_index].asid[1];
                data_packet.event_cycle = LQ.entry[lq_index].event_cycle;

                DP (if (warmup_complete[cpu]) {
                cout << "[RTL0] " << __func__ << " instr_id: " << LQ.entry[lq_index].instr_id << " rob_index: " << LQ.entry[lq_index].rob_index << " is popped to RTL0";
                cout << " head: " << RTL0_head << " tail: " << RTL0_tail << endl; }); 

                int rq_index = DTLB.add_rq(&data_packet);

                if (rq_index == -2)
                    break; // break here
                else  
                    {LQ.entry[lq_index].translated = INFLIGHT; if(DETAILED_PRINT || (LQ.entry[lq_index].instr_id >= range_start && LQ.entry[lq_index].instr_id <= range_end)) cout <<" Inserted into DTLB read queue, .translated is inflight, load issued "<<" current_core_cycle[0]: " <<current_core_cycle[0]<<endl; }

                RTL0[RTL0_head] = LQ_SIZE;
                RTL0_head++;
                if (RTL0_head == LQ_SIZE)
                    RTL0_head = 0;

                load_issued++;
            }
        }
        else {
            //DP (if (warmup_complete[cpu]) {
            //cout << "[RTL0] is empty head: " << RTL0_head << " tail: " << RTL0_tail << endl; });
            break;
        }

        num_iteration++;
        if (num_iteration == (LQ_SIZE-1))
            break;
    }

    num_iteration = 0;
    while (load_issued < LQ_WIDTH) {
        if (RTL1[RTL1_head] < LQ_SIZE) {
            uint32_t lq_index = RTL1[RTL1_head];
            if (LQ.entry[lq_index].event_cycle <= current_core_cycle[cpu]) {
							 if(DETAILED_PRINT || (LQ.entry[lq_index].instr_id >= range_start && LQ.entry[lq_index].instr_id <= range_end))
                 cout<<LQ.entry[lq_index].ip <<" Issued execute_load "<<" current_core_cycle[0]: " <<current_core_cycle[0]<<endl;	
                int rq_index = execute_load(LQ.entry[lq_index].rob_index, lq_index, LQ.entry[lq_index].data_index);

                if (rq_index != -2) {
                    RTL1[RTL1_head] = LQ_SIZE;
                    RTL1_head++;
                    if (RTL1_head == LQ_SIZE)
                        RTL1_head = 0;

                    load_issued++;
                }
            }
        }
        else {
            //DP (if (warmup_complete[cpu]) {
            //cout << "[RTL1] is empty head: " << RTL1_head << " tail: " << RTL1_tail << endl; });
            break;
        }

        num_iteration++;
        if (num_iteration == (LQ_SIZE-1))
            break;
    }
}

void O3_CPU::execute_store(uint32_t rob_index, uint32_t sq_index, uint32_t data_index)
{
    SQ.entry[sq_index].fetched = COMPLETED;
    SQ.entry[sq_index].event_cycle = current_core_cycle[cpu];

    ROB.entry[rob_index].num_mem_ops--;
    ROB.entry[rob_index].event_cycle = current_core_cycle[cpu];
    if (ROB.entry[rob_index].num_mem_ops < 0) {
        cerr << "instr_id: " << ROB.entry[rob_index].instr_id << endl;
        assert(0);
    }
    if (ROB.entry[rob_index].num_mem_ops == 0)
        inflight_mem_executions++;

    DP (if (warmup_complete[cpu]) {
    cout << "[SQ1] " << __func__ << " instr_id: " << SQ.entry[sq_index].instr_id << hex;
    cout << " full_address: " << SQ.entry[sq_index].physical_address << dec << " remain_mem_ops: " << ROB.entry[rob_index].num_mem_ops;
    cout << " event_cycle: " << SQ.entry[sq_index].event_cycle << endl; });

    // resolve RAW dependency after DTLB access
    // check if this store has dependent loads
    if (ROB.entry[rob_index].is_producer) {
	ITERATE_SET(dependent,ROB.entry[rob_index].memory_instrs_depend_on_me, ROB_SIZE) {
            // check if dependent loads are already added in the load queue
            for (uint32_t j=0; j<NUM_INSTR_SOURCES; j++) { // which one is dependent?
                if (ROB.entry[dependent].source_memory[j] && ROB.entry[dependent].source_added[j]) {
                    if (ROB.entry[dependent].source_memory[j] == SQ.entry[sq_index].virtual_address) { // this is required since a single instruction can issue multiple loads

                        // now we can resolve RAW dependency
                        uint32_t lq_index = ROB.entry[dependent].lq_index[j];
#ifdef SANITY_CHECK
                        if (lq_index >= LQ.SIZE)
                            assert(0);
                        if (LQ.entry[lq_index].producer_id != SQ.entry[sq_index].instr_id) {
                            cerr << "[SQ2] " << __func__ << " lq_index: " << lq_index << " producer_id: " << LQ.entry[lq_index].producer_id;
                            cerr << " does not match to the store instr_id: " << SQ.entry[sq_index].instr_id << endl;
                            assert(0);
                        }
#endif
                        // update correspodning LQ entry
                        LQ.entry[lq_index].physical_address = (SQ.entry[sq_index].physical_address & ~(uint64_t) ((1 << LOG2_BLOCK_SIZE) - 1)) | (LQ.entry[lq_index].virtual_address & ((1 << LOG2_BLOCK_SIZE) - 1));
                        LQ.entry[lq_index].translated = COMPLETED;
                        LQ.entry[lq_index].fetched = COMPLETED;
                        LQ.entry[lq_index].event_cycle = current_core_cycle[cpu];

                        uint32_t fwr_rob_index = LQ.entry[lq_index].rob_index;
                        ROB.entry[fwr_rob_index].num_mem_ops--;
                        ROB.entry[fwr_rob_index].event_cycle = current_core_cycle[cpu];
#ifdef SANITY_CHECK
                        if (ROB.entry[fwr_rob_index].num_mem_ops < 0) {
                            cerr << "instr_id: " << ROB.entry[fwr_rob_index].instr_id << endl;
                            assert(0);
                        }
#endif
                        if (ROB.entry[fwr_rob_index].num_mem_ops == 0)
                            inflight_mem_executions++;

                        DP(if(warmup_complete[cpu]) {
                        cout << "[LQ3] " << __func__ << " instr_id: " << LQ.entry[lq_index].instr_id << hex;
                        cout << " full_addr: " << LQ.entry[lq_index].physical_address << dec << " is forwarded by store instr_id: ";
                        cout << SQ.entry[sq_index].instr_id << " remain_num_ops: " << ROB.entry[fwr_rob_index].num_mem_ops << " cycle: " << current_core_cycle[cpu] << endl; });

                        release_load_queue(lq_index);

                        // clear dependency bit
                        if (j == (NUM_INSTR_SOURCES-1))
                            ROB.entry[rob_index].memory_instrs_depend_on_me.insert (dependent);
                    }
                }
            }
        }
    }
}

int O3_CPU::execute_load(uint32_t rob_index, uint32_t lq_index, uint32_t data_index)
{
    // add it to L1D
    PACKET data_packet;
    data_packet.fill_level = FILL_L1;
    data_packet.cpu = cpu;
    data_packet.data_index = LQ.entry[lq_index].data_index;
    data_packet.lq_index = lq_index;
    data_packet.address = LQ.entry[lq_index].physical_address >> LOG2_BLOCK_SIZE;
    data_packet.full_addr = LQ.entry[lq_index].physical_address;
    data_packet.instr_id = LQ.entry[lq_index].instr_id;
    data_packet.rob_index = LQ.entry[lq_index].rob_index;
    data_packet.ip = LQ.entry[lq_index].ip;
    data_packet.type = LOAD;
    data_packet.asid[0] = LQ.entry[lq_index].asid[0];
    data_packet.asid[1] = LQ.entry[lq_index].asid[1];
    data_packet.event_cycle = LQ.entry[lq_index].event_cycle;

    int rq_index = L1D.add_rq(&data_packet);

			if(DETAILED_PRINT || (LQ.entry[lq_index].instr_id >= range_start && LQ.entry[lq_index].instr_id <= range_end))	
      cout <<" Inside execute_load, added_to_L1D rq "<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " LQ.entry[lq_index].ip:  " << LQ.entry[lq_index].ip<<endl;
	
		

    if (rq_index == -2)
        return rq_index;
    else 
        LQ.entry[lq_index].fetched = INFLIGHT;

    return rq_index;
}

void O3_CPU::complete_execution(uint32_t rob_index)
{
    if (ROB.entry[rob_index].is_memory == 0) {
        if ((ROB.entry[rob_index].executed == INFLIGHT) && (ROB.entry[rob_index].event_cycle <= current_core_cycle[cpu])) {

			if(DETAILED_PRINT || (ROB.entry[rob_index].instr_id >= range_start && ROB.entry[rob_index].instr_id <= range_end))
      cout <<ROB.entry[rob_index].ip<<" Inside complete_execution for non-memory instruction "<<" current_core_cycle[0]: " <<current_core_cycle[0]<< endl;

            ROB.entry[rob_index].executed = COMPLETED; 
            inflight_reg_executions--;
            completed_executions++;

            if (ROB.entry[rob_index].reg_RAW_producer)
                reg_RAW_release(rob_index);

            if (ROB.entry[rob_index].branch_mispredicted) 
                fetch_stall = 0;

            DP(if(warmup_complete[cpu]) {
            cout << "[ROB] " << __func__ << " instr_id: " << ROB.entry[rob_index].instr_id;
            cout << " branch_mispredicted: " << +ROB.entry[rob_index].branch_mispredicted << " fetch_stall: " << +fetch_stall;
            cout << " event: " << ROB.entry[rob_index].event_cycle << endl; });
        }
    }
    else {
        if (ROB.entry[rob_index].num_mem_ops == 0) {
            if ((ROB.entry[rob_index].executed == INFLIGHT) && (ROB.entry[rob_index].event_cycle <= current_core_cycle[cpu])) {
                ROB.entry[rob_index].executed = COMPLETED;
                inflight_mem_executions--;
                completed_executions++;

			if(DETAILED_PRINT || (ROB.entry[rob_index].instr_id >= range_start && ROB.entry[rob_index].instr_id <= range_end))
      cout <<ROB.entry[rob_index].ip<<" Inside complete_execution, exected completed for non-memory instruction "<<" current_core_cycle[0]: " <<current_core_cycle[0]<< endl;            
       
                if (ROB.entry[rob_index].reg_RAW_producer)
                    reg_RAW_release(rob_index);

                if (ROB.entry[rob_index].branch_mispredicted) 
                    fetch_stall = 0;

                DP(if(warmup_complete[cpu]) {
                cout << "[ROB] " << __func__ << " instr_id: " << ROB.entry[rob_index].instr_id;
                cout << " is_memory: " << +ROB.entry[rob_index].is_memory << " branch_mispredicted: " << +ROB.entry[rob_index].branch_mispredicted;
                cout << " fetch_stall: " << +fetch_stall << " event: " << ROB.entry[rob_index].event_cycle << " current: " << current_core_cycle[cpu] << endl; });
            }
        }
    }
}

void O3_CPU::reg_RAW_release(uint32_t rob_index)
{
    // if (!ROB.entry[rob_index].registers_instrs_depend_on_me.empty()) 

    ITERATE_SET(i,ROB.entry[rob_index].registers_instrs_depend_on_me, ROB_SIZE) {
        for (uint32_t j=0; j<NUM_INSTR_SOURCES; j++) {
            if (ROB.entry[rob_index].registers_index_depend_on_me[j].search (i)) {
                ROB.entry[i].num_reg_dependent--;

                if (ROB.entry[i].num_reg_dependent == 0) {

									if(DETAILED_PRINT || (ROB.entry[i].instr_id >= range_start && ROB.entry[i].instr_id <= range_end))	
            cout <<" Inside reg_RAW_release "<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " ROB.entry[i].ip:  " << ROB.entry[i].ip<<endl;

                    ROB.entry[i].reg_ready = 1;
                    if (ROB.entry[i].is_memory)
                        ROB.entry[i].scheduled = INFLIGHT;
                    else {
                        ROB.entry[i].scheduled = COMPLETED;

#ifdef SANITY_CHECK
                        if (RTE0[RTE0_tail] < ROB_SIZE)
                            assert(0);
#endif
                        // remember this rob_index in the Ready-To-Execute array 0
                        RTE0[RTE0_tail] = i;

                        DP (if (warmup_complete[cpu]) {
                        cout << "[RTE0] " << __func__ << " instr_id: " << ROB.entry[i].instr_id << " rob_index: " << i << " is added to RTE0";
                        cout << " head: " << RTE0_head << " tail: " << RTE0_tail << endl; }); 

                        RTE0_tail++;
                        if (RTE0_tail == ROB_SIZE)
                            RTE0_tail = 0;

                    }
                }

                DP (if (warmup_complete[cpu]) {
                cout << "[ROB] " << __func__ << " instr_id: " << ROB.entry[rob_index].instr_id << " releases instr_id: ";
                cout << ROB.entry[i].instr_id << " reg_index: " << +ROB.entry[i].source_registers[j] << " num_reg_dependent: " << ROB.entry[i].num_reg_dependent << " cycle: " << current_core_cycle[cpu] << endl; });
            }
        }
    }
}

void O3_CPU::operate_cache()
{
    ITLB.operate();
    DTLB.operate();
    STLB.operate();
    L1I.operate();
    L1D.operate();
    L2C.operate();
}

void O3_CPU::update_rob()
{
    if (ITLB.PROCESSED.occupancy && (ITLB.PROCESSED.entry[ITLB.PROCESSED.head].event_cycle <= current_core_cycle[cpu]))
        complete_instr_fetch(&ITLB.PROCESSED, 1);

    if (L1I.PROCESSED.occupancy && (L1I.PROCESSED.entry[L1I.PROCESSED.head].event_cycle <= current_core_cycle[cpu]))
        complete_instr_fetch(&L1I.PROCESSED, 0);

    if (DTLB.PROCESSED.occupancy && (DTLB.PROCESSED.entry[DTLB.PROCESSED.head].event_cycle <= current_core_cycle[cpu]))
        complete_data_fetch(&DTLB.PROCESSED, 1);

    if (L1D.PROCESSED.occupancy && (L1D.PROCESSED.entry[L1D.PROCESSED.head].event_cycle <= current_core_cycle[cpu]))
        complete_data_fetch(&L1D.PROCESSED, 0);

    // update ROB entries with completed executions
    if ((inflight_reg_executions > 0) || (inflight_mem_executions > 0)) {
        if (ROB.head < ROB.tail) {
            for (uint32_t i=ROB.head; i<ROB.tail; i++) 
                complete_execution(i);
        }
        else {
            for (uint32_t i=ROB.head; i<ROB.SIZE; i++)
                complete_execution(i);
            for (uint32_t i=0; i<ROB.tail; i++)
                complete_execution(i);
        }
    }
}

void O3_CPU::complete_instr_fetch(PACKET_QUEUE *queue, uint8_t is_it_tlb)
{
    uint32_t index = queue->head,
             rob_index = queue->entry[index].rob_index,
             num_fetched = 0;

#ifdef SANITY_CHECK
    if (rob_index != check_rob(queue->entry[index].instr_id))
        assert(0);
#endif

		if(DETAILED_PRINT || (ROB.entry[rob_index].instr_id >= range_start && ROB.entry[rob_index].instr_id <= range_end))
        cout << "Inside complete_instr_fetch "<<ROB.entry[rob_index].instr_id<<" ";

    // update ROB entry
    if (is_it_tlb) {
				if(DETAILED_PRINT || (ROB.entry[rob_index].instr_id >= range_start && ROB.entry[rob_index].instr_id <= range_end))
        cout <<ROB.entry[rob_index].ip << "  Inside comp_instr_fetch: "<<" ROB.entry[fetch_index].translated completed "<< "current_core_cycle[cpu] : " << current_core_cycle[cpu]<<endl;
        ROB.entry[rob_index].translated = COMPLETED;
        ROB.entry[rob_index].instruction_pa = (queue->entry[index].instruction_pa << LOG2_PAGE_SIZE) | (ROB.entry[rob_index].ip & ((1 << LOG2_PAGE_SIZE) - 1)); // translated address
    }
    else
        ROB.entry[rob_index].fetched = COMPLETED;
    ROB.entry[rob_index].event_cycle = current_core_cycle[cpu];
    num_fetched++;
		if(DETAILED_PRINT || (ROB.entry[rob_index].instr_id >= range_start && ROB.entry[rob_index].instr_id <= range_end))
    cout <<ROB.entry[rob_index].ip << "  Inside comp_instr_fetch: "<<" ROB.entry[rob_index].fetched completed "<< "current_core_cycle[cpu] : " << current_core_cycle[cpu]<<endl;

    DP ( if (warmup_complete[cpu]) {
    cout << "[" << queue->NAME << "] " << __func__ << " cpu: " << cpu <<  " instr_id: " << ROB.entry[rob_index].instr_id;
    cout << " ip: " << hex << ROB.entry[rob_index].ip << " address: " << ROB.entry[rob_index].instruction_pa << dec;
    cout << " translated: " << +ROB.entry[rob_index].translated << " fetched: " << +ROB.entry[rob_index].fetched;
    cout << " event_cycle: " << ROB.entry[rob_index].event_cycle << endl; });

    // check if other instructions were merged
    if (queue->entry[index].instr_merged) {
	ITERATE_SET(i,queue->entry[index].rob_index_depend_on_me, ROB_SIZE) {
            // update ROB entry
            if (is_it_tlb) {
                ROB.entry[i].translated = COMPLETED;
                ROB.entry[i].instruction_pa = (queue->entry[index].instruction_pa << LOG2_PAGE_SIZE) | (ROB.entry[i].ip & ((1 << LOG2_PAGE_SIZE) - 1)); // translated address
            
                if(DETAILED_PRINT || (ROB.entry[i].instr_id >= range_start && ROB.entry[i].instr_id <= range_end))
                 cout << "Inside complete_instr_fetch merged "<<ROB.entry[i].instr_id<<" translation" <<" status: "<<(int)ROB.entry[i].translated <<" depends on: "<<queue->entry[index].instr_id<<endl;
            }
            else
            { 
                ROB.entry[i].fetched = COMPLETED;  
                if(DETAILED_PRINT || (ROB.entry[i].instr_id >= range_start && ROB.entry[i].instr_id <= range_end)) 
                    cout << "Inside complete_instr_fetch merged "<<ROB.entry[i].instr_id<<" instr fetch cmplt."<<" cycle: "<<current_core_cycle[cpu] << " status: "<<ROB.entry[i].fetched<<" COMPLETED: "<<COMPLETED<<" depends on: "<<queue->entry[index].instr_id <<endl;
            }      
            ROB.entry[i].event_cycle = current_core_cycle[cpu] + (num_fetched / FETCH_WIDTH);
            num_fetched++;
            DP ( if (warmup_complete[cpu]) {
            cout << "[" << queue->NAME << "] " << __func__ << " cpu: " << cpu <<  " instr_id: " << ROB.entry[i].instr_id;
            cout << " ip: " << hex << ROB.entry[i].ip << " address: " << ROB.entry[i].instruction_pa << dec;
            cout << " translated: " << +ROB.entry[i].translated << " fetched: " << +ROB.entry[i].fetched << " provider: " << ROB.entry[rob_index].instr_id;
            cout << " event_cycle: " << ROB.entry[i].event_cycle << endl; });
        }
    }

    // remove this entry
    queue->remove_queue(&queue->entry[index]);
}

void O3_CPU::complete_data_fetch(PACKET_QUEUE *queue, uint8_t is_it_tlb)
{
    uint32_t index = queue->head,
             rob_index = queue->entry[index].rob_index,
             sq_index = queue->entry[index].sq_index,
             lq_index = queue->entry[index].lq_index;

#ifdef SANITY_CHECK
    if (queue->entry[index].type != RFO) {
        if (rob_index != check_rob(queue->entry[index].instr_id))
            assert(0);
    }
#endif

    // update ROB entry
    if (is_it_tlb) { // DTLB

        if (queue->entry[index].type == RFO) {
            SQ.entry[sq_index].physical_address = (queue->entry[index].data_pa << LOG2_PAGE_SIZE) | (SQ.entry[sq_index].virtual_address & ((1 << LOG2_PAGE_SIZE) - 1)); // translated address
            SQ.entry[sq_index].translated = COMPLETED;
            SQ.entry[sq_index].event_cycle = current_core_cycle[cpu];

						if(DETAILED_PRINT || (SQ.entry[sq_index].instr_id >= range_start && SQ.entry[sq_index].instr_id <= range_end))
						cout << SQ.entry[sq_index].ip <<" Inside complete data_fetch, Inserted into RTS1, after translation is complete for store address"<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " event_cycle: "<<SQ.entry[sq_index].event_cycle<<endl;

            RTS1[RTS1_tail] = sq_index;
            RTS1_tail++;
            if (RTS1_tail == SQ_SIZE)
                RTS1_tail = 0;

            DP (if (warmup_complete[cpu]) {
            cout << "[ROB] " << __func__ << " RFO instr_id: " << SQ.entry[sq_index].instr_id;
            cout << " DTLB_FETCH_DONE translation: " << +SQ.entry[sq_index].translated << hex << " page: " << (SQ.entry[sq_index].physical_address>>LOG2_PAGE_SIZE);
            cout << " full_addr: " << SQ.entry[sq_index].physical_address << dec << " store_merged: " << +queue->entry[index].store_merged;
            cout << " load_merged: " << +queue->entry[index].load_merged << endl; }); 

            handle_merged_translation(&queue->entry[index]);
        }
        else { 
            LQ.entry[lq_index].physical_address = (queue->entry[index].data_pa << LOG2_PAGE_SIZE) | (LQ.entry[lq_index].virtual_address & ((1 << LOG2_PAGE_SIZE) - 1)); // translated address
            LQ.entry[lq_index].translated = COMPLETED;
            LQ.entry[lq_index].event_cycle = current_core_cycle[cpu];

						if(DETAILED_PRINT || (LQ.entry[lq_index].instr_id >= range_start && LQ.entry[lq_index].instr_id <= range_end))
              cout<< LQ.entry[lq_index].event_cycle<<" Inside complete data_fetch, Inserted into RTL1 as translation is complete "<<" current_core_cycle[0]: " <<current_core_cycle[0]<< " event_cycle: "<<LQ.entry[lq_index].event_cycle<<endl;


            RTL1[RTL1_tail] = lq_index;
            RTL1_tail++;
            if (RTL1_tail == LQ_SIZE)
                RTL1_tail = 0;

            DP (if (warmup_complete[cpu]) {
            cout << "[RTL1] " << __func__ << " instr_id: " << LQ.entry[lq_index].instr_id << " rob_index: " << LQ.entry[lq_index].rob_index << " is added to RTL1";
            cout << " head: " << RTL1_head << " tail: " << RTL1_tail << endl; }); 

            DP (if (warmup_complete[cpu]) {
            cout << "[ROB] " << __func__ << " load instr_id: " << LQ.entry[lq_index].instr_id;
            cout << " DTLB_FETCH_DONE translation: " << +LQ.entry[lq_index].translated << hex << " page: " << (LQ.entry[lq_index].physical_address>>LOG2_PAGE_SIZE);
            cout << " full_addr: " << LQ.entry[lq_index].physical_address << dec << " store_merged: " << +queue->entry[index].store_merged;
            cout << " load_merged: " << +queue->entry[index].load_merged << endl; }); 

            handle_merged_translation(&queue->entry[index]);
        }

        ROB.entry[rob_index].event_cycle = queue->entry[index].event_cycle;
    }
    else { // L1D

        if (queue->entry[index].type == RFO)
            handle_merged_load(&queue->entry[index]);
        else { 
#ifdef SANITY_CHECK
            if (queue->entry[index].store_merged)
                assert(0);
#endif
            LQ.entry[lq_index].fetched = COMPLETED;

						if(current_core_cycle[0] >= 7878298 && LQ.entry[lq_index].ip == CHECK_IP)
              cout << ".fetched is completed: " << current_core_cycle[0] << endl;


            LQ.entry[lq_index].event_cycle = current_core_cycle[cpu];
            ROB.entry[rob_index].num_mem_ops--;
            ROB.entry[rob_index].event_cycle = queue->entry[index].event_cycle;

#ifdef SANITY_CHECK
            if (ROB.entry[rob_index].num_mem_ops < 0) {
                cerr << "instr_id: " << ROB.entry[rob_index].instr_id << endl;
                assert(0);
            }
#endif
            if (ROB.entry[rob_index].num_mem_ops == 0)
                inflight_mem_executions++;

            DP (if (warmup_complete[cpu]) {
            cout << "[ROB] " << __func__ << " load instr_id: " << LQ.entry[lq_index].instr_id;
            cout << " L1D_FETCH_DONE fetched: " << +LQ.entry[lq_index].fetched << hex << " address: " << (LQ.entry[lq_index].physical_address>>LOG2_BLOCK_SIZE);
            cout << " full_addr: " << LQ.entry[lq_index].physical_address << dec << " remain_mem_ops: " << ROB.entry[rob_index].num_mem_ops;
            cout << " load_merged: " << +queue->entry[index].load_merged << " inflight_mem: " << inflight_mem_executions << endl; }); 

            release_load_queue(lq_index);
            handle_merged_load(&queue->entry[index]);
        }
    }

    // remove this entry
    queue->remove_queue(&queue->entry[index]);
}

void O3_CPU::handle_o3_fetch(PACKET *current_packet, uint32_t cache_type)
{
    uint32_t rob_index = current_packet->rob_index,
             sq_index  = current_packet->sq_index,
             lq_index  = current_packet->lq_index;

    // update ROB entry
    if (cache_type == 0) { // DTLB

#ifdef SANITY_CHECK
        if (rob_index != check_rob(current_packet->instr_id))
            assert(0);
#endif
        if (current_packet->type == RFO) {
            SQ.entry[sq_index].physical_address = (current_packet->data_pa << LOG2_PAGE_SIZE) | (SQ.entry[sq_index].virtual_address & ((1 << LOG2_PAGE_SIZE) - 1)); // translated address
            SQ.entry[sq_index].translated = COMPLETED;

            RTS1[RTS1_tail] = sq_index;
            RTS1_tail++;
            if (RTS1_tail == SQ_SIZE)
                RTS1_tail = 0;

            DP (if (warmup_complete[cpu]) {
            cout << "[ROB] " << __func__ << " RFO instr_id: " << SQ.entry[sq_index].instr_id;
            cout << " DTLB_FETCH_DONE translation: " << +SQ.entry[sq_index].translated << hex << " page: " << (SQ.entry[sq_index].physical_address>>LOG2_PAGE_SIZE);
            cout << " full_addr: " << SQ.entry[sq_index].physical_address << dec << " store_merged: " << +current_packet->store_merged;
            cout << " load_merged: " << +current_packet->load_merged << endl; }); 

            handle_merged_translation(current_packet);
        }
        else { 
            LQ.entry[lq_index].physical_address = (current_packet->data_pa << LOG2_PAGE_SIZE) | (LQ.entry[lq_index].virtual_address & ((1 << LOG2_PAGE_SIZE) - 1)); // translated address
            LQ.entry[lq_index].translated = COMPLETED;

            RTL1[RTL1_tail] = lq_index;
            RTL1_tail++;
            if (RTL1_tail == LQ_SIZE)
                RTL1_tail = 0;

            DP (if (warmup_complete[cpu]) {
            cout << "[RTL1] " << __func__ << " instr_id: " << LQ.entry[lq_index].instr_id << " rob_index: " << LQ.entry[lq_index].rob_index << " is added to RTL1";
            cout << " head: " << RTL1_head << " tail: " << RTL1_tail << endl; }); 

            DP (if (warmup_complete[cpu]) {
            cout << "[ROB] " << __func__ << " load instr_id: " << LQ.entry[lq_index].instr_id;
            cout << " DTLB_FETCH_DONE translation: " << +LQ.entry[lq_index].translated << hex << " page: " << (LQ.entry[lq_index].physical_address>>LOG2_PAGE_SIZE);
            cout << " full_addr: " << LQ.entry[lq_index].physical_address << dec << " store_merged: " << +current_packet->store_merged;
            cout << " load_merged: " << +current_packet->load_merged << endl; }); 

            handle_merged_translation(current_packet);
        }

        ROB.entry[rob_index].event_cycle = current_packet->event_cycle;
    }
    else { // L1D

        if (current_packet->type == RFO)
            handle_merged_load(current_packet);
        else { // do traditional things
#ifdef SANITY_CHECK
            if (rob_index != check_rob(current_packet->instr_id))
                assert(0);

            if (current_packet->store_merged)
                assert(0);
#endif
            LQ.entry[lq_index].fetched = COMPLETED;
            ROB.entry[rob_index].num_mem_ops--;

#ifdef SANITY_CHECK
            if (ROB.entry[rob_index].num_mem_ops < 0) {
                cerr << "instr_id: " << ROB.entry[rob_index].instr_id << endl;
                assert(0);
            }
#endif
            if (ROB.entry[rob_index].num_mem_ops == 0)
                inflight_mem_executions++;

            DP (if (warmup_complete[cpu]) {
            cout << "[ROB] " << __func__ << " load instr_id: " << LQ.entry[lq_index].instr_id;
            cout << " L1D_FETCH_DONE fetched: " << +LQ.entry[lq_index].fetched << hex << " address: " << (LQ.entry[lq_index].physical_address>>LOG2_BLOCK_SIZE);
            cout << " full_addr: " << LQ.entry[lq_index].physical_address << dec << " remain_mem_ops: " << ROB.entry[rob_index].num_mem_ops;
            cout << " load_merged: " << +current_packet->load_merged << " inflight_mem: " << inflight_mem_executions << endl; }); 

            release_load_queue(lq_index);

            handle_merged_load(current_packet);

            ROB.entry[rob_index].event_cycle = current_packet->event_cycle;
        }
    }
}

void O3_CPU::handle_merged_translation(PACKET *provider)
{
    if (provider->store_merged) {
	ITERATE_SET(merged, provider->sq_index_depend_on_me, SQ.SIZE) {
            SQ.entry[merged].translated = COMPLETED;
            SQ.entry[merged].physical_address = (provider->data_pa << LOG2_PAGE_SIZE) | (SQ.entry[merged].virtual_address & ((1 << LOG2_PAGE_SIZE) - 1)); // translated address
            SQ.entry[merged].event_cycle = current_core_cycle[cpu];

            RTS1[RTS1_tail] = merged;
            RTS1_tail++;
            if (RTS1_tail == SQ_SIZE)
                RTS1_tail = 0;

            DP (if (warmup_complete[cpu]) {
            cout << "[ROB] " << __func__ << " store instr_id: " << SQ.entry[merged].instr_id;
            cout << " DTLB_FETCH_DONE translation: " << +SQ.entry[merged].translated << hex << " page: " << (SQ.entry[merged].physical_address>>LOG2_PAGE_SIZE);
            cout << " full_addr: " << SQ.entry[merged].physical_address << dec << " by instr_id: " << +provider->instr_id << endl; });
        }
    }
    if (provider->load_merged) {
	ITERATE_SET(merged, provider->lq_index_depend_on_me, LQ.SIZE) {
            LQ.entry[merged].translated = COMPLETED;
            LQ.entry[merged].physical_address = (provider->data_pa << LOG2_PAGE_SIZE) | (LQ.entry[merged].virtual_address & ((1 << LOG2_PAGE_SIZE) - 1)); // translated address
            LQ.entry[merged].event_cycle = current_core_cycle[cpu];

            RTL1[RTL1_tail] = merged;
            RTL1_tail++;
            if (RTL1_tail == LQ_SIZE)
                RTL1_tail = 0;

            DP (if (warmup_complete[cpu]) {
            cout << "[RTL1] " << __func__ << " instr_id: " << LQ.entry[merged].instr_id << " rob_index: " << LQ.entry[merged].rob_index << " is added to RTL1";
            cout << " head: " << RTL1_head << " tail: " << RTL1_tail << endl; }); 

            DP (if (warmup_complete[cpu]) {
            cout << "[ROB] " << __func__ << " load instr_id: " << LQ.entry[merged].instr_id;
            cout << " DTLB_FETCH_DONE translation: " << +LQ.entry[merged].translated << hex << " page: " << (LQ.entry[merged].physical_address>>LOG2_PAGE_SIZE);
            cout << " full_addr: " << LQ.entry[merged].physical_address << dec << " by instr_id: " << +provider->instr_id << endl; });
        }
    }
}

void O3_CPU::handle_merged_load(PACKET *provider)
{
    ITERATE_SET(merged, provider->lq_index_depend_on_me, LQ.SIZE) {
        uint32_t merged_rob_index = LQ.entry[merged].rob_index;

        LQ.entry[merged].fetched = COMPLETED;
        LQ.entry[merged].event_cycle = current_core_cycle[cpu];
        ROB.entry[merged_rob_index].num_mem_ops--;
        ROB.entry[merged_rob_index].event_cycle = current_core_cycle[cpu];

#ifdef SANITY_CHECK
        if (ROB.entry[merged_rob_index].num_mem_ops < 0) {
            cerr << "instr_id: " << ROB.entry[merged_rob_index].instr_id << " rob_index: " << merged_rob_index << endl;
            assert(0);
        }
#endif

        if (ROB.entry[merged_rob_index].num_mem_ops == 0)
            inflight_mem_executions++;

        DP (if (warmup_complete[cpu]) {
        cout << "[ROB] " << __func__ << " load instr_id: " << LQ.entry[merged].instr_id;
        cout << " L1D_FETCH_DONE translation: " << +LQ.entry[merged].translated << hex << " address: " << (LQ.entry[merged].physical_address>>LOG2_BLOCK_SIZE);
        cout << " full_addr: " << LQ.entry[merged].physical_address << dec << " by instr_id: " << +provider->instr_id;
        cout << " remain_mem_ops: " << ROB.entry[merged_rob_index].num_mem_ops << endl; });

        release_load_queue(merged);
    }
}

void O3_CPU::release_load_queue(uint32_t lq_index)
{
    // release LQ entries
    DP ( if (warmup_complete[cpu]) {
    cout << "[LQ] " << __func__ << " instr_id: " << LQ.entry[lq_index].instr_id << " releases lq_index: " << lq_index;
    cout << hex << " full_addr: " << LQ.entry[lq_index].physical_address << dec << endl; });

    LSQ_ENTRY empty_entry;
    LQ.entry[lq_index] = empty_entry;
    LQ.occupancy--;
}

void O3_CPU::retire_rob()
{
    // Preventing clflush instruction to retire early, to compensate for the cache access that clflush would do.
    if(ROB.entry[ROB.head].clflush != 0 && ( current_core_cycle[cpu] - ROB.entry[ROB.head].retire_clflush <= 30) )
        return;

    for (uint32_t n=0; n<RETIRE_WIDTH; n++) {
        if (ROB.entry[ROB.head].ip == 0)
            return;

        // retire is in-order
        if (ROB.entry[ROB.head].executed != COMPLETED) { 
            DP ( if (warmup_complete[cpu]) {
            cout << "[ROB] " << __func__ << " instr_id: " << ROB.entry[ROB.head].instr_id << " head: " << ROB.head << " is not executed yet" << endl; });
            return;
        }

        // check store instruction
        uint32_t num_store = 0;
        for (uint32_t i=0; i<MAX_INSTR_DESTINATIONS; i++) {
            if (ROB.entry[ROB.head].destination_memory[i])
                num_store++;
        }

        if (num_store) {
            if ((L1D.WQ.occupancy + num_store) <= L1D.WQ.SIZE) {
                for (uint32_t i=0; i<MAX_INSTR_DESTINATIONS; i++) {
                    if (ROB.entry[ROB.head].destination_memory[i]) {

                        PACKET data_packet;
                        uint32_t sq_index = ROB.entry[ROB.head].sq_index[i];

                        // sq_index and rob_index are no longer available after retirement
                        // but we pass this information to avoid segmentation fault
                        data_packet.fill_level = FILL_L1;
                        data_packet.cpu = cpu;
                        data_packet.data_index = SQ.entry[sq_index].data_index;
                        data_packet.sq_index = sq_index;
                        data_packet.address = SQ.entry[sq_index].physical_address >> LOG2_BLOCK_SIZE;
                        data_packet.full_addr = SQ.entry[sq_index].physical_address;
                        data_packet.instr_id = SQ.entry[sq_index].instr_id;
                        data_packet.rob_index = SQ.entry[sq_index].rob_index;
                        data_packet.ip = SQ.entry[sq_index].ip;
                        data_packet.type = RFO;
                        data_packet.asid[0] = SQ.entry[sq_index].asid[0];
                        data_packet.asid[1] = SQ.entry[sq_index].asid[1];
                        data_packet.event_cycle = current_core_cycle[cpu];

                        L1D.add_wq(&data_packet);
                    }
                }
            }
            else {
                DP ( if (warmup_complete[cpu]) {
                cout << "[ROB] " << __func__ << " instr_id: " << ROB.entry[ROB.head].instr_id << " L1D WQ is full" << endl; });

                L1D.WQ.FULL++;
                L1D.STALL[RFO]++;

                return;
            }
        }

        // release SQ entries
        for (uint32_t i=0; i<MAX_INSTR_DESTINATIONS; i++) {
            if (ROB.entry[ROB.head].sq_index[i] != UINT32_MAX) {
                uint32_t sq_index = ROB.entry[ROB.head].sq_index[i];

                DP ( if (warmup_complete[cpu]) {
                cout << "[SQ] " << __func__ << " instr_id: " << ROB.entry[ROB.head].instr_id << " releases sq_index: " << sq_index;
                cout << hex << " address: " << (SQ.entry[sq_index].physical_address>>LOG2_BLOCK_SIZE);
                cout << " full_addr: " << SQ.entry[sq_index].physical_address << dec << endl; });

                LSQ_ENTRY empty_entry;
                SQ.entry[sq_index] = empty_entry;
                
                SQ.occupancy--;
                SQ.head++;
                if (SQ.head == SQ.SIZE)
                    SQ.head = 0;
            }
        }

        // release ROB entry
        DP ( if (warmup_complete[cpu]) {
        cout << "[ROB] " << __func__ << " instr_id: " << ROB.entry[ROB.head].instr_id << " is retired" << endl; });
        
				if(DETAILED_PRINT || (ROB.entry[ROB.head].instr_id >= range_start && ROB.entry[ROB.head].instr_id <= range_end))
				  cout <<" Retiring instr_id: " << ROB.entry[ROB.head].instr_id << " ip: "<< ROB.entry[ROB.head].ip<< " cpu: "<<cpu << " current_core_cycle[cpu]: "<< current_core_cycle[cpu]<< " fence: "<< ROB.entry[ROB.head].fence<< endl;			

				if(current_core_cycle[0] > 7878402 && ROB.entry[ROB.head].ip == 94187760952182)
				{ 
					stop_count++; 
					if(start_count != 0)
					{ cout << "distribution: "<<stop_count <<" " << current_core_cycle[0]-start_count << endl; 
					start_count = start_count_1 ;
					start_count_1 = 0;
					//cout << "start_count: " <<start_count <<" start_count_1: "<< start_count_1<< endl;
			 }
					else
					{assert(0);}
				}


        ooo_model_instr empty_entry;
        ROB.entry[ROB.head] = empty_entry;

        ROB.head++;
        if (ROB.head == ROB.SIZE)
            ROB.head = 0;
        ROB.occupancy--;
        completed_executions--;
        num_retired++;
    }
}
