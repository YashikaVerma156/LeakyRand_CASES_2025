`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    14:33:22 10/01/2024 
// Design Name: 
// Module Name:    lcd_driver 
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
module lcd_driver(output_ready, line1, line2, clk, lcd);

	// lcd_rs -> 0
	// lcd_rw -> 1
	// lcd_e -> 2
	// lcd 4,5,6,7 -> 3,4,5,6

    input output_ready;
    input [0:127] line1;
	input [0:127] line2;
	input clk;

	output reg [6:0] lcd; 
	
	reg [7:0] line1_index;
	reg [1:0] line1_state;
	 
	reg [7:0] line2_index;
	reg [1:0] line2_state;
	 
	reg [19:0] counter;
	reg [2:0] next_state;
	 
	reg [2:0] line_break_state;
	 
	reg [5:0] init_ROM [0:13];
	reg [3:0] init_ROM_index;
	
//    always @(posedge clk) begin
//        $display("In lcd, time = %d, output_ready = %b, line1 = %s, line2 = %s\n",$time, output_ready, line1, line2);
//    end
    
	always @ (posedge clk) begin
		if (output_ready == 0) begin
			line1_index <= 0;
			line1_state <= 3;
			line2_index <= 0;
			line2_state <= 3;
			counter <= 1_000_000;
			next_state <= 0;
			line_break_state <= 7;
			init_ROM_index <= 0;
			init_ROM[0] <= 6'h03;
			init_ROM[1] <= 6'h03;
			init_ROM[2] <= 6'h03;
			init_ROM[3] <= 6'h02;
			init_ROM[4] <= 6'h02;
			init_ROM[5] <= 6'h08;
			init_ROM[6] <= 6'h00;
			init_ROM[7] <= 6'h06;
			init_ROM[8] <= 6'h00;
			init_ROM[9] <= 6'h0c;
			init_ROM[10] <= 6'h00;
			init_ROM[11] <= 6'h01;
			init_ROM[12] <= 6'h08;
			init_ROM[13] <= 6'h00;
		end
		if (output_ready == 1) begin
			if (counter == 0) begin
				counter <= 1_000_000;  
			
				// Initialization state machine
				if (init_ROM_index == 14) begin
					next_state <= 4;
					init_ROM_index <= 0;
					line1_state <= 0;
				end
				
				if ((next_state != 4) && (init_ROM_index != 14)) begin
					case (next_state)
						0: begin
							lcd[2] <= 0;
							next_state <= 1;
						end
					
						1: begin
							lcd[0] <= init_ROM[init_ROM_index][5];
							lcd[1] <= init_ROM[init_ROM_index][4];
							lcd[6] <= init_ROM[init_ROM_index][3];
							lcd[5] <= init_ROM[init_ROM_index][2];
							lcd[4] <= init_ROM[init_ROM_index][1];
							lcd[3] <= init_ROM[init_ROM_index][0];
							next_state <= 2;
						end
						
						2: begin
							lcd[2] <= 1;
							next_state <= 3;
						end
					
						3: begin
							lcd[2] <= 0;
							next_state <= 1;
							init_ROM_index <= init_ROM_index + 1;
						end
					endcase
				end
			
				// First line state machine
				if (line1_index == 128) begin
					line1_state <= 3;
					line1_index <= 0;
					line_break_state <= 0;
				end
				if ((line1_state != 3) && (line1_index != 128)) begin
					case (line1_state)
						0: begin
							lcd[0] <= 1;
							lcd[1] <= 0;
							lcd[6] <= line1[line1_index];
							lcd[5] <= line1[line1_index+1];
							lcd[4] <= line1[line1_index+2];
							lcd[3] <= line1[line1_index+3];
							line1_state <= 1;
						end
						
						1: begin
							lcd[2] <= 1;
							line1_state <= 2;
						end
						
						2: begin
							lcd[2] <= 0;
							line1_state <= 0;
							line1_index <= line1_index+4;
						end
					endcase
				end
		
				// Line break state machine
				if (line_break_state != 7) begin
					case (line_break_state)
						0: begin
							lcd[0] <= 0;
							lcd[1] <= 0;
							lcd[6] <= 1;
							lcd[5] <= 1;
							lcd[4] <= 0;
							lcd[3] <= 0;
							line_break_state <= 1;
						end
						
						1: begin
							lcd[2] <= 1;
							line_break_state <= 2;
						end
							
						2: begin
							lcd[2] <= 0;
							line_break_state <= 3;
						end
							
						3: begin
							lcd[0] <= 0;
							lcd[1] <= 0;
							lcd[6] <= 0;
							lcd[5] <= 0;
							lcd[4] <= 0;
							lcd[3] <= 0;
							line_break_state <= 4;
						end
							
						4: begin
							lcd[2] <= 1;
							line_break_state <= 5;
						end
							
						5: begin
							lcd[2] <= 0;
							line_break_state <= 7;
							line2_state <= 0;
						end
					endcase
				end
		
				// Second line state machine
				if (line2_index == 128) begin
					line2_state <= 3;
					line2_index <= 0;
				end
				if ((line2_state != 3) && (line2_index != 128)) begin
					case (line2_state)
						0: begin
							lcd[0] <= 1;
							lcd[1] <= 0;
							lcd[6] <= line2[line2_index];
							lcd[5] <= line2[line2_index+1];
							lcd[4] <= line2[line2_index+2];
							lcd[3] <= line2[line2_index+3];
							line2_state <= 1;
						end
							
						1: begin
							lcd[2] <= 1;
							line2_state <= 2;
						end
						
						2: begin
							lcd[2] <= 0;
							line2_state <= 0;
							line2_index <= line2_index+4;
						end
					endcase
				end
			end
			else begin 
				counter <= counter - 1;
			end
		end
	end

endmodule

