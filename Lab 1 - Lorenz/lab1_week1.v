// Joash Shankar, Ming He, Nikhil Pillai
// ECE 5760 Lab 1 Spring 2024
// Hardware ODE solver with HPS control and sonification

`timescale 1ns/1ns

module testbench();
	
	reg clk_50, reset;
	
	wire signed [26:0]  testbench_out_x, testbench_out_y, testbench_out_z;
    reg  signed [26:0]  dt, x, y, z, sigma, beta, rho; 
	
	// Initialize clock and values
	initial begin
		clk_50 = 1'b0;
        dt     = 27'd4096;  // 1/256
        x      = {7'b1111111,20'b0}; // -1
        y      = 27'd104857;  // 0.1 
        z      = 27'd26214400;  // 25
        sigma  = 27'd10485760;  // 10
        beta   = 27'd2796202;  // 8/3
        rho    = 27'd29360128;  // 28
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


	// Instantiation of Device Under Test
    DDA DUT   (.clock(clk_50), 
               .reset(reset),
               .dt_in(dt),
               .x_in(x),
               .y_in(y),
               .z_in(z),
               .sigma_in(sigma),
               .beta_in(beta),
               .rho_in(rho),
               .out_x(testbench_out_x),
               .out_y(testbench_out_y),
               .out_z(testbench_out_z)
              );
	
endmodule

//////////////////////////////////////////////////////////////
////////////	Digital Differential Analyzer	//////////////
//////////////////////////////////////////////////////////////

module DDA (clock, reset, dt_in, x_in, y_in, z_in, sigma_in, beta_in, rho_in, out_x, out_y, out_z);
    // init inputs and outputs
    input clock, reset;
    input signed [26:0] dt_in, x_in, y_in, z_in, sigma_in, beta_in, rho_in;
    output signed [26:0] out_x, out_y, out_z;
    
    // intermediate signals
	wire signed [26:0] integ_x_out, integ_y_out, integ_z_out, dx_out, dy_out, dz_out;

	// instantiate differential blocks
	dxdt dx_diff (.out(dx_out), 
				  .sigma(sigma_in), 
				  .x_(integ_x_out), 
				  .y_(integ_y_out)
				  );

	dydt dy_diff (.out(dy_out), 
				  .rho(rho_in), 
				  .x_(integ_x_out), 
				  .y_(integ_y_out),
				  .z_(integ_z_out));

	dzdt dz_diff (.out(dz_out),
				  .beta(beta_in), 
				  .x_(integ_x_out), 
				  .y_(integ_y_out),
				  .z_(integ_z_out));

	
    // instantiate integrator blocks
	integrator integ_x (.out(integ_x_out),
						.funct(dx_out),
						.InitialOut(x_in),
						.clk(clock),
						.reset(reset));
	
	integrator integ_y (.out(integ_y_out),
						.funct(dy_out),
						.InitialOut(y_in),
						.clk(clock),
						.reset(reset));
	
	integrator integ_z (.out(integ_z_out),
						.funct(dz_out),
						.InitialOut(z_in),
						.clk(clock),
						.reset(reset));
	
	assign out_x = integ_x_out;
	assign out_y = integ_y_out;
	assign out_z = integ_z_out;

endmodule

//////////////////////////////////////////////////////////////
//////////////////////////	dx*dt	//////////////////////////
//////////////////////////////////////////////////////////////
module dxdt(out, sigma, x_, y_);
    // init inputs and outputs
    input signed [26:0] x_, y_, sigma;
	output signed [26:0] out;

	// intermediate signals
    wire signed [26:0] y_minus_x, dx_mult, out_dxdt;

	assign y_minus_x = y_ - x_; 
	assign dx_mult = y_minus_x >>> 8;
	signed_mult dx_mult_sigma (.out(out_dxdt), .a(dx_mult), .b(sigma));

	assign out = out_dxdt;

endmodule

//////////////////////////////////////////////////////////////
//////////////////////////	dy*dt	//////////////////////////
//////////////////////////////////////////////////////////////
module dydt(out, rho, x_, y_, z_);
    // init inputs and outputs
	input signed [26:0] x_, y_, z_, rho;
	output signed [26:0] out;
	
    // intermediate signals
    wire signed [26:0] shifted_x, shifted_y, rho_minus_z_shifted, rho_minus_z, dx_mult, xrho_minus_z, out_dydt;
	// assign shifted_x = x_ >>> 8;
	assign rho_minus_z = rho - z_;
	assign rho_minus_z_shifted = rho_minus_z >>> 8;
	assign shifted_y = y_ >>> 8;

	signed_mult shifted_x_mult_rho (.out(xrho_minus_z), .a(x_), .b(rho_minus_z_shifted));
	assign out_dydt = xrho_minus_z - shifted_y;

	assign out = out_dydt;
endmodule

//////////////////////////////////////////////////////////////
//////////////////////////	dz*dt	//////////////////////////
//////////////////////////////////////////////////////////////
module dzdt(out, beta, x_, y_, z_);
    // init inputs and outputs
	input signed [26:0] x_, y_, z_, beta;
	output signed [26:0] out;
	
    // intermediate signals
	wire signed [26:0] dt_mult_y, dt_mult_z, xy_minus_betaz, out_zbeta, out_xy, out_dzdt;

	assign dt_mult_y = y_ >>> 8;
	assign dt_mult_z = z_ >>> 8;
	
	signed_mult dt_mult_x_mult_beta(.out(out_zbeta), .a(dt_mult_z), .b(beta));
	signed_mult x_mult_dt_mult_y(.out(out_xy), .a(x_), .b(dt_mult_y));
	
	assign xy_minus_betaz = out_xy - out_zbeta;
	assign out = xy_minus_betaz;
    
endmodule
    
/////////////////////////////////////////////////
///////////////// integrator ////////////////////
/////////////////////////////////////////////////
module integrator(out,funct,InitialOut,clk,reset);
	output signed [26:0] out; 		 // the state variable V
	input signed [26:0] funct;       // the dV/dt function
	input clk, reset;
	input signed [26:0] InitialOut;  // the initial state variable V
	
	wire signed	[26:0] out, v1new ;
	reg signed	[26:0] v1 ;
	
	always @ (posedge clk) begin
		if (reset==1) // active high reset	
			v1 <= InitialOut ;
		else 
			v1 <= v1new ;	
	end
    
	assign v1new = v1 + funct ;
	assign out = v1 ;
endmodule

//////////////////////////////////////////////////
//// signed mult of 7.20 format 2'comp////////////
//////////////////////////////////////////////////
module signed_mult (out, a, b);
	output 	signed  [26:0]	out;
	input 	signed	[26:0] 	a;
	input 	signed	[26:0] 	b;
    
	// intermediate full bit length
	wire 	signed	[53:0]	mult_out;
	assign mult_out = a * b;
    
	// select bits for 7.20 fixed point
	assign out = {mult_out[53], mult_out[45:20]};
endmodule