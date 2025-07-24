`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    14:15:24 10/01/2024 
// Design Name: 
// Module Name:    detect_event 
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
module detect_event(clk, rot_a, rot_b, rot_event);
	
	// input signals from environment
    input clk, rot_a, rot_b;
	
	// output signal to input interface
	output reg rot_event;
	
	always @ (posedge clk) begin
		if ((rot_a == 1) && (rot_b == 1)) begin
			rot_event <= 1;
		end
		else if ((rot_a == 0) && (rot_b == 0)) begin
			rot_event <= 0;
		end
	end

endmodule 
