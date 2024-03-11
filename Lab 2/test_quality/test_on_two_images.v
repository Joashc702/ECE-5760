// Mandelbrot Set Visualizer

`timescale 1ns/1ns

module testbench();
reg clk_50, reset;
reg signed [26:0] ci, cr, ci_2, cr_2;
reg [26:0] step_size_x, step_every_other_x, step_size_y;
wire [11:0] max_iter;
wire [11:0] out, out_2;
reg [7:0] arbiter_state;

wire [7:0] write_data, write_data_2;
wire [17:0] write_address, write_address_2;

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

wire calc_done, calc_done_2;
//assign c_i = {4'd0, 1'd1, 22'd0}; // 0.25
//assign c_r = {4'd0, 1'd1, 22'd0}; // 0.25
assign max_iter = 12'd1000;

//always@(posedge clk_50) begin
// Zero everything in reset
//if (reset) begin
//vga_reset <= 1'b_1 ;
//x_coord <= 10'd_0 ;
//y_coord <= 10'd_0 ;
//x_coord_2 <= 10'd_1 ;
//y_coord_2 <= 10'd_0 ; 
//ci <= {4'b0001, 23'd0};
//cr <= {-4'b0010, 23'd0};
//ci_2 <= {4'b0001, 23'd0};
//cr_2 <= {4'sb1110, 23'b00000001001100110011001};

//step_every_other_x <= {4'b0000,23'b00000010011001100110010};
//step_size_x <= {4'b0000,23'b00000001001100110011001}; // 3/640
//step_size_y <= {4'b0000,23'b00000001000100010001000}; // 2/480

//end
// Otherwiser repeatedly write a large checkerboard to memory
//end

// Instantiation of Device Under Test
/*    mandelbrot MBROT (.clock(clk_50),
               .reset(reset),
			   .x_coord_init(10'd_0),
			   .y_coord_init(10'd_0),
               .c_i_init({4'b0001, 23'd0}),
   .c_r_init({-4'b0010, 23'd0}),
   .max_iter(max_iter),
   .out(out),
   .write_addr(write_address),
   .done(calc_done)         );
    
	mandelbrot MBROT_2 (.clock(clk_50),
               .reset(reset),
			   .x_coord_init(10'd_1),
			   .y_coord_init(10'd_0),
               .c_i_init({4'b0001, 23'd0}),
   .c_r_init({4'sb1110, 23'b00000001001100110011001}),
   .max_iter(max_iter),
   .out(out_2),
   .write_addr(write_address_2),
   .done(calc_done_2)         );
  */
    mandelbrot MBROT (.clock(clk_50),
               .reset(reset),
               .c_i_init({4'b0001, 23'd0}),
   .c_r_init({4'b0000, 23'd0}),
   .max_iter(max_iter),
   .out(out),
   .write_addr(write_address),
   .done(calc_done)         );
    
	mandelbrot MBROT_2 (.clock(clk_50),
               .reset(reset),
               .c_i_init({4'b0000, 1'd1, 22'd0}),
   .c_r_init({4'b0000, 1'd1, 22'd0}),
   .max_iter(max_iter),
   .out(out_2),
   .write_addr(write_address_2),
   .done(calc_done_2)         );
  
   // Instantiation of color write data
	color_write color_iterOne(.clk(clk_50), .counter(out), .max_iterations(max_iter), .write_data_out(write_data));
	color_write color_iterTwo(.clk(clk_50), .counter(out_2), .max_iterations(max_iter), .write_data_out(write_data_2));
endmodule


module mandelbrot (clock, reset, c_i_init, c_r_init, max_iter, out, write_addr, done);
    input clock, reset;
	//input [9:0] x_coord_init, y_coord_init;
	input signed [26:0] c_i_init, c_r_init;
	input [11:0] max_iter;
	output [11:0] out;
	output [17:0] write_addr;
	output done;
	
	reg [1:0] state;
	reg [9:0] x_coord, y_coord;
	reg signed [26:0] ci, cr;
	reg [18:0] write_address;
    reg signed [26:0] z_r, z_i;
    reg [11:0] temp_iter;
    wire signed [26:0] zr_sq, zi_sq, zr_zi;
    wire signed [26:0] zr_temp, zi_temp;
	wire [26:0] zr_abs, zi_abs;

   
    // clocked
    always @(posedge clock) begin
        if (reset == 1) begin
            z_r <= 0;
            z_i <= 0;
            temp_iter <= 0;
			ci <= c_i_init;
			cr <= c_r_init;
			state <= 2'd0;
        end
        /*else if (done == 0) begin
            z_r <= zr_temp;
            z_i <= zi_temp;
            temp_iter <= temp_iter + 1;
        end*/
		if (state == 0) begin
            z_r <= 0;
            z_i <= 0;
            temp_iter <= 0;
			state <= 1;
		end
		else if (state == 1) begin
			if (~done) begin
				z_r <= zr_temp;
				z_i <= zi_temp;
				temp_iter <= temp_iter + 1;
				state <= 1;
			end
			else begin
				state <= 2;
			end
		end
		else if (state == 2) begin
			//write_address <= (18'd_640 * y_coord) + (x_coord >> 1);
			/* x_coord <= (x_coord==10'd_638+x_coord_init) ? (10'd_0+x_coord_init):(x_coord + 10'd_2) ;
			y_coord <= (x_coord==10'd_638+x_coord_init) ? ((y_coord==10'd_479)?10'd_0:(y_coord+10'd_1)):y_coord ;
			cr <= (x_coord == 10'd_638+x_coord_init) ? (c_r_init) : (cr + {4'b0000,23'b00000010011001100110010}); // x
			ci <= (x_coord==10'd_638+x_coord_init) ? ((y_coord==10'd_479) ? (c_r_init): (ci - {4'b0000,23'b00000001000100010001000})) : ci ; // y */
			state <= 0;
		end
		
    end
   
    // signed mults
    signed_mult zrSq (.out(zr_sq), .a(z_r), .b(z_r));
    signed_mult ziSq (.out(zi_sq), .a(z_i), .b(z_i));
    signed_mult twoZrZi (.out(zr_zi), .a(z_r), .b(z_i));

    // combinational
    assign zr_temp = zr_sq - zi_sq + cr;
    assign zi_temp = (zr_zi <<< 1) + ci;
	assign zr_abs = (z_r[26] == 1) ? (~z_r + 1) : (z_r);  
	assign zi_abs = (z_i[26] == 1) ? (~z_i + 1) : (z_i);  
    assign done = (zr_abs > {4'd2, 23'd0}) || (zi_abs > {4'd2, 23'd0}) || ((zr_sq + zi_sq) > {4'd4, 23'd0}) || (temp_iter >= max_iter);
	assign out = temp_iter;
	assign write_addr = write_address;
endmodule

module color_write(clk, counter, max_iterations, write_data_out);
	input clk;
	input [11:0] counter, max_iterations;
	output [7:0] write_data_out;

	reg [7:0] write_data;

	always@(posedge clk) begin
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
	end
	
	assign write_data_out = write_data;
endmodule

//////////////////////////////////////////////////
//// signed mult of 4.23 format 2'comp////////////
//////////////////////////////////////////////////
module signed_mult (out, a, b);
output signed  [26:0] out;
input signed [26:0] a;
input signed [26:0] b;
   
// intermediate full bit length
wire signed [53:0] mult_out;
assign mult_out = a * b;
   
// select bits for 4.23 fixed point
assign out = {mult_out[53], mult_out[48:23]};
endmodule
