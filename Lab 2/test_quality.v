// Joash Shankar, Ming He, Nikhil Pillai
// ECE 5760 Lab 2 Spring 2024
// Mandelbrot Set Visualizer

`timescale 1ns/1ns

module testbench();
	
	reg clk_50, reset;
	reg [26:0] ci, cr, ci_2, cr_2;
reg 		[18:0] 	write_address ;
reg 		[18:0] 	write_address_2 ;
reg 		[7:0] 	arbiter_state ;
reg 		[9:0] 	x_coord ;
reg 		[9:0] 	y_coord ;
reg 		[9:0] 	x_coord_2 ;
reg 		[9:0] 	y_coord_2;

reg 		[7:0] 	write_data ;
reg 		[7:0] 	write_data_2;

reg 			write_enable ;
reg  		         write_enable_2;


        wire [11:0] counter;
        wire [11:0] counter_2;
        
    	wire [26:0] step_size_x, step_every_other_x, step_size_y;
        reg md_reset;
	reg md_reset_2;

        wire calc_done;
	wire calc_done_2;
	
	wire [11:0] max_iterations;

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

always@(posedge clk_50) begin
	// Zero everything in reset
	if (reset) begin
		arbiter_state <= 8'd_0 ;
		//vga_reset <= 1'b_1 ;
		x_coord <= 10'd_0 ;
		x_coord_2 <= 10'd_1;
	
		y_coord <= 10'd_0 ;
		y_coord_2 <= 10'd_0;
		
		ci <= {4'b0001, 23'd0};
		ci_2 <= {4'b0001, 23'd0};
		
		cr <= {-4'sb0010, 23'd0};
		//cr_2 <= {-4'sb0010, 23'd0} + step_size_x;
		cr_2 <= {4'sb1110 , 23'b00000001001100110011001};
		
//		step_size_x <= {4'b0000, 23'b00000001001100110011001}; // 3/640
//		step_size_y <= {4'b0000, 23'b00000001000100010001000}; // 2/480
		md_reset <= 1'd1;
		md_reset_2 <= 1'd1;
		
	end
	// Otherwiser repeatedly write a large checkerboard to memory
	else begin
		
		if (arbiter_state == 8'd_0) begin
			//vga_reset <= 1'b_0 ;
			write_enable <= 1'b_1 ;
			write_enable_2 <= 1'b_1;
			write_address <= (19'd_320 * y_coord) + (x_coord >> 1);
			write_address_2 <= (19'd_320 * y_coord_2) + (x_coord_2 >> 1);

			if (counter >= max_iterations) begin
			  write_data <= 8'b_000_000_00 ; // black
			end
			else if (counter >= (max_iterations >>> 1)) begin
			  write_data <= 8'b_011_001_00 ; 
			end
			else if (counter >= (max_iterations >>> 2)) begin
			  write_data <= 8'b_011_001_00 ;
			end
			else if (counter >= (max_iterations >>> 3)) begin
			  write_data <= 8'b_101_010_01 ;
			end
			else if (counter >= (max_iterations >>> 4)) begin
			  write_data <= 8'b_011_001_01 ;
			end
			else if (counter >= (max_iterations >>> 5)) begin
			  write_data <= 8'b_001_001_01 ;
			end
			else if (counter >= (max_iterations >>> 6)) begin
			  write_data <= 8'b_011_010_10 ;
			end
			else if (counter >= (max_iterations >>> 7)) begin
			  write_data <= 8'b_010_100_10 ;
			end
			else if (counter >= (max_iterations >>> 8)) begin
			  write_data <= 8'b_010_100_10 ; 
			end
			else begin
			  write_data <= 8'b_010_100_10 ;
			end
			
			if (counter_2 >= max_iterations) begin
			  write_data_2 <= 8'b_000_000_00 ; // black
			end
			else if (counter_2 >= (max_iterations >>> 1)) begin
			  write_data_2 <= 8'b_011_001_00 ; 
			end
			else if (counter_2 >= (max_iterations >>> 2)) begin
			  write_data_2 <= 8'b_011_001_00 ;
			end
			else if (counter_2 >= (max_iterations >>> 3)) begin
			  write_data_2 <= 8'b_101_010_01 ;
			end
			else if (counter_2 >= (max_iterations >>> 4)) begin
			  write_data_2 <= 8'b_011_001_01 ;
			end
			else if (counter_2 >= (max_iterations >>> 5)) begin
			  write_data_2 <= 8'b_001_001_01 ;
			end
			else if (counter_2 >= (max_iterations >>> 6)) begin
			  write_data_2 <= 8'b_011_010_10 ;
			end
			else if (counter_2 >= (max_iterations >>> 7)) begin
			  write_data_2 <= 8'b_010_100_10 ;
			end
			else if (counter_2 >= (max_iterations >>> 8)) begin
			  write_data_2 <= 8'b_010_100_10 ; 
			end
			else begin
			  write_data_2 <= 8'b_010_100_10 ;
			end
			
			if (calc_done) begin
//				x_coord <= (x_coord==10'd_638) ? ((md_reset==1'd1) ? x_coord : 10'd_0 ):((md_reset==1'd1) ? x_coord : x_coord + 10'd_2) ;
				
				x_coord <= (x_coord==10'd_638) ? 10'd_0 : (x_coord + 10'd_2) ;
				y_coord <= (x_coord==10'd_638) ? ((y_coord==10'd_479) ? 10'd_0 : (y_coord+10'd_1)) : y_coord ;
			
				cr <= (x_coord == 10'd_638) ? ({-4'sb0010, 23'd0}) : (cr + step_every_other_x); // x
				ci <= (x_coord==10'd_638) ? ((y_coord==10'd_479) ? ({4'b0001, 23'd0}): (ci - step_size_y)) : ci ; // y

				
				md_reset <= 1'd1;
			end
			else begin
				md_reset <= 1'd0;
				cr <= cr;
				ci <= ci;
				x_coord <= x_coord;
				y_coord<= y_coord;
				write_enable <= 1'b0;
				write_address <= (19'd_320 * y_coord) + (x_coord >> 1);
			end
			
			if (calc_done_2) begin
				x_coord_2 <= (x_coord_2==10'd_639) ? 10'd_1 : (x_coord_2 + 10'd_2) ;
//				x_coord_2 <= (x_coord_2==10'd_639) ? ((md_reset_2==1'd1) ? x_coord_2 : 10'd_1 ):((md_reset_2==1'd_1) ? x_coord_2: x_coord_2 + 10'd_2) ;
				y_coord_2 <= (x_coord_2==10'd_639) ? ((y_coord_2==10'd_479) ? 10'd_0 : (y_coord_2+10'd_1)) : y_coord_2 ;
			
				cr_2 <= (x_coord_2 == 10'd_639) ? ({4'sb1110 , 23'b00000001001100110011001} ) : (cr_2 + step_every_other_x); // x 
				ci_2 <= (x_coord_2==10'd_639) ? ((y_coord_2==10'd_479) ? ({4'b0001, 23'd0}) : (ci_2 - step_size_y)) : ci_2; // y
				
				md_reset_2 <= 1'd1;
			end
			else begin
				md_reset_2 <= 1'd0;
				cr_2 <= cr_2;
				ci_2 <= ci_2;
				x_coord_2 <= x_coord_2;
				y_coord_2 <= y_coord_2;
				write_enable_2 <= 1'b0;
				write_address_2 <= (19'd_320 * y_coord_2) + (x_coord_2 >> 1);
			end
			
			
//			if (flag_counter) begin
//				temp_counter <= running_counter;
//				running_counter <= 32'd0;
//			end
//			else begin 
//				running_counter <= running_counter + 1;
//			end
			
			arbiter_state <= 8'd_0 ;
		end
	end
end


	//assign c_i = {4'd0, 1'd1, 22'd0}; // 0.25
	//assign c_r = {4'd0, 1'd1, 22'd0}; // 0.25

	assign step_every_other_x = ({4'b0000, 23'b00000010011001100110010});
	assign step_size_x = {4'b0000, 23'b00000001001100110011001}; //3/640
	assign step_size_y = {4'b0000, 23'b00000001000100010001000};

	// Instantiation of Device Under Test
    mandelbrot MBROT (.clock(clk_50), 
               .reset(md_reset),
               .c_i(ci), 
			   .c_r(cr), 
			   .max_iter(max_iterations), 
			   .out(counter),
		.done(calc_done)
              );

    mandelbrot MBROT_2 (.clock(clk_50), 
               .reset(md_reset_2),
               .c_i(ci_2), 
			   .c_r(cr_2), 
			   .max_iter(max_iterations), 
			   .out(counter_2),
		.done(calc_done_2)
              );
	
endmodule

//////////////////////////////////////////////////////////////
////////////	Mandelbrot Set Visualizer	    //////////////
//////////////////////////////////////////////////////////////

module mandelbrot (clock, reset, c_i, c_r, max_iter, out, done);
    input clock, reset;
	input signed [26:0] c_i, c_r;
	input [11:0] max_iter;
	output [11:0] out;
	output done;
    
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
