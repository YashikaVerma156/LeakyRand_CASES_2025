`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    14:35:18 10/01/2024 
// Design Name: 
// Module Name:    match 
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
module match(clk, outstring_ready, in_string, out_string, equal, num_errors, eos, match_done);

    //Input from environment
	input clk;
    
    //Input from input interface
	input [`STR_LEN-1:0] in_string;	
    input [`LOG_STR_LEN-1:0] eos;
    
    //Input from receiver
    input outstring_ready;
	input [`STR_LEN-1:0] out_string;
	
    //Output to environment
	output wire equal;	
	output reg [`LOG_STR_LEN:0] num_errors = 0;
	output reg match_done = 0;
		
    //Internal variables
	reg [`LOG_STR_LEN:0] index;
	reg init = 0;
	
	assign equal = (outstring_ready && (in_string == out_string)) ? 1 : 0;
    
	always @(posedge clk) begin
		if(outstring_ready == 1) begin
			if(init == 0) begin
				index <= {1'b0, eos};
				init <= 1;
			end
			else if(index < `STR_LEN && match_done == 0) begin
				num_errors <= (in_string[index] != out_string[index]) ? (num_errors + 1) : num_errors;
				index <= (index == `STR_LEN) ? index : index + 1;
			end
            else if(index == `STR_LEN) begin
                match_done <= 1;
            end
		end
	end
	
//	always @(posedge clk) begin
//		$display("In match: time = %d, eos = %d, outstring_ready = %b, outstring = %b\n",$time, eos, outstring_ready, out_string);
//	end
	
endmodule 
