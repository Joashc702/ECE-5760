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
    wire signed [17:0] q, q_prev; // read value
	reg signed [17:0] d, d_prev;  // write value
	reg [4:0] write_addr, write_addr_prev;
	reg [4:0] read_addr, read_addr_prev;
	reg we, we_prev;
    
    // Drum node variables
	reg signed [17:0] curr_reg;
    reg signed [17:0] down_reg, bottom_reg;
    wire [17:0] rho_eff_init;
    wire signed [17:0] next_u;

    // index rows
    reg [4:0] index_rows;
	reg [4:0] write_rows;
	//reg [4:0] index_rows_prev;
    
    // increment/decrement between nodes
    wire [17:0] step_size;

	// intermediate drum instantiation flag check
	wire signed [17:0] up_check;
	wire signed [17:0] down_check; 
	wire signed [17:0] curr_check;
	
	assign rho_eff_init = {1'b0, 17'b00010000000000000};
	assign up_check = (index_rows >= 5'd29) ? 18'b0 : q;      // checks if up node reaches top, otherwise increments
	assign down_check = (index_rows == 5'd0) ? 18'b0 : down_reg; // checks if down node is at bottom, otherwise increments
	assign curr_check = (index_rows == 5'd0) ? bottom_reg : curr_reg; // checks bottom_u for compute module

// genvar i;
// generate 
// for (i = 0; i < 1; i = i+1) begin: initCols
    // curr and prev M10k block instantiations
    M10K_32_5 m10k_curr(.q(q), .d(d), .write_address(write_addr), .read_address(read_addr), .we(we), .clk(clk_50));
    M10K_32_5 m10k_prev(.q(q_prev), .d(d_prev), .write_address(write_addr_prev), .read_address(read_addr_prev), .we(we_prev), .clk(clk_50));
    
    // drum instantiation
    drum oneNode (.clk(clk_50), 
				  .rho_eff(rho_eff_init),
				  .curr_u(curr_reg),
				  .prev_u(q_prev),
				  .u_left(18'd0),
				  .u_right(18'd0),
				  .u_up(q),
				  .u_down(down_reg),
				  .next(next_u));
    
	// state variables
    reg [2:0] state;
    reg signed [17:0] initial_val;
    reg signed [17:0] intermed_val;
    reg signed [17:0] out_val;
	
	//assign initial_val = {1'b0, 17'b00000000000000000}; // 0
	//assign step_size = {1'b0, 17'b00000111111111111};   // (1/8) / 32
    assign step_size = {1'b0, 17'b00000010000000000};   // (1/8) / 16
	
    always @(posedge clk_50) begin
        if (reset) begin
            state <= 3'd0;
        end
        else begin
            // State 0 - Reset
            if (state == 3'd0) begin
                state <= 3'd1;
				initial_val <= {1'b0, 17'b00000000000000000};
                index_rows <= 5'd0;
				write_rows <= 5'd0;
				//index_rows_prev <= 5'd0;
				bottom_reg <= {1'b0, 17'b00000000000000000};
            end
            // State 1 - Init
            else if (state == 3'd1) begin
                // once all 30 nodes are init
                if (index_rows == 5'd30) begin // if it exceeds the top
					index_rows <= 5'd0;
					read_addr <=  5'd1; // sets read address to this if node isn't the last one  
					read_addr_prev <= 5'd0; // set prev read addr to index_rows value to read prev_u of the current node
                    we <= 0; // make sure you're reading
					we_prev <= 0; // make sure you're reading
					state <= 3'd2;
				
                end
                else begin // init curr node and prev_u M10k blocks
                    if (index_rows < 5'd15) begin
                        we <= 1'd1;
                        we_prev <= 1'd1;
                        read_addr <= index_rows;
                        read_addr_prev <= index_rows;
                        write_addr <= index_rows;
                        write_addr_prev <= index_rows;
                        d <= initial_val;
                        d_prev <= initial_val;
						/*
						if (index_rows == 5'd0) begin
							bottom_reg <= initial_val;
						end
						*/
                        initial_val <= initial_val + step_size;
                    end
                    else begin
                        we <= 1'd1;
                        we_prev <= 1'd1;
                        read_addr <= index_rows;
                        read_addr_prev <= index_rows;
                        write_addr <= index_rows;
                        write_addr_prev <= index_rows;
                        d <= initial_val;
                        d_prev <= initial_val;
                        initial_val <= initial_val - step_size;
                    end
                    
                    index_rows <= index_rows + 5'd1; // increment the index to check rows
                    state <= 3'd1;
                end
            end
			else if (state == 3'd2) begin
					read_addr <=  5'd2;   
					read_addr_prev <= 5'd1; 
					state <= 3'd3;
					write_addr <= 5'd0;
					write_addr_prev <= 5'd0;
			end
			// State 2 -> bottom case + prepare read/write for the next cycle
			else if (state == 3'd3) begin
				bottom_reg <= next_u;
				d_prev <= bottom_reg;
				down_reg <= bottom_reg;
				curr_reg <= q;
				read_addr <=  read_addr + 5'd1; // sets read address to this if node isn't the last one  
				read_addr_prev <= read_addr_prev + 5'd1; // set prev read addr to index_rows value to read prev_u of the current node
				write_addr <= write_addr + 5'd1;
       			write_addr_prev <= write_addr_prev + 5'd1;
				we <= 1; // make sure you're reading
				we_prev <= 1; // make sure you're reading
				state <= 3'd4;
			end
			// State 3 -> move up case + prepare
			else if (state == 3'd4) begin
				d <= next_u;
				down_reg <= curr_reg;
				d_prev <= curr_reg;
				curr_reg <= q;
				read_addr <= (read_addr == 5'd29) ?  read_addr : read_addr + 5'd1; // sets read address to this if node isn't the last one  
				read_addr_prev <= read_addr_prev + 5'd1; // set prev read addr to index_rows value to read prev_u of the current node
				write_addr <= write_addr + 5'd1;
                write_addr_prev <= write_addr_prev + 5'd1;
				state <= (read_addr_prev == 5'd28) ? 3'd5 : 3'd4;
			end
			// State 4 -> top case
			else if (state == 3'd5) begin
				d <= next_u;
				d_prev <= curr_reg;
				read_addr <= 5'd1;
				read_addr_prev <= 5'd0;  
				state <= 3'd2;
				write_addr <= write_addr + 5'd1;
                write_addr_prev <= write_addr_prev + 5'd1;
			end
			
			
/*
            // State 2 - Set up read/write address for M10k blocks
            else if (state == 3'd2) begin
                if (index_rows < 5'd30) begin // if not at top
					read_addr <= (index_rows == 5'd29) ? index_rows : index_rows + 5'd1; // sets read address to this if node isn't the last one  
					read_addr_prev <= index_rows; // set prev read addr to index_rows value to read prev_u of the current node
					index_rows <= index_rows +1;
                    we <= 0; // make sure you're reading
					we_prev <= 0; // make sure you're reading
				end
				else begin
					index_rows <= (write_rows == 5'd29) ? 5'd0 : index_rows;
				end
				
				if(index_rows > 1)begin
					write_addr <= (write_rows < 5'd30) ? write_rows : 5'd0;	
					write_addr_prev <= (write_rows < 5'd30) ? write_rows : 5'd0;
					write_rows <= (write_rows < 5'd29) ? write_rows + 5'd1 : 5'd0;
					we <= 1; // make sure you're reading
					we_prev <= 1; // make sure you're reading
					if(write_rows == 5'd0) begin
						bottom_reg <= next_u;
						down_reg <= bottom_reg;
						curr_reg <= up_check;
						d_prev <= bottom_reg;
					end
				end
				state <= 3'd3;
            end
			// State 3
            else if (state == 3'd3) begin
				if (write_rows == 29) begin
					d <= next_u;
					d_prev <= curr_reg;
				end
				else begin
					d <= next_u;
					down_reg <= curr_reg;
					d_prev <= curr_reg;
					curr_reg <= up_check;	
				end

				state <= 3'd4;
                //prev_u <= q_prev; // q_prev would be data read from memory that stores prev state of curr node -> I think it's curr u 
               
            end
			
			else if (state == 3'd4) begin
				if (index_rows == 5'd15) begin
					out_val <= curr_reg;
				end

				//index_rows <= index_rows + 1;
				state <= 3'd2;
			end
*/



				/*if (index_rows == 5'd30 && write_rows == 5'd29) begin
					index_rows <= 5'd0;
					state <= 3'd2;
				end
				else begin
					index_rows <= (index_rows == 5'd30) ? index_rows : index_rows + 1;
					state <= 3'd2;
				end*/

			//else if (state == 3'd6) begin
			//	out_val <= intermed_val; // outputs amplitude
            //    state <= 3'd2;
			//end
            
			/*// State 4 - Get out (next node) and write and set up next row
            else if (state == 3'd5) begin
                we <= 1'd1;
                write_addr <= index_rows;
                d <= next_u;
                
                we_prev <= 1'd1;
                write_addr_prev <= index_rows;
                d_prev <= (index_rows == 5'd0) ? bottom_reg : curr_reg;
                
                if (index_rows == 5'd15) begin
					intermed_val <= curr_reg;
				end
				if (index_rows == 5'd0) begin
					bottom_reg <= next_u;
				end
				
				//down_reg <= curr_reg;
				//curr_reg <= u_up;
				
				if (index_rows < 5'd29) begin
					down_reg <= (index_rows == 5'd0) ? bottom_reg : curr_reg;
					curr_reg <= u_up;
					index_rows <= index_rows + 5'd1;
					state <= 3'd2;
				end
				else begin 
					index_rows_prev <= index_rows_prev + 5'd1;
					index_rows <= 5'd0;
					state <= 3'd6;
				end
            end
            // State 5 - Output value
            else if (state == 3'd6) begin
                out_val <= intermed_val; // outputs amplitude
                state <= 3'd2;
            end
			*/
        end
    end
// end (for loop)
// endgenerate
	
endmodule

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
	reg [4:0] inter; 
	 
    always @ (posedge clk) begin
        if (we) begin
            mem[write_address] <= d;
		  end
		inter <= read_address;
        q <= mem[inter]; // q doesn't get d in this clock cycle
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