`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   18:30:50 08/24/2024
// Design Name:   controller
// Module Name:   /home/ise/Controlller/controller_top.v
// Project Name:  Controlller
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: controller
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////
`include "macros.v"

module top;
    
	// Inputs
	reg clk;

	wire [`STR_LEN-1:0] out_string;
	wire opcodeR;
	wire [`ADDR_LEN-1:0] addrR;
	wire validR;	

	reg [`STR_LEN-1:0] in_string;
	wire opcodeS;
	wire [`ADDR_LEN-1:0] addrS;
	wire validS;	

	// Outputs
	wire [`NUM_BLOCKS-1:0] hit;
	wire [`TAG_LEN -1 : 0] hitrow;
	wire [`LOG_NB-1 : 0] replace_index;
	wire [`LOG_NB-1 : 0] hit_index;
	wire stall;
	wire retvalR;
	wire retvalS;

	// Instantiate controller
	controller uut (
		.clk(clk), 
		.opcodeR(opcodeR),
		.addrR(addrR),
		.validR(validR),
		.opcodeS(opcodeS),
		.addrS(addrS),
		.validS(validS),
		.hit(hit),
		.hitrow(hitrow),
		.replace_index(replace_index),
		.hit_index(hit_index),
		.retvalR(retvalR),
		.retvalS(retvalS),
		.stall(stall)
	);

	// Instantiate sender
	sender uutS (
		.string(in_string),
		.clk(clk),
		.retvalS(retvalS),
		.validS(validS),
		.opcodeS(opcodeS),
		.addrS(addrS)
	);

//	 Instantiate receiver
	receiver uutR (
		.string(out_string),
		.clk(clk),
		.retvalR(retvalR),
		.validR(validR),
		.opcodeR(opcodeR),
		.addrR(addrR)
	);

//	always @(negedge clk) begin
//		 $display("In top : time = %d, validR = %b, opcodeR = %d, addrR = %d, validS = %b, opcodeS = %d, addrS = %d, hit = %b, hitrow = %d, retvalR = %b, retvalS = %b, replace_index = %d, hit_index = %d, stall = %b", $time, validR, opcodeR, addrR, validS, opcodeS, addrS, hit, hitrow, retvalR, retvalS, replace_index, hit_index, stall);
//	end
	always @(posedge clk) begin
		$display("In top        : time = %d, in_string = %b, out_string = %b, validR = %b, validS = %b, addrR = %d, addrS = %d, hit = %b, hitrow = %d, retvalR = %b, retvalS = %b, replace_index = %d, hit_index = %d, stall = %b", $time, in_string, out_string, validR, validS, addrR, addrS, hit, hitrow, retvalR, retvalS, replace_index, hit_index, stall);
	end
	 
	integer i;
	initial begin
		in_string = 16'b0101101000101001;
//		for(i = 0; i < `STR_LEN; i = i+1) begin
//			in_string[i] = i&1;
//		end
		#10000;
		$finish;
	end
	
	initial begin
		// Simulate clk
		forever begin
			clk=0;
			#5
			clk=1;
			#5
			clk=0;
		end        
	end
      
endmodule