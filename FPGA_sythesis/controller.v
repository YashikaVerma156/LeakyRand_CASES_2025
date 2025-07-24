`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    14:49:23 10/01/2024 
// Design Name: 
// Module Name:    controller 
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
module controller(clk, opcodeR, addrR, validR, opcodeS, addrS, validS, retvalR, retvalS);
    
	// input signals from environment
	input clk;

	// input signals from receiver
	input opcodeR;
	input [`TAG_LEN-1:0] addrR;
	input validR;

	// input signals from sender
	input opcodeS;
	input [`TAG_LEN-1:0] addrS;
	input validS;
	
	// output signals
	output wire retvalR;
	output wire retvalS;
	
	// Internal variables used by controller
	wire [1:0] opcode;
	wire [`LOG_NB-1 : 0] replace_index_out;
	wire [`TAG_LEN-1:0] addr;
	reg [`TAG_LEN-1:0] tag;
	reg [1:0] prev_opcode = 0;
    reg [`NUM_BLOCKS-1:0] hit = 8'b00000001;
	reg stall = 0;
    reg reset_done = 0;
    
    //Cache
	reg [`TAG_LEN : 0] cache [0 : `NUM_BLOCKS-1];
	
	// Invalid tracker variables
	reg [`LOG_NB-1 : 0] invalid_index [0 : `NUM_BLOCKS-1];
	reg [`LOG_NB-1 : 0] invalid_index_RP = 0;
	reg [`LOG_NB-1 : 0] invalid_index_WP = 0;
	reg [`LOG_NB : 0] invalid_count = `NUM_BLOCKS;
	
	wire [`LOG_NB-1 : 0] hit_index_out [0 : `NUM_BLOCKS];

	// Instantiate random number generator
	wire [`LOG_NB-1:0] temprand;
	random_gen random_(clk, temprand);

	integer i;
    
	assign opcode = (validR ? {1'b0, opcodeR} : (validS ? {1'b0, opcodeS} : `NOP));
	assign addr = (validR ? addrR : (validS ? addrS : 0));
	assign replace_index_out = ((invalid_count == 0) ? temprand : invalid_index[invalid_index_RP]);
	assign retvalR = (((stall == 1) || (validR == 0) || ((hit == 0) && (opcode == 0)))? 1'b0 : 1'b1);
	assign retvalS = (((stall == 1) || (validS == 0) || ((hit == 0) && (opcode == 0)))? 1'b0 : 1'b1);

	assign hit_index_out[0] = 0;
	genvar j;
	generate 
		for (j=0; j<`NUM_BLOCKS; j=j+1) begin:gen_block
			assign hit_index_out[j+1] = ((hit[j] == 0) ? hit_index_out[j] : j);
		end
	endgenerate
	
	always @(posedge clk) begin
		if(reset_done == 0) begin //initialisation of cache
			for(i=0; i<`NUM_BLOCKS; i = i+1) begin
				cache[i] <= (1<<`TAG_LEN) + ((`MAX_TAG+1)>>1) + i;
				invalid_count <= 0;
			end
			reset_done <= 1;
		end
		else begin //serving a cache request
			stall <= `PROP_DELAY (((((hit == 0) && (prev_opcode == 0)) || ((hit != 0) && (prev_opcode == 1))) && (tag == addr) && (stall == 0)) ? 1'b1 : 1'b0);
			if((prev_opcode == 1) && (hit != 0) && (stall == 0)) begin // flush
				cache[hit_index_out[`NUM_BLOCKS]] <= `PROP_DELAY 0;
				invalid_index[invalid_index_WP] <= `PROP_DELAY hit_index_out[`NUM_BLOCKS];
				invalid_index_WP <= `PROP_DELAY ((invalid_index_WP + 1) & (`NUM_BLOCKS - 1));
				invalid_count <= `PROP_DELAY (invalid_count + 1);
			end
			else if((prev_opcode == 0) && (hit == 0) && (stall == 0)) begin   // replace for miss
				invalid_index_RP <= `PROP_DELAY ((invalid_count == 0) ? invalid_index_RP : ((invalid_index_RP+1) & (`NUM_BLOCKS-1)));
				invalid_count <= `PROP_DELAY ((invalid_count == 0) ? invalid_count : (invalid_count - 1));
				cache[replace_index_out] <= `PROP_DELAY {1'b1, tag};
			end
			tag <= `PROP_DELAY addr;
			for (i = 0; i < `NUM_BLOCKS; i = i+1) begin
				hit[i] <= `PROP_DELAY ((opcode != `NOP) & cache[i][`TAG_LEN] & (addr == cache[i][`TAG_LEN-1 : 0]) ? 1'b1 : 1'b0);
			end
			prev_opcode <= `PROP_DELAY opcode;
		end
	end
	
	always @(posedge clk) begin
		$display("In ctrl : time = %d, cache[0] = %d, cache[1] = %d, cache[2] = %d, cache[3] = %d, cache[4] = %d, cache[5] = %d, cache[6] = %d, cache[7] = %d", $time, cache[0], cache[1], cache[2], cache[3], cache[4], cache[5], cache[6], cache[7]);
		$display("In ctrl : time = %d, cache[8] = %d, cache[9] = %d, cache[10] = %d, cache[11] = %d, cache[12] = %d, cache[13] = %d, cache[14] = %d, cache[15] = %d", $time, cache[8], cache[9], cache[10], cache[11], cache[12], cache[13], cache[14], cache[15]);
	end
	
endmodule 