`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    14:17:32 10/01/2024 
// Design Name: 
// Module Name:    input_interface 
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
`define INP_DATA_LEN (`STR_LEN + `INPUT_SIZE3 + 3*`INPUT_SIZE2 + 4*`INPUT_SIZE1)
module input_interface(clk, sw, rot_event, PB1, input_ready, eos, in_string, eci, sndr_probe_blocks, 
                       crf_blocks0, crf_blocks1, crf_blocks2, exp0, exp1, exp2);
    
    //Input from environment
	input clk;
    input [3:0] sw;
    input rot_event;
	input PB1;
	
    //Output to environment
    output wire [`STR_LEN-1:0] in_string;
    output reg input_ready = 0;
	output reg [`LOG_STR_LEN-1:0] eos = 0;
    output wire [`INPUT_SIZE3-1:0] eci;
    output wire [`INPUT_SIZE1-1:0] sndr_probe_blocks;
    
    output wire [`INPUT_SIZE2-1:0] crf_blocks0;
    output wire [`INPUT_SIZE2-1:0] crf_blocks1;
    output wire [`INPUT_SIZE2-1:0] crf_blocks2;
    
    output wire [`INPUT_SIZE1-1:0] exp0;
    output wire [`INPUT_SIZE1-1:0] exp1;
    output wire [`INPUT_SIZE1-1:0] exp2;
    
    // Internal registers
    reg [`COUNTER_SIZE:0] counter = `INP_DATA_LEN - 4;
    reg [`INP_DATA_LEN-1:0] input_data = 0;
    reg prev_rot_event = 1;
	reg params_done = 0;
	reg input_end = 0;
	reg limit_reached = 0;
    
    
    assign eci = input_data[`INP_DATA_LEN-1 : `INP_DATA_LEN-`INPUT_SIZE3]; // 1075 to 1064
    assign sndr_probe_blocks = input_data[`INP_DATA_LEN-`INPUT_SIZE3-1:`INP_DATA_LEN-`INPUT_SIZE3-`INPUT_SIZE1]; // 1063 to 1060
    assign crf_blocks0 = input_data[`INP_DATA_LEN-`INPUT_SIZE3-`INPUT_SIZE1-1:`INP_DATA_LEN-`INPUT_SIZE3-`INPUT_SIZE2-`INPUT_SIZE1]; // 1059 to 1052
    assign exp0 = input_data[`INP_DATA_LEN-`INPUT_SIZE3-`INPUT_SIZE2-`INPUT_SIZE1-1:`INP_DATA_LEN-`INPUT_SIZE3-`INPUT_SIZE2-2*`INPUT_SIZE1]; // 1051 to 1048
    assign crf_blocks1 = input_data[`INP_DATA_LEN-`INPUT_SIZE3-`INPUT_SIZE2-2*`INPUT_SIZE1-1:`INP_DATA_LEN-`INPUT_SIZE3-2*`INPUT_SIZE2-2*`INPUT_SIZE1]; // 1047 to 1040
    assign exp1 = input_data[`INP_DATA_LEN-`INPUT_SIZE3-2*`INPUT_SIZE2-2*`INPUT_SIZE1-1:`INP_DATA_LEN-`INPUT_SIZE3-2*`INPUT_SIZE2-3*`INPUT_SIZE1]; // 1039 to 1036
    assign crf_blocks2 = input_data[`INP_DATA_LEN-`INPUT_SIZE3-2*`INPUT_SIZE2-3*`INPUT_SIZE1-1:`INP_DATA_LEN-`INPUT_SIZE3-3*`INPUT_SIZE2-3*`INPUT_SIZE1]; // 1035 to 1028
    assign exp2 = input_data[`INP_DATA_LEN-`INPUT_SIZE3-3*`INPUT_SIZE2-3*`INPUT_SIZE1-1:`INP_DATA_LEN-`INPUT_SIZE3-3*`INPUT_SIZE2-4*`INPUT_SIZE1]; // 1027 to 1024
    
    assign in_string = input_data[`STR_LEN-1 : 0]; // 1023 to 0
    
	always @(posedge PB1) begin
		input_end <= 1;
	end
	
	always @(posedge clk) begin
		eos <= (limit_reached) ? 10'd0 : (counter[`LOG_STR_LEN-1:0] + 10'd4);
		input_ready <= (input_end | limit_reached) ;
        if ((prev_rot_event == 0) && (rot_event == 1)) begin
            if(input_ready == 0) begin
				input_data[counter+3] <= sw[3];
				input_data[counter+2] <= sw[2];
				input_data[counter+1] <= sw[1];
				input_data[counter+0] <= sw[0];
				
				counter <= (counter == 0) ? 0 : (counter - 4);
				if (counter == 0) begin
					limit_reached <= 1;
				end
			end
        end
        prev_rot_event <= rot_event;
    end
    
//    always @(posedge clk) begin
//        $display("in ii1: time = %d, input_ready = %b, eos = %d, in_string = %b, eci = %d, sndr_probe_blocks = %d", $time, input_ready, eos, in_string, eci, sndr_probe_blocks);
//        $display("in ii2: time = %d, crf_blocks0 = %d, crf_blocks1 = %d, crf_blocks2 = %d", $time, crf_blocks0, crf_blocks1, crf_blocks2);
//        $display("in ii3: time = %d, exp0 = %d, exp1 = %d, exp2 = %d", $time, exp0, exp1, exp2);
//    end

endmodule 
