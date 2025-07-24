`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   09:01:28 10/13/2024
// Design Name:   environment
// Module Name:   /home/ise/Xilinx_files/fillcheck.v
// Project Name:  UGP
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

module CRFcheck;

	// Inputs
	reg clk;
	reg [3:0] Y;
	reg rot_a;
	reg rot_b;
	reg PB1;

	// Outputs
	wire [7:0] led;
	wire [6:0] lcd;

	// Instantiate the Unit Under Test (UUT)
	environment uut (
		.clk(clk), 
		.Y(Y), 
		.rot_a(rot_a), 
		.rot_b(rot_b), 
		.PB1(PB1), 
		.led(led), 
		.lcd(lcd)
	);

	initial begin
		forever begin
			clk = 0;
			#5;
			clk = 1;
			#5;
			clk = 0;
		end
	end
	integer i;


	initial begin
        PB1 = 0;
        
        // ECI
        Y = 4'b0000;
		rot_a <= 0; rot_b <= 0; #10;
        rot_a <= 1; rot_b <= 1; #10;
        rot_a <= 0; rot_b <= 0; #10;
        
        Y = 4'b0000;
		rot_a <= 0; rot_b <= 0; #10;
        rot_a <= 1; rot_b <= 1; #10;
        rot_a <= 0; rot_b <= 0; #10;
        
        Y = 4'b1000;
		rot_a <= 0; rot_b <= 0; #10;
        rot_a <= 1; rot_b <= 1; #10;
        rot_a <= 0; rot_b <= 0; #10;
        
        // SNDR_PROBE_BLOCKS
        Y = 4'b0010;
		rot_a <= 0; rot_b <= 0; #10;
        rot_a <= 1; rot_b <= 1; #10;
        rot_a <= 0; rot_b <= 0; #10;
        
        
        // CRF_BLOCKS0
        Y = 4'b0001;
		rot_a <= 0; rot_b <= 0; #10;
        rot_a <= 1; rot_b <= 1; #10;
        rot_a <= 0; rot_b <= 0; #10;
        
        Y = 4'b0000;
		rot_a <= 0; rot_b <= 0; #10;
        rot_a <= 1; rot_b <= 1; #10;
        rot_a <= 0; rot_b <= 0; #10;
        
        // EXP0
        Y = 4'b0001;
		rot_a <= 0; rot_b <= 0; #10;
        rot_a <= 1; rot_b <= 1; #10;
        rot_a <= 0; rot_b <= 0; #10;
        
        // CRF_BLOCKS1
        Y = 4'b0001;
		rot_a <= 0; rot_b <= 0; #10;
        rot_a <= 1; rot_b <= 1; #10;
        rot_a <= 0; rot_b <= 0; #10;
        
        Y = 4'b0010;
		rot_a <= 0; rot_b <= 0; #10;
        rot_a <= 1; rot_b <= 1; #10;
        rot_a <= 0; rot_b <= 0; #10;
        
        // EXP1
        Y = 4'b0010;
		rot_a <= 0; rot_b <= 0; #10;
        rot_a <= 1; rot_b <= 1; #10;
        rot_a <= 0; rot_b <= 0; #10;
        
        // CRF_BLOCKS2
        Y = 4'b0000;
		rot_a <= 0; rot_b <= 0; #10;
        rot_a <= 1; rot_b <= 1; #10;
        rot_a <= 0; rot_b <= 0; #10;
        
        Y = 4'b0000;
		rot_a <= 0; rot_b <= 0; #10;
        rot_a <= 1; rot_b <= 1; #10;
        rot_a <= 0; rot_b <= 0; #10;
        
        // EXP2
        Y = 4'b0000;
		rot_a <= 0; rot_b <= 0; #10;
        rot_a <= 1; rot_b <= 1; #10;
        rot_a <= 0; rot_b <= 0; #10;
        
        // Input string
        Y <= 4'b1011;
//        Y <= 4'b0010;
//        Y<= 4'b1111;
//        Y <= 4'b0000;
        
		for(i=0; i<256; i = i+1) begin
			rot_a <= 0; rot_b <= 0; #10;
			rot_a <= 1;	rot_b <= 1; #10;
			rot_a <= 0; rot_b <= 0; #10;
		end
		PB1 <= 1;
		#2;
		PB1 <= 0;

	end
      
endmodule

