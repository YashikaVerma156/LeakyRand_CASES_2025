`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    18:37:42 08/24/2024 
// Design Name: 
// Module Name:    macros 
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

`define LOG_BLOCK_WIDTH 0
//`define BLOCK_WIDTH (1 << `LOG_BLOCK_WIDTH)
`define ADDR_LEN 16 
`define TAG_LEN (`ADDR_LEN - `LOG_BLOCK_WIDTH)
`define BLOCK_OFFSET `LOG_BLOCK_WIDTH
`define PROP_DELAY #1
`define NOP 2

`define LOG_NB 4
`define NUM_BLOCKS (1 << `LOG_NB)

`define LOG_STR_LEN 10
`define STR_LEN (1 << `LOG_STR_LEN)
`define COUNTER_SIZE 20
`define INPUT_SIZE3 12
`define INPUT_SIZE2 8
`define INPUT_SIZE1 4

`define MISS_LATENCY 4
`define HIT_LATENCY 2
//`define TOTAL_CYCLES (`SNDR_WAIT_CYCLES + `RECV_WAIT_CYCLES)

`define MAX_TAG ((1 << `TAG_LEN) - 1)
//`define RECV_WAIT_CYCLES ((`MISS_LATENCY + `HIT_LATENCY) * `SNDR_PROBE_BLOCKS)

//`define SNDR_PROBE_BLOCKS 4
//`define SNDR_WAIT_CYCLES (`SNDR_PROBE_BLOCKS * `MISS_LATENCY + (`NUM_BLOCKS - `SNDR_PROBE_BLOCKS) * `HIT_LATENCY)

//`define EXP0 1
//`define CRF_BLOCKS0 16
//`define CRF_CYCLES0 (`CRF_BLOCKS0 * `MISS_LATENCY)	
//`define START_TAG0 `MAX_TAG
//`define END_TAG0 (`START_TAG0 - `CRF_BLOCKS0 + 1)
 
//`define EXP1 0
//`define CRF_BLOCKS1 0
//`define CRF_CYCLES1 (`CRF_BLOCKS1 * `MISS_LATENCY)
//`define START_TAG1 (`END_TAG0 - 1)
//`define END_TAG1 (`START_TAG1 - `CRF_BLOCKS1 + 1)

//`define EXP2 0
//`define CRF_BLOCKS2 0
//`define CRF_CYCLES2 (`CRF_BLOCKS2 * `MISS_LATENCY)
//`define START_TAG2 (`END_TAG1 - 1)
//`define END_TAG2 (`START_TAG2 - `CRF_BLOCKS2 + 1)

//`define EXP3 0
//`define CRF_BLOCKS3 0
//`define CRF_CYCLES3 (`CRF_BLOCKS3 * `MISS_LATENCY)
//`define START_TAG3 (`END_TAG2 - 1)
//`define END_TAG3 (`START_TAG3 - `CRF_BLOCKS3 + 1)

//`define EXP4 0
//`define CRF_BLOCKS4 0
//`define CRF_CYCLES4 (`CRF_BLOCKS4 * `MISS_LATENCY)
//`define START_TAG4 (`END_TAG3 - 1)
//`define END_TAG4 (`START_TAG4 - `CRF_BLOCKS4 + 1)

//`define CRF_TOTAL_CYCLES (`CRF_CYCLES0*`EXP0 + `CRF_CYCLES1*`EXP1 + `CRF_CYCLES2*`EXP2 + `CRF_CYCLES3*`EXP3 + `CRF_CYCLES4*`EXP4)

//`define CRP_BLOCKS 16
//`define CRP_CYCLES0 ((`MISS_LATENCY + `HIT_LATENCY) * (`CRP_BLOCKS))
//`define CRP_CYCLES1 (`NUM_BLOCKS * `MISS_LATENCY)
//`define CRP_TOTAL_CYCLES (`CRP_CYCLES0 + `CRP_CYCLES1)

//`define ECI 16
//`define TOTAL_ED_CYCLES (`TOTAL_CYCLES + `SNDR_PROBE_BLOCKS * (3*`MISS_LATENCY + `NUM_BLOCKS * `HIT_LATENCY))
