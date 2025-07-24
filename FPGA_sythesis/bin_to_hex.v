`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    02:47:35 09/28/2024 
// Design Name: 
// Module Name:    bin_to_hex 
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
module bin_to_hex(clk, binary_in, binary_ready, bch_out, hex_ready, print_done);
    // input from environment
	input clk;

	// input from receiver
	input binary_ready;
	input [63:0] binary_in;

	// input from lcd driver
	input print_done;
	
    // output to environment
    output reg [127:0] bch_out; 
	
	// output to environment
	output reg hex_ready = 0;
    
    integer i;
    always @(posedge clk) begin
        if(print_done == 1) begin
            hex_ready <= 0;
        end
        else if(binary_ready == 1 && hex_ready == 0) begin
            for (i = 0; i < 16; i = i+1) begin
                case (binary_in[63 - i*4 -: 4])
                    4'b0000: bch_out[127 - i*8 -: 8] <= 8'h30; 
                    4'b0001: bch_out[127 - i*8 -: 8] <= 8'h31; 
                    4'b0010: bch_out[127 - i*8 -: 8] <= 8'h32;
                    4'b0011: bch_out[127 - i*8 -: 8] <= 8'h33; 
                    4'b0100: bch_out[127 - i*8 -: 8] <= 8'h34; 
                    4'b0101: bch_out[127 - i*8 -: 8] <= 8'h35; 
                    4'b0110: bch_out[127 - i*8 -: 8] <= 8'h36; 
                    4'b0111: bch_out[127 - i*8 -: 8] <= 8'h37; 
                    4'b1000: bch_out[127 - i*8 -: 8] <= 8'h38; 
                    4'b1001: bch_out[127 - i*8 -: 8] <= 8'h39; 
                    4'b1010: bch_out[127 - i*8 -: 8] <= 8'h41; 
                    4'b1011: bch_out[127 - i*8 -: 8] <= 8'h42; 
                    4'b1100: bch_out[127 - i*8 -: 8] <= 8'h43; 
                    4'b1101: bch_out[127 - i*8 -: 8] <= 8'h44; 
                    4'b1110: bch_out[127 - i*8 -: 8] <= 8'h45; 
                    4'b1111: bch_out[127 - i*8 -: 8] <= 8'h46; 
                endcase
            end
            hex_ready <= 1;
        end
    end
endmodule 