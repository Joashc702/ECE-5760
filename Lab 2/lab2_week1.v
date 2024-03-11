// Joash Shankar, Ming He, Nikhil Pillai
// ECE 5760 Lab 2 Spring 2024
// Mandelbrot Set Visualizer

`timescale 1ns/1ns

module testbench();
	
	reg clk_50, reset;
	wire [26:0] c_i, c_r;
	wire [11:0] max_iter;
	wire [11:0] out;
	
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

	assign c_i = {4'd0, 1'd1, 22'd0}; // 0.25
	assign c_r = {4'd0, 1'd1, 22'd0}; // 0.25
	assign max_iter = 12'd1000; 

	// Instantiation of Device Under Test
    mandelbrot MBROT (.clock(clk_50), 
               .reset(reset),
               .c_i(c_i), 
			   .c_r(c_r), 
			   .max_iter(max_iter), 
			   .out(out)
              );
	
endmodule

//////////////////////////////////////////////////////////////
////////////	Mandelbrot Set Visualizer	    //////////////
//////////////////////////////////////////////////////////////

module mandelbrot (clock, reset, c_i, c_r, max_iter, out);
    input clock, reset;
	input signed [26:0] c_i, c_r;
	input [11:0] max_iter;
	output [11:0] out;
    
    reg signed [26:0] z_r, z_i;
    reg [11:0] temp_iter;
    wire signed [26:0] zr_sq, zi_sq, zr_zi;
    wire signed [26:0] zr_temp, zi_temp;
	wire [26:0] zr_abs, zi_abs;
	wire done;
    
    // clocked
    always @(posedge clock) begin
        if (reset == 1) begin
            z_r <= 0;
            z_i <= 0;
            temp_iter <= 0;
        end 
        else if (done == 0) begin
            z_r <= zr_temp;
            z_i <= zi_temp;
            temp_iter <= temp_iter + 1;
        end
    end
    
    // signed mults
    signed_mult zrSq (.out(zr_sq), .a(z_r), .b(z_r));
    signed_mult ziSq (.out(zi_sq), .a(z_i), .b(z_i));
    signed_mult twoZrZi (.out(zr_zi), .a(z_r), .b(z_i));

    // combinational
    assign zr_temp = zr_sq - zi_sq + c_r;
    assign zi_temp = (zr_zi <<< 1) + c_i;
	
	assign zr_abs = (z_r[26] == 1) ? (~z_r + 1) : (z_r);  
	assign zi_abs = (z_i[26] == 1) ? (~z_i + 1) : (z_i);  
    assign done = (zr_abs > {4'd2, 23'd0}) || (zi_abs > {4'd2, 23'd0}) || ((zr_sq + zi_sq) > {4'd4, 23'd0}) || (temp_iter >= max_iter);
	
	assign out = temp_iter;
endmodule

//////////////////////////////////////////////////
//// signed mult of 4.23 format 2'comp////////////
//////////////////////////////////////////////////
module signed_mult (out, a, b);
	output 	signed  [26:0]	out;
	input 	signed	[26:0] 	a;
	input 	signed	[26:0] 	b;
    
	// intermediate full bit length
	wire 	signed	[53:0]	mult_out;
	assign mult_out = a * b;
    
	// select bits for 4.23 fixed point
	assign out = {mult_out[53], mult_out[48:23]};
endmodule
