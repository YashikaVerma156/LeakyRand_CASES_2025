`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    10:37:08 08/18/2024 
// Design Name: 
// Module Name:    random_gen 
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
`define PROP_DELAY #1
`include "macros.v"

module random_gen(clk, randout);
	input clk;
	reg[15:0] randnum = 16'b1010110011100001;
	output wire [`LOG_NB-1:0] randout;
	assign randout = randnum[`LOG_NB-1:0];
	always @(posedge clk) begin
		randnum <= `PROP_DELAY {randnum[5] ^ randnum[3] ^ randnum[2] ^ randnum[0] ,randnum[15:1]};
	end
endmodule