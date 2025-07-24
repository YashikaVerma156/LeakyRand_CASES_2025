`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    14:25:01 10/01/2024 
// Design Name: 
// Module Name:    bin_to_dec 
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
module bin_to_dec(clk, binary_ready, binary_in, ascii_out, decimal_ready);

    //Input from environment
	input clk;
	input binary_ready;
	input [31:0] binary_in;
    
    //Output to environment
    output wire [127:0] ascii_out;
	output reg decimal_ready = 0;

    //Internal variables 
    reg [5:0] i = 32;
    reg [39:0] temp = 0;
    reg [3:0] decimal_digit [0:9];
    wire [7:0] padded_digit [0:15]; 
	reg init = 0;
	reg state = 0;
    
    integer j;
    
//    always @(posedge clk) begin
//        $display("In bcd temp, time = %d, binary_in = %b, binary_ready = %b, ascii_out = %s, decimal_ready = %b , i = %d, decimal_digit[0] = %d, decimal_digit[1] = %d",$time, binary_in, binary_ready, ascii_out, decimal_ready, i, decimal_digit[0], decimal_digit[1]);
//    end

    always @(posedge clk) begin
		if(init == 0) begin
			for(j=0; j<10; j = j+1) begin
				decimal_digit[j] <= 0;
			end
			init <= 1;
		end
		else if(binary_ready == 1 && decimal_ready == 0) begin
			if(i == 0) begin
				i <= 32;
				decimal_ready <= 1;
				decimal_digit[9] <= temp[39:36];
				decimal_digit[8] <= temp[35:32];
				decimal_digit[7] <= temp[31:28];
				decimal_digit[6] <= temp[27:24];
				decimal_digit[5] <= temp[23:20];
				decimal_digit[4] <= temp[19:16];
				decimal_digit[3] <= temp[15:12];
				decimal_digit[2] <= temp[11:8];
				decimal_digit[1] <= temp[7:4];
				decimal_digit[0] <= temp[3:0];
			end
			else if(state == 0) begin
				if (temp[39:36] >= 5) begin
					temp[39:36] <= temp[39:36] + 3;
				end
				if (temp[35:32] >= 5) begin
					temp[35:32] <= temp[35:32] + 3;
				end
				if (temp[31:28] >= 5) begin
					temp[31:28] <= temp[31:28] + 3;
				end
				if (temp[27:24] >= 5) begin
					temp[27:24] <= temp[27:24] + 3;
				end
				if (temp[23:20] >= 5) begin
					temp[23:20] <= temp[23:20] + 3;
				end
				if (temp[19:16] >= 5) begin
					temp[19:16] <= temp[19:16] + 3;
				end
				if (temp[15:12] >= 5) begin
					temp[15:12] <= temp[15:12] + 3;
				end
				if (temp[11:8] >= 5) begin
					temp[11:8] <= temp[11:8] + 3;
				end
				if (temp[7:4] >= 5) begin
					temp[7:4] <= temp[7:4] + 3;
				end
				if (temp[3:0] >= 5) begin
					temp[3:0] <= temp[3:0] + 3;
				end	
				state <= 1;
			end
			else begin
				temp <= {temp[38:0],binary_in[i-1]};
				state <= 0;
				i <= i-1;
			end
		end
    end
	
	genvar k;
	generate
		for(k=0; k<10; k = k+1) begin:gen3	
			assign padded_digit[k] = decimal_digit[k] + 8'd48; 
		end
	endgenerate
	
	genvar h;
	generate
		for(h=10; h<16; h = h+1) begin:gen4	
			assign padded_digit[h] = 8'd48; 
		end
	endgenerate
	
	assign ascii_out = {
						padded_digit[15], 
						padded_digit[14], 
						padded_digit[13], 
						padded_digit[12], 
						padded_digit[11], 
						padded_digit[10], 
						padded_digit[9], 
						padded_digit[8], 
						padded_digit[7], 
						padded_digit[6], 
						padded_digit[5], 
						padded_digit[4], 
						padded_digit[3], 
						padded_digit[2], 
						padded_digit[1], 
						padded_digit[0]
					};
	
endmodule 
