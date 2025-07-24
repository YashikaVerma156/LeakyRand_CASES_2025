`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    14:23:00 10/01/2024 
// Design Name: 
// Module Name:    receiver 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
`include "macros.v"
module receiver(input_ready, string_out, clk, retvalR, validR, opcodeR, addrR, outstring_ready, total_cycle_counter, eos, occ_final, occ_init,
				eci, sndr_probe_blocks, crp_blocks, crf_blocks0, crf_blocks1, crf_blocks2, exp0, exp1, exp2, 
				recv_wait_cycles, total_cycles, crf_cycles0, crf_cycles1, crf_cycles2, crf_total_cycles, crp_cycles0, crp_cycles1, total_ed_cycles 
                );

	// input signals from the environment
	input clk;
	input [`LOG_STR_LEN-1:0] eos;
	
	// input signal from input interface
	input input_ready;
	input [`INPUT_SIZE3-1:0] eci;
	input [`INPUT_SIZE1-1:0] sndr_probe_blocks;
	input [`INPUT_SIZE2-1:0] crp_blocks;

	input [`INPUT_SIZE2-1:0] crf_blocks0;
	input [`INPUT_SIZE2-1:0] crf_blocks1;
	input [`INPUT_SIZE2-1:0] crf_blocks2;

	input [`INPUT_SIZE1-1:0] exp0;
	input [`INPUT_SIZE1-1:0] exp1;
	input [`INPUT_SIZE1-1:0] exp2;
	
	input [`COUNTER_SIZE-1:0] recv_wait_cycles;
	input [`COUNTER_SIZE-1:0] total_cycles;

	input [`COUNTER_SIZE-1:0] crf_cycles0;
	input [`COUNTER_SIZE-1:0] crf_cycles1;
	input [`COUNTER_SIZE-1:0] crf_cycles2;
	
	input [`COUNTER_SIZE-1:0] crf_total_cycles;
	input [`COUNTER_SIZE-1:0] crp_cycles0;
	input [`COUNTER_SIZE-1:0] crp_cycles1;
	input [`COUNTER_SIZE-1:0] total_ed_cycles;
	
	// input signal from controller
	input retvalR;

	// output signals to controller
	output reg validR = 0;
	output wire [`TAG_LEN-1 : 0] addrR;
	output reg opcodeR = 0;
	
	// output signals to environment
	output wire [`STR_LEN-1:0] string_out;
	output reg [`COUNTER_SIZE-1:0] total_cycle_counter = 0; //counts total number of cycles spent in communication
	output wire [`LOG_NB:0] occ_final;
	output reg [`LOG_NB:0] occ_init = 0;
	
	// output signal to bin_to_dec
	output reg outstring_ready = 0;
	
	// Internal variables used by receiver
	reg [`STR_LEN-1:0] string = 0;
	reg [`COUNTER_SIZE-1 : 0] count= 0;
	reg [`COUNTER_SIZE-1 : 0] cycle_counter = 0;
	reg [`COUNTER_SIZE-1 : 0] crf_count = 0;
	reg [`COUNTER_SIZE-1 : 0] crf_total_count = 0;
	reg [`COUNTER_SIZE-1 : 0] crp_count = 0;
	reg [`COUNTER_SIZE-1 : 0] crp_cycle_counter = 0;
	reg [`LOG_STR_LEN : 0] index = 0;
	reg [`TAG_LEN-1 : 0] tagR = `MAX_TAG;	
	reg [1:0] flag = 0;
	reg state = 0;
	reg first_block = 1;
	reg rem = 1;
	reg init = 0;
	reg [2:0] crf_state = 0;
	reg [1:0] crp_state = 0;
	reg [4:0] exp = 0;
	reg [`LOG_NB:0] crp_hits = 0;
	reg ed_state = 0;
	reg [`LOG_NB:0] num_misses = 0; 
	reg [`COUNTER_SIZE-1:0] ed_count = 0;
	reg [`LOG_STR_LEN:0] eci_counter = 0;
	reg [`LOG_NB-1:0] ed_iter = 0;
	reg [1:0] ec_state = 0;
	reg miss_detected = 0;
	reg miss_flag = 0;
    reg reset_done = 0;
	
	wire [`TAG_LEN-1 : 0] start_tag0;
	wire [`TAG_LEN-1 : 0] end_tag0;
	wire [`TAG_LEN-1 : 0] start_tag1;
	wire [`TAG_LEN-1 : 0] end_tag1;
	wire [`TAG_LEN-1 : 0] start_tag2;
	wire [`TAG_LEN-1 : 0] end_tag2;
	
	assign start_tag0 = `MAX_TAG;
	assign end_tag0 = start_tag0 - crf_blocks0 + 1;
	assign start_tag1 = `MAX_TAG;
	assign end_tag1 = start_tag1 - crf_blocks1 + 1;
	assign start_tag2 = `MAX_TAG;
	assign end_tag2 = start_tag2 - crf_blocks2 + 1;

	assign addrR = tagR;
	assign string_out = string;
	assign occ_final = crp_hits;
	
//    always @(posedge clk) begin
//		$display("In receiver : time = %d, inpReady = %b, outstrReady = %b, flag = %d, crf_state = %d, crf_count = %d, exp = %d, count = %d, crp_count = %d, crp_state = %d, tagR = %d, 1stBlock = %b, state = %b, retvalR = %b, validR = %b, crpHits = %d, crpCycleCtr = %d", $time, input_ready, outstring_ready, flag, crf_state, crf_count, exp, count, crp_count, crp_state, tagR, first_block, state, retvalR, validR, crp_hits, crp_cycle_counter);
//	end
	
//	always @(posedge clk) begin
//		$display("In receiver    : time = %d, total_cycles = %d, input_ready = %b, outstring_ready = %b, init = %b, reset_done = %b, rem = %b, flag = %d, crf_state = %d, crf_count = %d, exp = %d, count = %d, crp_count = %d, crp_state = %d, tagR = %d, first_block = %b, state = %b, retvalR = %b, validR = %b, crp_hits = %d, crp_cycle_counter = %d\n", $time, total_cycles, input_ready, outstring_ready, init, reset_done, rem, flag, crf_state, crf_count, exp, count, crp_count, crp_state, tagR, first_block, state, retvalR, validR, crp_hits, crp_cycle_counter);
//	end

	always @(posedge clk) begin
		$display("Recv 1 : time = %d, crf_count = %d, crf_state = %d, crp_state = %d, crp_count = %d, flag = %d, count = %d, ed_count = %d, ed_state = %b, miss_detected = %b, exp = %d, eci = %d", $time, crf_count, crf_state, crp_state, crp_count, flag, count, ed_count, ed_state, miss_detected, exp, eci);
		$display("Recv 2 : time = %d, state = %b, tagR = %d, validR = %b, opcodeR = %b,  retvalR = %b, index = %d, occ_final = %d, num_misses = %d, cycle_counter = %d, ed_iter = %d, ec_state = %d, miss_flag = %b\n", $time, state, tagR, validR, opcodeR, retvalR, index, occ_final, num_misses, cycle_counter, ed_iter, ec_state, miss_flag);
	end
	
	always @(posedge clk) begin
		if (input_ready == 1 && rem == 0) begin //marks end of communication
			outstring_ready <= 1;
		end
		else if(reset_done == 0) begin 
			reset_done <= 1;
		end
		else if(input_ready == 1 && rem == 1) begin
			if(init == 0) begin //initialisation
				index <= {1'b0,eos};
				init <= 1;
				validR <= 1;
			end
			else begin 
				total_cycle_counter <= `PROP_DELAY total_cycle_counter + 1;
				if(flag == 0) begin  //CRFill
					crf_total_count <= crf_total_count + 1;
					if(crf_state == 0 && exp < exp0) begin
						state <= `PROP_DELAY ( (retvalR == 1) && (state == 0))? 1 : 0;
						tagR <= `PROP_DELAY (crf_count ==crf_cycles0-1) ? ((exp == exp0-1) ? start_tag1 : start_tag0) : ( (state == 0 && retvalR == 1 && tagR > end_tag0) ? tagR-1 : tagR ) ;						
						crf_count <= `PROP_DELAY (crf_count == crf_cycles0-1) ? 0 : crf_count + 1;
						exp <= (crf_count == crf_cycles0-1) ? ((exp < exp0 - 1) ? exp+1 : 0) : exp;
						crf_state <= (crf_count == crf_cycles0 - 1 && exp == exp0 - 1) ? 1 : 0;
					end
					else if(crf_state == 1 && exp < exp1) begin
						state <= `PROP_DELAY ( (retvalR == 1) && (state == 0))? 1 : 0;
						tagR <= `PROP_DELAY (crf_count ==crf_cycles1-1) ? ((exp == exp1-1) ? start_tag2 : start_tag1) : ( (state == 0 && retvalR == 1 && tagR > end_tag1) ? tagR-1 : tagR ) ;
						crf_count <= `PROP_DELAY (crf_count == crf_cycles1-1) ? 0 : crf_count + 1;	
						exp <= (crf_count == crf_cycles1-1) ? ((exp < exp1 - 1) ? exp+1 : 0) : exp;
						crf_state <= (crf_count == crf_cycles1 - 1 && exp == exp1 - 1) ? 2 : 1;
					end
					else if(crf_state == 2  && exp < exp2) begin
						state <= `PROP_DELAY ( (retvalR == 1) && (state == 0))? 1 : 0;
						tagR <= `PROP_DELAY (crf_count ==crf_cycles2-1) ? ((exp == exp2-1) ? end_tag2-1 : start_tag2) : ( (state == 0 && retvalR == 1 && tagR > end_tag2) ? tagR-1 : tagR ) ;
						crf_count <= `PROP_DELAY (crf_count == crf_cycles2-1) ? 0 : crf_count + 1;	
						exp <= (crf_count == crf_cycles2-1) ? ((exp < exp2 - 1) ? exp+1 : 0) : exp;
						crf_state <= (crf_count == crf_cycles2 - 1 && exp == exp2 - 1) ? 3 : 2;
					end
//					else if(crf_state == 3 && exp < exp3) begin
//						state <= `PROP_DELAY ( (retvalR == 1) && (state == 0))? 1 : 0;
//						tagR <= `PROP_DELAY (crf_count ==crf_cycles3-1) ? ((exp == exp3-1) ? start_tag4 : start_tag3) : ( (state == 0 && retvalR == 1 && tagR > end_tag3) ? tagR-1 : tagR ) ;
//						crf_count <= `PROP_DELAY (crf_count == crf_cycles3-1) ? 0 : crf_count + 1;	
//						exp <= (crf_count == crf_cycles3-1) ? ((exp < exp3 - 1) ? exp+1 : 0) : exp;
//						crf_state <= (crf_count == crf_cycles3 - 1 && exp == exp3 - 1) ? 4 : 3;
//					end
//					else if(crf_state == 4 && exp < exp4) begin
//						state <= `PROP_DELAY ( (retvalR == 1) && (state == 0))? 1 : 0;
//						tagR <= `PROP_DELAY (crf_count ==crf_cycles4-1)? ((exp == exp4-1) ? end_tag4-1 : start_tag4) : ( (state == 0 && retvalR == 1 && tagR > end_tag4) ? tagR-1 : tagR ) ;
//						crf_count <= `PROP_DELAY (crf_count == crf_cycles4-1) ? 0 : crf_count + 1;	
//						exp <= (crf_count == crf_cycles4-1) ? ((exp < exp4 - 1) ? exp+1 : 0) : exp;
//						crf_state <= (crf_count == crf_cycles4 - 1 && exp == exp4 - 1) ? 5 : 4;						
//					end
					flag <= `PROP_DELAY (crf_total_count == crf_total_cycles -1) ? 2'd1 :2'd0;
					validR <= (crf_total_count == crf_total_cycles -1) ? 0 : validR;
				end
                else if(flag == 1) begin // CRProbe
                    if(crp_state == 0) begin //initialisation 1
                        crp_state <= 1;
                        tagR <= `MAX_TAG - crp_blocks + 1;
						validR <= 1;
                    end
                    else 
					if(crp_state == 1) begin  // access-flush
                        opcodeR <= (state == 0 && retvalR == 1) ? (~opcodeR) : (opcodeR);
                        state <= `PROP_DELAY ( (retvalR == 1) && (state == 0))? 1 : 0;
                        crp_count <= `PROP_DELAY (crp_count == crp_cycles0-1) ? 0 : crp_count + 1;
                        crp_state <= (crp_count == crp_cycles0-1) ? 2:1;
                        tagR <= (tagR == `MAX_TAG) ? `MAX_TAG  : ((state == 0 && retvalR == 1 && opcodeR == 1) ? (tagR+1) : tagR); 
                        validR <= (tagR == `MAX_TAG && state == 0 && retvalR == 1 && opcodeR == 1) ? 0:validR;
                        crp_cycle_counter <= `PROP_DELAY ((retvalR == 0 || crp_cycle_counter == 0) && validR == 1 && opcodeR == 0) ? (crp_cycle_counter + 1) : 0;
                        crp_hits <= (crp_cycle_counter == `HIT_LATENCY-1 && retvalR == 1) ? (crp_hits +1) : crp_hits;
						occ_init <= (crp_cycle_counter == `HIT_LATENCY-1 && retvalR == 1) ? (occ_init +1) : occ_init;
                    end 
                    else if (crp_state == 2) begin //initialisation 2
                        tagR <= `MAX_TAG - crp_hits+1;
                        crp_state <= 3;
                        validR <= 1;
                        opcodeR <= 0;
                    end
                    else if (crp_state == 3) begin //only accesses
                        validR <= (tagR == `MAX_TAG && state == 0 && retvalR == 1) ? 0:validR;
                        tagR <= (tagR == `MAX_TAG) ? `MAX_TAG : ((state == 0 && retvalR == 1) ? (tagR+1) : tagR);
                        crp_count <= `PROP_DELAY (crp_count == crp_cycles1-1) ? 0 : crp_count + 1;
                        state <= `PROP_DELAY ( (retvalR == 1) && (state == 0))? 1 : 0;
                        flag <= (crp_count == crp_cycles1-1) ? 2:1;
                        crp_state <= (crp_count == crp_cycles1-1) ? 4 : 3;
                    end
                end
				else if(flag == 2) begin // communication
                    eci_counter <= `PROP_DELAY (count == total_cycles-1) ? ((eci_counter == eci-1) ?0 : eci_counter+1) : eci_counter;
					first_block <= `PROP_DELAY ((count == recv_wait_cycles -1) ? 1:0);
					state <= `PROP_DELAY ( (retvalR == 1) && (state == 0))? 1 : 0;
					tagR <= `PROP_DELAY ( ( (count < recv_wait_cycles -1) || (count ==total_cycles-1) || (first_block)) ? `MAX_TAG : ( (state == 0 && retvalR == 1 && (tagR != `MAX_TAG - crp_hits)) ? tagR-1 : tagR ) );
					count <= `PROP_DELAY (count == total_cycles-1) ? 0 : count + 1;
					index <= `PROP_DELAY (count == total_cycles-1) ? index+1 : index;
					rem <= `PROP_DELAY (index == `STR_LEN-1 && count == total_cycles-1) ? 0 : 1;
					cycle_counter <= `PROP_DELAY ((retvalR == 0 || cycle_counter == 0) && validR == 1) ? (cycle_counter + 1) : 0;
					string[index] <= `PROP_DELAY (cycle_counter == `MISS_LATENCY-1) ? 1 : (0 | string[index]);
                    validR <= ((count < recv_wait_cycles -1) || ((tagR == `MAX_TAG - crp_hits +1) && (state == 0 && retvalR == 1)) || (tagR < `MAX_TAG - crp_hits +1) || (count == total_cycles-1)) ? 0 : 1;
					flag <= (index <= `STR_LEN-1) ? ((eci_counter == eci-1 && count == total_cycles-1) ? 3 : flag) : 0;
				end 
				else begin //error detection and correction
					ed_count <= `PROP_DELAY (ed_count == total_ed_cycles-1) ? 0 : ed_count + 1;
					flag <= (ed_count == total_ed_cycles-1) ? 2 : flag;
					ed_state <= (ed_count < total_cycles-1 || ed_count == total_ed_cycles-1) ? 0:1;
					if(ed_state == 0) begin //error detection
						first_block <= `PROP_DELAY ((ed_count == recv_wait_cycles -1) ? 1:0);
						state <= `PROP_DELAY ( (retvalR == 1) && (state == 0))? 1 : 0;
						tagR <= (ed_count < recv_wait_cycles -1 || ed_count == total_ed_cycles-1 || first_block) ? `MAX_TAG : ((ed_count == total_cycles-1 && (num_misses < sndr_probe_blocks-1 || cycle_counter == `MISS_LATENCY-1)) ? `MAX_TAG - crp_hits : ((state == 0 && retvalR == 1 && tagR != `MAX_TAG - crp_hits) ? tagR-1 : tagR));
						validR <= (ed_count < recv_wait_cycles - 1) ? 0 : ((tagR > `MAX_TAG - crp_hits+1 || (tagR == `MAX_TAG - crp_hits +1 && (state == 1 || retvalR == 0))) ? 1 : ((ed_count < total_cycles-1) ? 0 : ((num_misses == sndr_probe_blocks || (num_misses == sndr_probe_blocks-1 && cycle_counter == `MISS_LATENCY-1)) ? 0 : 1)));
						cycle_counter <= `PROP_DELAY ((retvalR == 0 || cycle_counter == 0) && validR == 1) ? (cycle_counter + 1) : 0;
						num_misses <= (cycle_counter == `MISS_LATENCY-1) ? (num_misses+1) : num_misses;
					end
					else begin 
						if(ed_iter < sndr_probe_blocks && num_misses < sndr_probe_blocks && miss_flag == 0) begin
							if(ec_state == 0) begin //access
								state <= `PROP_DELAY ( (retvalR == 1) && (state == 0))? 1 : 0;
								opcodeR <= (state == 0 && retvalR == 1) ? (~opcodeR) : (opcodeR);
								ec_state <= (state == 0 && retvalR == 1) ? 1 : ec_state;
							end
							else if(ec_state == 1) begin //flush
								state <= `PROP_DELAY ( (retvalR == 1) && (state == 0))? 1 : 0;
								opcodeR <= (state == 0 && retvalR == 1) ? (~opcodeR) : (opcodeR);
								ec_state <= (state == 0 && retvalR == 1) ? 2 : ec_state;
								tagR <= (state == 0 && retvalR == 1) ? `MAX_TAG : tagR;
							end
							else if(ec_state == 2) begin 
								state <= `PROP_DELAY ( (retvalR == 1) && (state == 0))? 1 : 0;
								tagR <= (ed_count == total_ed_cycles -1) ? `MAX_TAG : ((state == 0 && retvalR == 1 && tagR > `MAX_TAG - crp_hits) ? tagR-1 : tagR);
								cycle_counter <= ((retvalR == 0 || cycle_counter == 0) && validR == 1) ? (cycle_counter + 1) : 0;
								miss_detected <= ((miss_detected == 1 && (tagR > `MAX_TAG - crp_hits + 1 || (tagR == `MAX_TAG - crp_hits + 1 && (state == 1 || retvalR == 0)))) || (cycle_counter == `MISS_LATENCY-1 && tagR > `MAX_TAG - crp_hits + 1)) ? 1 : 0;
								miss_flag <= (tagR == `MAX_TAG - crp_hits + 1 && state == 0 && retvalR == 1 && (miss_detected == 1 || cycle_counter == `MISS_LATENCY-1)) ? 1 : 0; 
								ec_state <= (tagR == `MAX_TAG - crp_hits + 1 && state == 0 && retvalR == 1) ? ((miss_detected == 1 || cycle_counter == `MISS_LATENCY-1) ? 0 : 3) : 2;
								validR <= (validR == 0 || (tagR == `MAX_TAG - crp_hits + 1 && state == 0 && retvalR == 1 && (miss_detected == 1 || cycle_counter == `MISS_LATENCY-1))) ? 0 : 1;
								ed_iter <= (tagR == `MAX_TAG - crp_hits + 1 && state == 0 && retvalR == 1 && (miss_detected == 1 || cycle_counter == `MISS_LATENCY-1)) ? (ed_count == total_ed_cycles-1 ? 0 : ed_iter+1) : ed_iter;
							end
							else if(ec_state == 3) begin // access
								state <= (state == 0 && retvalR == 1)? 1 : 0;
								crp_hits <= (state == 0 && retvalR == 1) ? crp_hits + 1 : crp_hits;
								validR <= (ed_count == total_ed_cycles-1 || (ed_iter == sndr_probe_blocks-1 && state == 0 && retvalR == 1)) ? 0 : validR;
								ed_iter <= (state == 0 && retvalR == 1) ? (ed_count == total_ed_cycles-1 ? 0 : ed_iter+1) : ed_iter;
								ec_state <= (state == 0 && retvalR == 1) ? 0 : ec_state;
								tagR <= (state == 0 && retvalR == 1) ? tagR - 1: tagR;
							end
						end
						else begin 
							validR <= 0;
							tagR <= `MAX_TAG;
							ed_iter <= (ed_count == total_ed_cycles-1) ? 0 : ed_iter;
							num_misses <= 0;
							miss_flag <= (ed_count == total_ed_cycles-1) ? 0 : miss_flag;
						end
					end
				end
			end
		end
	end
		
endmodule 
