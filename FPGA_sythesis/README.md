# Covert Craft

## Introduction
CovertCraft is a covert channel that exploits a fully associative cache with random replacement. We have synthesized Covert Craft on a Spartan-3E FPGA board. This synthesis effort serves as a proof of concept to demonstrate that a fully associative cache exercising random replacement policy is still vulnerable to timing-based covert channel attacks.

## Description of Files
- **`environment.v`**: Top level module which acts as an interface of communication among all the other modules.
- **`input_interface.v`**: Takes input from the FPGA board.
- **`detect_event.v`**: Detects a rotation event. Used by `input_interface.v` to take input.
- **`controller.v`**: Represents the cache controller. Used to simulate a fully associative cache.
- **`random_gen.v`**: A pseudo random generator used for implementing the random replacement policy. Used by `controller.v` to implement the cache.
- **`sender.v`**: Sender FSM which represents the sender process.
- **`receiver.v`**: Receiver FSM which represents the receiver process.
- **`bin_to_dec.v`**: Converts a binary number to its ascii-coded decimal representation. Used to display number of errors and clock cycles on the LCD display in base 10. 
- **`lcd_driver.v`**: Driver to display the output on the LCD.
- **`match.v`**: Matches the input string (sent by the sender) and the output string (inferred by receiver) and counts the number of errors.
- **`macros.v`**: Contains macro definitions of all the configurable parameters. 
- **`environment.ucf`**: Constraints file containing mappings of signals (and other information) on the FPGA board.

Please refer to __Section 3: Implementation__ of **`./docs/Report.pdf`** for design, detailed description and working of all the files. <br>
Please refer to __Section 5: Input Protocol__ of **`./docs/Report.pdf`** for detailed steps for providing input to the FPGA Board. 

## Requirements
 - Xilinx ISE 14.7 software.
 - Xilinx Spartan 3E starter board.
 - Connecting cables and cable driver.

## Synthesis
 - Open Xilinx ISE 14.7.
 - Create a new project and add all the verilog (.v) files and `environment.ucf` file.
 - Synthesize the design.

For a brief overview of the project please refer to **`./docs/Presentation.pdf`**. <br>
For detailed documentation of the project, obseevations and results, please refer to the report : **`./docs/Report.pdf`**.

## Evaluation
- Input format is explained in section5 and figure 5.2 of /docs/Report.pdf. </br>
- The input parameters specified in figure 5.2 of /docs/Report.pdf are: </br>
  eci(error correction interval)  </br>
  sndr_probe_blocks(disturbance set or DS)  </br>
  in_string(1024) 1024 bits string  </br>
  crf_blocks0, exp0, crf_blocks1, exp1, crf_blocks2, and exp2 specify the occupancy sequence used as shown in table 3 in paper. </br>
  (crf_blocks0) ^ exp0  (crf_blocks1) ^ exp1  (crf_blocks2) ^ exp2
- Three types of input strings(length 1024) are used, (a) all 1s, (b) 1011 repeated 256 times, (c) 0010 repeated 256 times  </br>
- The results in the paper are mentioned in section 5.10.1 under the heading "FPGA Synthesis of LeakyRand:".
