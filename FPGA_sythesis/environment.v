`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    14:02:09 10/01/2024 
// Design Name: 
// Module Name:    environment 
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
module environment(clk, Y, rot_a, rot_b, PB1, led, lcd);
	// Inputs
	input clk;
	input [3:0] Y;
	input rot_a, rot_b;
	input PB1;
	
	// Outputs
	output wire [7:0] led;
	output wire [6:0] lcd;
	
    //Internal Variables
	wire rot_event;
	
	wire [`STR_LEN-1:0] in_string;
	wire [`STR_LEN-1:0] out_string;
	
	wire opcodeS;
	wire [`TAG_LEN-1:0] addrS;
	wire validS;
	wire retvalS;
	
	wire opcodeR;
	wire [`TAG_LEN-1:0] addrR;
	wire validR;
	wire retvalR;
	
	wire [`COUNTER_SIZE-1:0] total_cycle_counter;
	wire outstring_ready;
	
	wire input_ready;
	wire [`LOG_STR_LEN-1:0] eos;
    wire [`LOG_NB:0] occ_init;
	wire [`LOG_NB:0] occ_final;
	wire equal;
	wire [`LOG_STR_LEN:0] num_errors;
	wire match_done;
	wire output_ready;
	wire decimal_ready1;
	wire decimal_ready2;
	
	wire [31:0] temp;
	wire [127:0] temp_out;
	wire [127:0] ascii_out1;
	wire [127:0] ascii_out2;
	wire [0:127] line1;
	wire [0:127] line2;

    wire [`INPUT_SIZE3-1:0] eci;
    wire [`INPUT_SIZE1-1:0] sndr_probe_blocks;
    wire [`INPUT_SIZE2-1:0] crp_blocks;
    
    wire [`INPUT_SIZE2-1:0] crf_blocks0;
    wire [`INPUT_SIZE2-1:0] crf_blocks1;
    wire [`INPUT_SIZE2-1:0] crf_blocks2;

    wire [`INPUT_SIZE1-1:0] exp0;
    wire [`INPUT_SIZE1-1:0] exp1;
    wire [`INPUT_SIZE1-1:0] exp2;
	
	wire [`COUNTER_SIZE-1:0] sndr_wait_cycles;
	wire [`COUNTER_SIZE-1:0] recv_wait_cycles;
	wire [`COUNTER_SIZE-1:0] total_cycles;

	wire [`COUNTER_SIZE-1:0] crf_cycles0;
	wire [`COUNTER_SIZE-1:0] crf_cycles1;
	wire [`COUNTER_SIZE-1:0] crf_cycles2;
	wire [`COUNTER_SIZE-1:0] crf_total_cycles;
    
	wire [`COUNTER_SIZE-1:0] crp_cycles0;
	wire [`COUNTER_SIZE-1:0] crp_cycles1;
	wire [`COUNTER_SIZE-1:0] crp_total_cycles;
    
    wire [`COUNTER_SIZE-1:0] total_ed_cycles;
	wire [31:0] bin_total_cycle_counter;
    
    
	assign bin_total_cycle_counter = {12'd0, total_cycle_counter};
	assign temp = {21'd0, num_errors};
	assign temp_out = equal ? {"PASS, Error=", ascii_out1[31:0]} : {"FAIL, Error=", ascii_out1[31:0]};
	assign output_ready = decimal_ready1 & match_done & decimal_ready2;
	
    assign sndr_wait_cycles = (sndr_probe_blocks * `MISS_LATENCY) + (`NUM_BLOCKS - sndr_probe_blocks) * `HIT_LATENCY;
	assign recv_wait_cycles = (`MISS_LATENCY + `HIT_LATENCY) * sndr_probe_blocks; 
	assign total_cycles = sndr_wait_cycles + recv_wait_cycles;

	assign crf_cycles0 = crf_blocks0 * `MISS_LATENCY;
	assign crf_cycles1 = crf_blocks1 * `MISS_LATENCY;
	assign crf_cycles2 = crf_blocks2 * `MISS_LATENCY;
	
	assign crf_total_cycles = crf_cycles0*exp0 + crf_cycles1*exp1 + crf_cycles2*exp2;
	assign total_ed_cycles = total_cycles + sndr_probe_blocks * (3*`MISS_LATENCY + `NUM_BLOCKS * `HIT_LATENCY);
    
   assign crp_blocks = (crf_blocks0 > crf_blocks1) ? ((crf_blocks0 > crf_blocks2) ? crf_blocks0 : crf_blocks2) : ((crf_blocks1 > crf_blocks2) ? crf_blocks1 : crf_blocks2);
	assign crp_cycles0 = (`MISS_LATENCY + `HIT_LATENCY) * crp_blocks;
	assign crp_cycles1 = `NUM_BLOCKS * `MISS_LATENCY;
	assign crp_total_cycles = crp_cycles0 + crp_cycles1;
	
    
	detect_event uutD(
		.clk(clk),
		.rot_a(rot_a),
		.rot_b(rot_b),
		.rot_event(rot_event)
	);
	
	input_interface uutI(
		.clk(clk), 
		.sw(Y), 
		.rot_event(rot_event), 
        .PB1(PB1),
        .input_ready(input_ready), 
        .eos(eos), 
		.in_string(in_string), 
        .eci(eci),
        .sndr_probe_blocks(sndr_probe_blocks),
        .crf_blocks0(crf_blocks0), .crf_blocks1(crf_blocks1), .crf_blocks2(crf_blocks2),
        .exp0(exp0), .exp1(exp1), .exp2(exp2)
	);
	
	sender uutS(
		.input_ready(input_ready), 
		.string(in_string), 
		.clk(clk), 
		.retvalS(retvalS), 
		.validS(validS), 
		.opcodeS(opcodeS), 
		.addrS(addrS), 
		.eos(eos),
        .eci(eci), .sndr_probe_blocks(sndr_probe_blocks), .total_cycles(total_cycles), 
        .recv_wait_cycles(recv_wait_cycles), .total_ed_cycles(total_ed_cycles), 
        .crf_total_cycles(crf_total_cycles), .crp_total_cycles(crp_total_cycles)
	);
	
	controller uutC(
		.clk(clk), 
		.opcodeR(opcodeR), 
		.addrR(addrR), 
		.validR(validR), 
		.opcodeS(opcodeS), 
		.addrS(addrS), 
		.validS(validS), 
		.retvalR(retvalR), 
		.retvalS(retvalS)
	);
	
	receiver uutR(
		.input_ready(input_ready), 
		.string_out(out_string), 
		.clk(clk), 
		.retvalR(retvalR), 
		.validR(validR), 
		.opcodeR(opcodeR), 
		.addrR(addrR), 
		.outstring_ready(outstring_ready), 
		.total_cycle_counter(total_cycle_counter), 
		.eos(eos),
		.occ_final(occ_final),
		.occ_init(occ_init),
        .eci(eci),
        .sndr_probe_blocks(sndr_probe_blocks),
        .crp_blocks(crp_blocks),
        .crf_blocks0(crf_blocks0), .crf_blocks1(crf_blocks1), .crf_blocks2(crf_blocks2),
        .exp0(exp0), .exp1(exp1), .exp2(exp2),
		.recv_wait_cycles(recv_wait_cycles), .total_cycles(total_cycles), 
		.crf_cycles0(crf_cycles0), .crf_cycles1(crf_cycles1), .crf_cycles2(crf_cycles2), 
		.crf_total_cycles(crf_total_cycles), .crp_cycles0(crp_cycles0), .crp_cycles1(crp_cycles1), .total_ed_cycles(total_ed_cycles)  
	);
	
	bin_to_dec uutB1(
		.clk(clk), 
		.binary_ready(match_done), 
		.binary_in(temp), 
		.ascii_out(ascii_out1), 
		.decimal_ready(decimal_ready1)
	);
	
	bin_to_dec uutB2(
		.clk(clk), 
		.binary_ready(outstring_ready),  
		.binary_in(bin_total_cycle_counter), 
		.ascii_out(ascii_out2), 
		.decimal_ready(decimal_ready2)
	);
	
	lcd_driver uutL(
		.output_ready(output_ready), 
		.line1(line1), 
		.line2(line2), 
		.clk(clk), 
		.lcd(lcd)
	);
	
	match uutM(
		.clk(clk), 
		.outstring_ready(outstring_ready), 
		.in_string(in_string), 
		.out_string(out_string), 
		.equal(equal), 
		.num_errors(num_errors), 
		.eos(eos), 
		.match_done(match_done)
	);

	assign led[7:4] = occ_init[3:0];  //assuming occupancy >0 and <=16
	assign led[3:0] = occ_final[3:0]; //assuming occupancy >0 and <=16
	
	genvar k;
	generate
		for(k=0; k<128; k=k+1) begin : gen2
			assign line2[k] = ascii_out2[127-k];
		end
	endgenerate
	
	genvar p;
	generate
		for(p=0; p<128; p=p+1) begin : gen3
			assign line1[p] = temp_out[127-p];
		end
	endgenerate
	
	always @(posedge clk) begin
		$display("In env1 : time = %d, input_ready = %b, outstring_ready = %b, num_errors = %d, out_string = %b", $time, input_ready, outstring_ready, num_errors, out_string);
		$display("In env2 : time = %d, sndr_wait_cycles = %d, recv_wait_cycles = %d, total_cycles = %d, crf_total_cycles = %d, crf_cycles0 = %d, crf_cycles1 = %d, crf_cycles2 = %d", $time, sndr_wait_cycles, recv_wait_cycles, total_cycles, crf_total_cycles, crf_cycles0, crf_cycles1, crf_cycles2);
		$display("In env3 : time = %d, crp_blocks = %d, crp_total_cycles = %d, crp_cycles0 = %d, crp_cycles1 = %d, total_ed_cycles = %d", $time, crp_blocks, crp_total_cycles, crp_cycles0, crp_cycles1, total_ed_cycles);	
	end
endmodule
