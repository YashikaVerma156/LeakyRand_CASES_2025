`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    07:21:06 09/26/2024 
// Design Name: 
// Module Name:    top 
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
module top(clk, lcd);
	input clk;
	output wire [6:0] lcd;
	
	reg outstring_ready = 0;
	wire [31:0] total_cycles;
	wire [127:0] ascii_out;
	wire decimal_ready;
	wire print_done;
	wire [0:127] line1;	
	wire [0:127] line2;

	
	bin_to_dec uutBTD(clk, outstring_ready, total_cycles, ascii_out, decimal_ready);
	lcd_driver uutLCD(
		.output_ready(decimal_ready),
		.clk(clk),
		.line1(line1),		
		.line2(line2),
		.lcd(lcd),
		.output_done(print_done)
	);

	genvar k;
	generate
		for(k=0; k<127; k=k+1) begin : gen2
			assign line2[k] = ascii_out[127-k];
		end
	endgenerate
	assign line1 = "Dhruv    Pragati";
	assign total_cycles = 32'd928;
	reg [31:0] count;
	always @(posedge clk) begin
		count <= count +1;
		if(count == 100) begin
			outstring_ready <= 1;
		end
	end
endmodule
