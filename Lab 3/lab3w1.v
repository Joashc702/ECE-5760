// Joash Shankar, Ming He, Nikhil Pillai
// ECE 5760 Lab 3 Spring 2024
// Multiprocessor Drum Synthesis

`timescale 1ns/1ns

module testbench();
	
	reg clk_50, reset;
	wire signed [17:0] out;
	reg signed [17:0] curr_u, prev_u;
	// Initialize clock
	initial begin
		clk_50 = 1'b0;
	end
	
	// Toggle the clocks
	always begin
		#10
		clk_50  = !clk_50;
	end
	
	
	// Intialize and drive signals
	initial begin
		reset  = 1'b0;
		#10 
		reset  = 1'b1;
		#30
		reset  = 1'b0;
	end
	
	always @(posedge clk_50) begin
		if (reset) begin
			curr_u <= {1'b0, 17'b00100000000000000};
			prev_u <= {1'b0, 17'b00100000000000000};
		end
		else begin
			curr_u <= out;
			prev_u <= curr_u;
		end
	end
	
	// Instantiation of Device Under Test
    drum oneNode (.clk(clk_50), 
				  .reset(reset), 
				  .rho_eff({1'b0, 17'b00010000000000000}),
				  .curr_u(curr_u),
				  .prev_u(prev_u),
				  .u_left(18'd0),
				  .u_right(18'd0),
				  .u_up(18'd0),
				  .u_down(18'd0),
				  .next(out));
	
endmodule

//////////////////////////////////////////////////////////////
////////////	Mandelbrot Set Visualizer	    //////////////
//////////////////////////////////////////////////////////////

module drum (clk, reset, rho_eff, curr_u, prev_u, u_left, u_right, u_up, u_down, next); 
	input clk, reset;
    input [17:0] rho_eff;
	input signed [17:0] curr_u, prev_u;
	//input signed [17:0] init_condition; // u
	input signed [17:0] u_left, u_right, u_up, u_down;
	output signed [17:0] next;
	
	wire signed [17:0] u_sum, rho_usum, inter_val;
	
	assign u_sum = u_left + u_right + u_up + u_down - (curr_u <<< 2);
	signed_mult rho_mult_usum(.out(rho_usum), .a(u_sum), .b(rho_eff)); 
	assign inter_val = rho_usum + (curr_u <<< 1) - prev_u + (prev_u >>> 10);
	assign next = inter_val - (inter_val >>> 9);
endmodule



//////////////////////////////////////////////////
//// signed mult of 1.17 format 2'comp////////////
//////////////////////////////////////////////////
module signed_mult (out, a, b);
	output 	signed  [17:0]	out;
	input 	signed	[17:0] 	a;
	input 	signed	[17:0] 	b;
    
	// intermediate full bit length
	wire 	signed	[35:0]	mult_out;
	assign mult_out = a * b;
    
	// select bits for 1.17 fixed point
	assign out = {mult_out[35], mult_out[33:17]};
endmodule