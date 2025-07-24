`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    14:20:12 10/01/2024 
// Design Name: 
// Module Name:    sender 
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
module sender(input_ready, string, clk, retvalS, validS, opcodeS, addrS, eos, eci, sndr_probe_blocks, total_cycles, recv_wait_cycles, total_ed_cycles, crf_total_cycles, crp_total_cycles);
    
	// input signals from the environment
	input [`STR_LEN-1:0] string;
	input clk;
	input [`LOG_STR_LEN-1:0] eos;
	
	// input signal from input interface
	input input_ready;
    input [`INPUT_SIZE3-1:0] eci;
    input [`INPUT_SIZE1-1:0] sndr_probe_blocks;
    
    input [`COUNTER_SIZE-1:0] total_cycles;
    input [`COUNTER_SIZE-1:0] recv_wait_cycles;
    input [`COUNTER_SIZE-1:0] crf_total_cycles;
    input [`COUNTER_SIZE-1:0] crp_total_cycles;
    input [`COUNTER_SIZE-1:0] total_ed_cycles;
    
	// input signal from controller
	input retvalS;

	// output signals to controller
	output reg validS = 0;
	output wire [`TAG_LEN-1 : 0] addrS;
	output reg opcodeS = 0;

	// Internal variables used by sender
	reg [`COUNTER_SIZE-1:0] count = 0; // keeps track of number of cycles
	reg [`COUNTER_SIZE-1:0] crf_count = 0; // keeps track of number of cycles in cr fill
	reg [`LOG_STR_LEN : 0] index = 0; // keeps track of which bit of string we send
	reg [`TAG_LEN-1 : 0] tagS = 0;
	reg state = 0;
	reg first_block = 1;
	reg [1:0] flag = 0;
	reg [`COUNTER_SIZE-1:0] ed_count = 0; //error detection cycles counter
    reg [`LOG_STR_LEN:0] eci_counter = 0; //error correction interval counter
	reg init = 0;
    
	assign addrS = tagS;

//	always @(posedge clk) begin
//		$display("In sender 	 : time = %d, flag = %b, crf_count = %d, count = %d, validS = %d, opcodeS = %d, tagS = %d, retvalS = %d, index = %d, string[index] = %d, state = %b, first_block = %d", $time, flag, crf_count, count, validS, opcodeS, tagS, retvalS, index, string[index], state, first_block);
//	end
	
	always @(posedge clk) begin
		$display("Sndr 1 : time = %d, flag = %d, count = %d, ed_count = %d, state = %b, tagS = %d, validS = %b, opcodeS = %b,  retvalS = %b, index = %d", $time, flag, count, ed_count, state, tagS, validS, opcodeS, retvalS, index);
	end

	
	reg reset_done = 0;
	always @(posedge clk) begin
		if(reset_done == 0) begin
			reset_done <= 1;
		end
		else if(input_ready == 1) begin
			if(init == 0) begin //initialisation
				index <= {1'b0,eos[`LOG_STR_LEN - 1 : 0]};
				init <= 1;
			end
			else begin
				if(flag == 1) begin //communication
					if(string[index] == 1) begin
						state <= `PROP_DELAY ( (retvalS == 1) && (state == 0))? 1 : 0;
						opcodeS <= `PROP_DELAY ( (count < (`MISS_LATENCY * sndr_probe_blocks)-1) || (count == total_cycles-1) || (count == recv_wait_cycles -1) ? 0 : 1 );
						tagS <= `PROP_DELAY ( ( (count >= recv_wait_cycles-1) || (count == (`MISS_LATENCY * sndr_probe_blocks) -1) || (count ==total_cycles-1) || (first_block)) ? 0 : ( (state == 0 && retvalS == 1) ? tagS+1 : tagS ) );
					end
					count <= `PROP_DELAY (count == total_cycles-1) ? 0 : count + 1;
					index <= `PROP_DELAY ((index < `STR_LEN) && (count == total_cycles-1)) ? index+1 : index;
					first_block <= `PROP_DELAY (count == total_cycles-1) ? 1:0;
                    validS <= `PROP_DELAY ((string[index]==1 && count < recv_wait_cycles - 1) || ((string[index+1]==1 || eci_counter == eci-1) && count == total_cycles-1)) ? 1 : 0;
//					validS <= `PROP_DELAY (string[index]==1 && count < recv_wait_cycles - 1) ? 1 : ((((eci_counter != eci-1 && string[index+1]==1) || eci_counter == eci-1) && count == total_cycles-1) ? 1 : 0);
                    flag <= (index <= `STR_LEN-1) ? ((eci_counter == eci-1 && count == total_cycles-1) ? 2 : flag) : 0;
                    eci_counter <= `PROP_DELAY (count == total_cycles-1) ? ((eci_counter == eci-1) ?0 : eci_counter+1) : eci_counter;
				end
				else if(flag == 2) begin  //Error detection & correction
					state <= `PROP_DELAY ( (retvalS == 1) && (state == 0))? 1 : 0;
					opcodeS <= `PROP_DELAY ( (ed_count < (`MISS_LATENCY * sndr_probe_blocks)-1) || (ed_count == total_ed_cycles-1) || (ed_count == recv_wait_cycles -1) ? 0 : 1 );
					tagS <= `PROP_DELAY ( ( (ed_count >= recv_wait_cycles-1) || (ed_count == (`MISS_LATENCY * sndr_probe_blocks) -1) || (ed_count ==total_ed_cycles-1) || (first_block)) ? 0 : ( (state == 0 && retvalS == 1) ? tagS+1 : tagS ) );
					ed_count <= `PROP_DELAY (ed_count == total_ed_cycles-1) ? 0 : ed_count + 1;
					first_block <= `PROP_DELAY (ed_count == total_ed_cycles-1) ? 1:0;
					validS <= `PROP_DELAY ((ed_count < recv_wait_cycles - 1) || (ed_count == total_ed_cycles - 1 && string[index]==1)) ? 1 : 0;
					flag <= (ed_count == total_ed_cycles -1) ? 1 : flag;
				end
				else begin //inital wait for CRFill and CRProbe steps 
					crf_count <= `PROP_DELAY (crf_count == crf_total_cycles + crp_total_cycles +1) ? 0 : crf_count +1;
					flag <= `PROP_DELAY (crf_count == crf_total_cycles + crp_total_cycles +1) ? 1 : 0;
					validS <= `PROP_DELAY (crf_count == crf_total_cycles + crp_total_cycles +1 && string[index] == 1) ? 1 : 0;
				end
			end
		end
	end
	
endmodule 
