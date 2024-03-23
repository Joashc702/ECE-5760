// Joash Shankar, Ming He, Nikhil Pillai
// ECE 5760 Lab 3 Spring 2024
// Multiprocessor Drum Synthesis

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
    
    // M10K variables
    wire signed [17:0] q [29:0];
	wire signed [17:0] q_prev [29:0]; // read value
	reg signed [17:0] d [29:0];
	reg signed [17:0] d_prev [29:0];  // write value
	reg [4:0] write_addr [29:0];
	reg [4:0] write_addr_prev [29:0];
	reg [4:0] read_addr [29:0];
	reg [4:0] read_addr_prev [29:0];
	reg we[29:0], we_prev[29:0];
    
    // Drum node variables
	reg signed [17:0] curr_reg [29:0];
	reg signed [17:0] prev_u [29:0];
	//reg signed [17:0] curr_left [29:0];
	//reg signed [17:0] curr_right [29:0];
    reg signed [17:0] u_up [29:0];
	reg signed [17:0] down_reg [29:0];
	reg signed [17:0] bottom_reg [29:0];
    wire [17:0] rho_eff_init;
    wire signed [17:0] next_u [29:0];
	reg signed [17:0] initial_val_test[29:0];
	reg signed [17:0] out_val;

    // index rows
    reg [4:0] index_rows [29:0];
	// state variables
	reg [2:0] state [29:0];
    
    // increment/decrement between nodes
    wire [17:0] step_size;

    // store initial amplitude values for all the nodes
    wire signed [17:0] initial_ampl [29:0];

	reg [31:0] counter;
	reg [31:0] counter_reg;
	
	//assign rho_eff_init = {1'b0, 17'b00010000000000000};
	
	//wire signed [17:0] u_G;
	//	input signed [17:0] out_val;

	wire signed [17:0] u_G;
	wire signed [17:0] center_node_shift;
	assign center_node_shift = out_val >>> 4;
	signed_mult u_mult_G(.out(u_G), .a(center_node_shift), .b(center_node_shift)); 
	assign rho_eff_init = ({1'b0, 17'b01111101011100001} < ({1'b0, 17'b01000000000000000} + u_G)) ? {1'b0, 17'b01111101011100001} : ({1'b0, 17'b01000000000000000} + u_G);

	//signed_mult u_mult_G(.out(u_G), .a(out_val>>>4), .b(out_val>>>4)); 
	//assign rho_eff_init = ({1'b0, 17'b01111101011100001} < ({1'b0, 17'b01000000000000000} + u_G)) ? {1'b0, 17'b01111101011100001} : ({1'b0, 17'b01000000000000000} + u_G); // 0.49 < 0.25 + ug ? 0.49 : 0.25 + ug
	// TODO signed_mult for handling rho being nonlinear somehow
	
	//rho_calc rho_eff_calc(.out_val(((index_rows[15] == 5'd15) ? out_val : curr_reg[15])), 
	//				      .rho_eff(rho_eff_init[15]));

genvar i;
generate
	for (i = 0; i < 32; i = i+1) begin: initCols
        // curr and prev M10k block instantiations
        M10K_32_5 m10k_curr(.q(q[i]), .d(d[i]), .write_address(write_addr[i]), .read_address(read_addr[i]), .we(we[i]), .clk(clk_50));
        M10K_32_5 m10k_prev(.q(q_prev[i]), .d(d_prev[i]), .write_address(write_addr_prev[i]), .read_address(read_addr_prev[i]), .we(we_prev[i]), .clk(clk_50));
        //init_pyramid pyramid_init(.out(initial_ampl[i]), .index_cols(i[4:0]), .index_rows(index_rows[i]), .num_of_cols(5'd29), .num_of_rows(5'd29), .step_size(step_size));

        // drum instantiation
        drum oneNode (.clk(clk_50),
                      .rho_eff(rho_eff_init),
                      .curr_u((index_rows[i] == 5'd0) ? bottom_reg[i] : curr_reg[i]), // curr_check
                      .prev_u(prev_u[i]),
                      .u_left((i == 0) ? 0 :((index_rows[i] == 5'd0) ? bottom_reg[i - 1] : curr_reg[i-1])), // TODO left check //(i == 0) ? 0 : curr_reg[i-1]
                      .u_right((i == 29) ? 0 :((index_rows[i] == 5'd0) ? bottom_reg[i + 1] : curr_reg[i+1])), // TODO right check // (i == 29) ? 0 : curr_reg[i+1]
                      .u_up((index_rows[i] == 5'd29) ? 5'b0 : u_up[i]),               // up_check
                      .u_down((index_rows[i] == 5'd0) ? 5'b0 : down_reg[i]),          // down_check
                      .next(next_u[i]));
					 
		//rho_calc rho_eff_calc(.out_val(((index_rows[15] == 5'd15) ? out_val : curr_reg[15])), 
		//			          .rho_eff(rho_eff_init[15]));
        
		// reg signed [17:0] initial_val; // This won't be used for week 2 and we gonna need the new module for initial values (like it says on website)
        reg signed [17:0] intermed_val;
        
		reg signed [17:0] initial_val;

        assign step_size = {1'b0, 17'b00000010000000000};   // (1/8) / 16

        always @(posedge clk_50) begin
            if (reset) begin
                state[i] <= 3'd0;
            end
            else begin
                // State 0 - Reset
                if (state[i] == 3'd0) begin
                    state[i] <= 3'd1;
                    // initial_val <= {1'b0, 17'b00000000000000000};
                    index_rows[i] <= 5'd0;
					initial_val <= 18'd0;
					out_val<= 18'd0;
					counter <= 32'd0;
					counter_reg <= 32'd0;
					u_up[i] <= 18'd0;
					
                end
                // State 1 - Init
                else if (state[i] == 3'd1) begin
                    // once all 30 nodes are init
                    if (index_rows[i] == 5'd30) begin // if it exceeds the top
                        state[i] <= 3'd2;
                        index_rows[i] <= 5'd0;
						
						//we[i] <= 1'd1;
						write_addr[i] <= 5'd0;

						//we_prev[i] <= 1'd1;
						write_addr_prev[i] <= 5'd0;
						
						read_addr[i] <= 5'd1;
						read_addr_prev[i] <= 5'd0; // set prev read addr to index_rows value to read prev_u of the current node
                    end
                    else begin // init curr node and prev_u M10k blocks
                        we[i] <= 1'd1;
						we_prev[i] <= 1'd1;
						read_addr[i] <= index_rows[i];
						read_addr_prev[i] <= index_rows[i];
						write_addr[i] <= index_rows[i];
						write_addr_prev[i] <= index_rows[i];
						d[i] <= initial_val;
						d_prev[i] <= initial_val;
						//new initialization
						if (i < 15) begin
							if (index_rows[i] < i) begin 
								initial_val <= initial_val + step_size;
							end
							else if ((29 - index_rows[i]) < i) begin
								initial_val <= initial_val - step_size;
							end
						end
						else if (i >= 15) begin
							if (index_rows[i] <= (30 - i)) begin
								initial_val <= initial_val + step_size;
							end
							else if ((29 - index_rows[i]) < (30 - i)) begin
								initial_val <= initial_val - step_size;
							end
						end
                        if (index_rows[i] == 5'd0) begin
                            bottom_reg[i] <= initial_val; //change made here
                        end
						if (index_rows[i] == 5'd15) begin
							initial_val_test[i] <= initial_val;
						end
                        index_rows[i] <= index_rows[i] + 5'd1; // increment the index to check rows
                        state[i] <= 3'd1;
                    end
                end
                // State 2 - Set up read address for M10k blocks
                else if (state[i] == 3'd2) begin				
					read_addr[i] <= 5'd2;
                    read_addr_prev[i] <= 5'd1; // set prev read addr to index_rows value to read prev_u of the current node
					counter <= counter + 1;

                    state[i] <= 3'd3;
                end
                // State 3 - wait for M10K to see the read_addr
                else if (state[i] == 3'd3) begin
					read_addr[i] <= 5'd3;
                    read_addr_prev[i] <= 5'd2; // set prev read addr to index_rows value to read prev_u of the current node
					we[i] <= 1'd1;
					we_prev[i] <= 1'd1;
                    state[i] <= 3'd4;
					counter <= counter + 1;
                end
                // State 4 - Get out (next node) and write and set up next row
                else if (state[i] == 3'd4) begin
					read_addr[i] <= (read_addr[i] == 5'd29) ? 5'd0 : read_addr[i] + 5'd1;
                    read_addr_prev[i] <= (read_addr_prev[i] == 5'd29) ? 5'd0 : read_addr_prev[i] + 5'd1; // set prev read addr to index_rows value to read prev_u of the current node
					
					u_up[i] <= (index_rows[i] < 5'd29) ? q[i] : u_up[i]; // since each node's next state depends on its own state and the one above it
                    prev_u[i] <= q_prev[i]; // q_prev would be data read from memory that stores prev state of curr node
					
					counter <= counter + 1;
				
                    write_addr[i] <= (write_addr[i] == 5'd29) ? 5'd0 : index_rows[i] + 5'd1;
                    d[i] <= next_u[i];

                    write_addr_prev[i] <= (write_addr_prev[i] == 5'd29) ? 5'd0 : index_rows[i] + 5'd1;
                    d_prev[i] <= (index_rows[i] == 5'd0) ? bottom_reg[i] : curr_reg[i];

                    if ((index_rows[i] == 5'd15) && i == 15) begin
                        out_val <= curr_reg[i];
                    end
                    if (index_rows[i] == 5'd0) begin
                        bottom_reg[i] <= next_u[i];
                    end
					
                    down_reg[i] <= (index_rows[i] == 5'd0) ? 0 : curr_reg[i];
                    curr_reg[i] <= u_up[i];
                    index_rows[i] <=(index_rows[i] == 5'd29)? 5'd0 : index_rows[i] + 5'd1;
                    state[i] <= 3'd4;

                end
            end
        end
    end
endgenerate
endmodule

//module rho_calc(out_val, rho_eff);
//	input signed [17:0] out_val;
//	output [17:0] rho_eff;
//	wire signed [17:0] u_G;
//	signed_mult u_mult_G(.out(u_G), .a(out_val>>>4), .b(out_val>>>4)); 
//	assign rho_eff = ({1'b0, 17'b01111101011100001} < ({1'b0, 17'b01000000000000000} + u_G)) ? {1'b0, 17'b01111101011100001} : ({1'b0, 17'b01000000000000000} + u_G);
	//assign rho_eff = {1'b0, 17'b01111101011100001};// 0.49 < 0.25 + ug ? 0.49 : 0.25 + ug
//endmodule

//////////////////////////////////////////////////////////////
////////////	Mandelbrot Set Visualizer	    //////////////
//////////////////////////////////////////////////////////////
module drum (clk, rho_eff, curr_u, prev_u, u_left, u_right, u_up, u_down, next); 
	input clk;
    input [17:0] rho_eff;
	input signed [17:0] curr_u, prev_u;
	//input signed [17:0] init_condition; // u
	input signed [17:0] u_left, u_right, u_up, u_down;
	output signed [17:0] next;
	wire signed [17:0] u_sum, rho_usum, inter_val;
	assign u_sum = u_left - curr_u + u_right - curr_u + u_up - curr_u + u_down - curr_u; // (curr_u <<< 2)
	signed_mult rho_mult_usum(.out(rho_usum), .a(u_sum), .b(rho_eff)); 
	assign inter_val = rho_usum + (curr_u <<< 1) - prev_u + (prev_u >>> 10);
	assign next = inter_val - (inter_val >>> 9);
endmodule

//////////////////////////////////////////////////////////////
////////////	Init Amplitude Calculation	    //////////////
//////////////////////////////////////////////////////////////
/*module init_pyramid(out, index_cols, index_rows, num_of_cols, num_of_rows, step_size);
    output signed [17:0] out;
    input [4:0] index_cols, index_rows;   // i, j
    input [4:0] num_of_cols, num_of_rows; // i, j
    input [17:0] step_size;
    
    wire [4:0] horiz_edge_check, vert_edge_check, min_dist;
    
    // calculate distances for cols, rows to edge of grid
    assign horiz_edge_check = ((num_of_cols - index_cols) < index_cols) ? (num_of_cols - index_cols) : index_cols; 
    assign vert_edge_check = ((num_of_rows - index_rows) < index_rows) ? (num_of_rows - index_rows) : index_rows;
    
    // find min of two distances
    assign min_dist = (horiz_edge_check >= vert_edge_check) ? vert_edge_check : horiz_edge_check;
    
    // output is min distance to nearest edge * step size
    assign out = (min_dist + 1) * step_size;
endmodule*/
/*
module init_pyramid(out, index_cols, index_rows, step_size, peak);
	output signed [17:0] out;
    input [4:0] index_cols, index_rows; 
	input [17:0] step_size, peak;
	wire [17:0] max_val_pos;
	//assign max_val_pos = step_size * index_cols;
	assign out = (step_size
	assign out = ((step_size * index_cols) >= peak) ? (step_size * index_cols) : (peak);
endmodule*/ 
//============================================================
// M10K module
//============================================================
module M10K_32_5( 
    output reg [17:0] q,
    input [17:0] d,
    input [4:0] write_address, read_address,
    input we, clk
);
    // force M10K ram style
    // 307200 words of 8 bits
    reg [17:0] mem [31:0]  /* synthesis ramstyle = "no_rw_check, M10K" */;
	reg [4:0] inter_reg;	
	
    always @ (posedge clk) begin
        if (we) begin
            mem[write_address] <= d;
		  end
		inter_reg <= read_address;  
        q <= mem[inter_reg]; // q doesn't get d in this clock cycle
    end
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
