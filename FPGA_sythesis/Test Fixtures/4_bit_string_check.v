`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   06:36:03 09/14/2024
// Design Name:   environment
// Module Name:   /home/ise/FPGACovert/top.v
// Project Name:  FPGACovert
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: environment
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////
module top;

	// Inputs
	reg clk;
	reg PB1;
	reg [3:0] Y;

	// Outputs
	wire [7:0] led;

	// Instantiate the Unit Under Test (UUT)
	environment uut (
		.clk(clk), 
		.PB1(PB1), 
		.Y(Y), 
		.led(led) 
	);
	always @(posedge clk) begin
		$display(" time = %d, led = %b", $time, led);
	end
	initial begin
		forever begin
			clk = 0;
			#5
			clk = 1;
			#5
			clk = 0;
		end
	end
	initial begin
		// Initialize Inputs
		PB1 = 0;
		Y = 4'b1010;
		#100000;
		PB1 = 1;
        
		// Add stimulus here

	end
	
      
endmodule

