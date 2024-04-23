
`timescale 1ns/1ns

module testbench();
	reg clk_50, reset;
    
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
	wire [5:0] output_random_vals [5:0];
	genvar i;
	generate // generate 30 columns
		for (i = 0; i < 5; i = i+1) begin: initCols 
			wire [5:0] output_random [5:0];
			wire [5:0] test_num;
			assign output_random_vals[i] = {output_random[5][0],output_random[4][0],output_random[3][0],output_random[2][0],output_random[1][0],output_random[0][0]} ;
			//rand127 random_num(.rand_out(output_random), .seed_in(64'h54555555), .clock_in(clk_50), .reset_in(reset));
			rand6 random_num(.rand_out(output_random[0]), .seed_in(6'b101010 ^ i), .clock_in(clk_50), .reset_in(reset));
			rand6 random_num_2(.rand_out(output_random[1]), .seed_in(6'b101011 ^ i), .clock_in(clk_50), .reset_in(reset));
			rand6 random_num_3(.rand_out(output_random[2]), .seed_in(6'b100010 ^ i), .clock_in(clk_50), .reset_in(reset));
			rand6 random_num_4(.rand_out(output_random[3]), .seed_in(6'b111010 ^ i), .clock_in(clk_50), .reset_in(reset));
			rand6 random_num_5(.rand_out(output_random[4]), .seed_in(6'b100110 ^ i), .clock_in(clk_50), .reset_in(reset));
			rand6 random_num_6(.rand_out(output_random[5]), .seed_in(6'b111110 ^ i), .clock_in(clk_50), .reset_in(reset));
			
		end
	endgenerate
    
endmodule

//////////////////////////////////////////////////////////
// 16-bit parallel random number generator ///////////////
//////////////////////////////////////////////////////////
// Algorithm is based on:
// A special-purpose processor for the Monte Carlo simulation of ising spin systems
// A. Hoogland, J. Spaa, B. Selman and A. Compagner
// Journal of Computational Physics
// Volume 51, Issue 2, August 1983, Pages 250-260
//////////////////////////////////////////////////////////

module rand6(rand_out, seed_in, clock_in, reset_in);
	output wire [5:0] rand_out;
	input wire clock_in, reset_in;
	input wire [5:0] seed_in;
	
	reg [5:0] interm_rand;
	
	always @(posedge clock_in)
	begin
		if (reset_in) begin
			interm_rand <= seed_in;
		end
		else begin
			interm_rand <= {interm_rand[4], interm_rand[3], interm_rand[2], interm_rand[1], interm_rand[0], interm_rand[5]^interm_rand[4]};
		end
	end
	assign rand_out = interm_rand;
endmodule
module rand127(rand_out, seed_in, clock_in, reset_in);
	// 16-bit random number on every cycle
	output wire [7:0] rand_out ;
	// the clocks and stuff
	//input wire [3:0] state_in ;
	input wire clock_in, reset_in ;
	input wire [64:1] seed_in; // 128 bits is 32 hex digits 0xffff_ffff_ffff_ffff_ffff_ffff_ffff_ffff
	
	
	reg [8:1] sr1, sr2, sr3, sr4, sr5, sr6, sr7, sr8; 
				//sr9, sr10, sr11, sr12, sr13, sr14, sr15, sr16;
	
	// state names
	parameter react_start= 4'd0 ;

	// generate random numbers	
	assign rand_out = {sr1[7], sr2[7], sr3[7], sr4[7],
							sr5[7], sr6[7], sr7[7], sr8[7]};
							//sr9[7], sr10[7], sr11[7], sr12[7],
							//sr13[7], sr14[7], sr15[7], sr16[7]} ;
							
	always @ (posedge clock_in) //
	begin
		if (reset_in)
		begin	
			//init random number generator 
			sr1 <= seed_in[8:1] ;
			sr2 <= seed_in[16:9] ;
			sr3 <= seed_in[24:17] ;
			sr4 <= seed_in[32:25] ;
			sr5 <= seed_in[40:33] ;
			sr6 <= seed_in[48:41] ;
			sr7 <= seed_in[56:49] ;
			sr8 <= {1'b0, seed_in[63:57]};
			//sr8 <= seed_in[64:57] ;
			/*
			sr9 <= seed_in[72:65] ;
			sr10 <= seed_in[80:73] ;
			sr11 <= seed_in[88:81] ;
			sr12 <= seed_in[96:89] ;
			sr13 <= seed_in[104:97] ;
			sr14 <= seed_in[112:105] ;
			sr15 <= seed_in[120:113] ;
			sr16 <= {1'b0,seed_in[127:121]} ;*/
		end
		
		// update 127-bit shift register
		// 16 times in parallel
		else 
		begin
			//if(state_in == react_start) 
			//begin
				sr1 <= {sr1[7:1], sr1[7]^sr8[7]} ;
				sr2 <= {sr2[7:1], sr2[7]^sr1[8]}  ;
				sr3 <= {sr3[7:1], sr3[7]^sr2[8]}  ;
				sr4 <= {sr4[7:1], sr4[7]^sr3[8]}  ;
				sr5 <= {sr5[7:1], sr5[7]^sr4[8]}  ;
				sr6 <= {sr6[7:1], sr6[7]^sr5[8]}  ;
				sr7 <= {sr7[7:1], sr7[7]^sr6[8]}  ;
				sr8 <= {sr8[7:1], sr8[7]^sr7[8]}  ;
				//sr9 <= {sr9[7:1], sr9[7]^sr8[8]}  ;
				/*
				sr10 <= {sr10[7:1], sr10[7]^sr9[8]}  ;
				sr11 <= {sr11[7:1], sr11[7]^sr10[8]}  ;
				sr12 <= {sr12[7:1], sr12[7]^sr11[8]}  ;
				sr13 <= {sr13[7:1], sr13[7]^sr12[8]}  ;
				sr14 <= {sr14[7:1], sr14[7]^sr13[8]}  ;
				sr15 <= {sr15[7:1], sr15[7]^sr14[8]}  ;
				sr16 <= {sr16[6:1], sr16[7]^sr15[8]}  ;*/
			//end	
		end
	end
endmodule


