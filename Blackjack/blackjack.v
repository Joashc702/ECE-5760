
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
	parameter [9:0] num_simul = 5;//half of the number of columns
    wire [7:0] output_random [num_simul-1:0];
	//wire [63:0] seed_samples [5:0];
	wire init_done;
	
	reg [2:0] read_write_offset;
	reg [7:0] write_addr_inter;
	
	
	
	wire signed [3:0] q [num_simul-1:0];
	reg signed [3:0] d [num_simul-1:0];
	reg [9:0] write_addr [num_simul-1:0];
	reg [9:0] read_addr [num_simul-1:0];
	reg we[num_simul-1:0];
	
	reg [2:0] drum_state [num_simul-1:0];
	reg [9:0] index_rows [num_simul-1:0];
	
	
	//Shared m10k
	genvar i;
	generate // generate 30 columns
		for (i = 0; i < num_simul; i = i+1) begin: initCols  
			rand127 random_num(.rand_out(output_random[i]), .seed_in(64'h54555555 ^ i), .clock_in(clk_50), .reset_in(reset));
			reg [3:0] array_hit_card [11:0]; 
			reg [48:0] hit_card_reg;
			reg [3:0] drawn_card_val [11:0];
			reg [3:0] dealer_card;
			wire picked; 
			reg player_result;
			reg [4:0] dealer_hands, player_hands;
			reg [4:0] card_itr;
			reg [3:0] init_samp_counter;
			
			reg init_done;
			
			//assign hit_card_reg = ;
			
			M10K_32_5 m10k_curr(.q(q[i]), .d(d[i]), .write_address(write_addr[i]), .read_address(read_addr[i]), .we(we[i]), .clk(clk_50));
			Search_hit_card card_check( .picked_card(hit_card_reg), 
										.card(output_random[i]), // TODO we don't want generated number all the time
										.picked(picked));
			always @ (posedge clk_50) begin
				if (reset) begin // reset conditions
					drum_state[i] <= 3'd0;
					index_rows[i] <= 10'd0;
					we[i] <= 1'd1;
				end
				else begin
					// STATE 0: Initialization
					if (drum_state[i] == 3'd0) begin // check if initialization has occurred
						if (init_done) begin
							drum_state[i] <= 3'd1;
							index_rows[i] <= 10'd0;
							player_hands <= 5'd0;
							dealer_hands <= 5'd0;
							card_itr <= 4'd0;
							we[i] <= 1'd0;
						end
						else begin
							if (index_rows[i] <= 10'd49) begin
								if (init_samp_counter > 4'd10) begin
									init_samp_counter <= 4'd0;
								end
								write_addr[i] <= index_rows[i];
								d[i] <= init_samp_counter;
								init_samp_counter <= init_samp_counter + 4'd1;
								index_rows[i] <= index_rows[i] + 4'd1;
							end else begin
								init_done <= 1'd1;
							end
							drum_state[i] <= 3'd0;
						end
					end
					// STATE 1: Dealer's card
					else if (drum_state[i] == 3'd1) begin //initiate dealers cards
						drum_state[i] <= 3'd2;
						read_addr[i] <= output_random[i];
						array_hit_card[card_itr] <= output_random[i];	
						card_itr <= card_itr + 4'd1;						
					end
					// STATE 2: Buffer the memory output and ready for the next read operation
					else if (drum_state[i] == 3'd2) begin //buffer state for dealers card and initiate user reads
						if (picked == 1) begin
							read_addr[i] <= output_random[i];
							array_hit_card[card_itr] <= output_random[i];
							card_itr <= card_itr + 4'd1;
						end
						else begin 
							drum_state[i] <= 3'd3;
						end
					end
					// STATE 3: Player's turn
					else if (drum_state[i] == 3'd3) begin //dealer card read and handle transition into stand or hit
						//dealer_card <= q[i];
						drawn_card_val[card_itr - 2] <= q[i];
						card_itr <= card_itr + 4'd1;
						if (picked == 1) begin
							read_addr[i] <= output_random[i];
							array_hit_card[card_itr] <= output_random[i];
						end
						else begin
							//array_hit_card[0] <= q[i]; //TODO

							player_hands <= player_hands + q[i];
							if ((player_hands == 5'd12 && drawn_card_val[0] >= 5'd4 && drawn_card_val[0] <= 5'd6) || (player_hands >= 5'd13 && player_hands < 5'd17 && drawn_card_val[0] >= 5'd2 && drawn_card_val[0] <= 5'd6) || (player_hands >= 5'd17)) begin
								drum_state[i] <= 3'd4; // STAND
							end
							else begin
								drum_state[i] <= 3'd3;	//HIT	
								hit_card_reg <= (1 << array_hit_card[card_itr]);
								read_addr[i] <= output_random[i];
								array_hit_card[card_itr] <= output_random[i];
							end
						end
					end
					// STATE 4: Dealer's turn
					else if (drum_state[i] == 3'd4) begin // Dealer HIT State
						drawn_card_val[card_itr-2] <= q[i];
						card_itr <= card_itr + 4'd1;
						if (picked == 1) begin
							read_addr[i] <= output_random[i];
							array_hit_card[0] <= output_random[i];
						end
						else begin
							dealer_hands <= dealer_hands + q[i];
							if (dealer_hands >= 5'd17) begin
								drum_state[i] <= 3'd5; // Result
							end
							else begin
								drum_state[i] <= 3'd4;	//HIT	
							end
						end
					end
					// STATE 5: Result
					else if (drum_state[i] == 3'd5) begin // Dealer HIT State
					/*
						// TODO: ACE 1/11, plus blackjack case
						if (player_hands > 5'd21) begin
							player_result <= 1'd0;
						end
						else if (player_hands <= 5'd21 && dealer_hands < 5'd21 && player_hands <= dealer_hands) begin
							player_result <= 1'd0;
						end
						else if ((player_hands <= 5'd21 && player_hands > dealer_hands) || (dealer_hands > 5'd21)) begin
							player_result <= 1'd1;
						end
						*/
					end
				end
			end 
			
		end
	endgenerate
   
endmodule



module Search_hit_card(picked_card, card, picked);
	//input clock;
	//input [3:0] picked_cards_arr [11:0];
	input [48:0] picked_card;
	input [9:0]card;
	//reg picked_inter;
	output picked;
    //reg [2:0] check_state; 
	//reg [3:0] arr_itr;
	
	/*always  @ (posedge clock) begin
		if (!check_state) begin
			check_state <= 3'd1;
			arr_itr <= 4'd0;
		end 
		else if check_state == 3'd1 begin
			if (card == array_hit_card[arr_itr]) begin
				picked_inter <= 1'd1;
			end
			
			if (picked == 1'd1) begin
				check_state <= 3'd0;
			end
			else if (picked == 1'd0 && arr_itr ==16) begin
				check_state <= 3'd0;
			end
			else begin
				check_state <= 3'd1;
			end
		end
	end
	
	arr_itr <= arr_itr + 4'd1;
	assign picked = picked_inter;*/
	
	/*assign picked = (picked_cards_arr[0] == card) || (picked_cards_arr[1] == card) || (picked_cards_arr[2] == card) || (picked_cards_arr[3] == card) || 
					(picked_cards_arr[4] == card) || (picked_cards_arr[5] == card) || (picked_cards_arr[6] == card) || (picked_cards_arr[7] == card) || 
					(picked_cards_arr[8] == card) || (picked_cards_arr[9] == card) || (picked_cards_arr[10] == card) || (picked_cards_arr[11] == card);*/
					
	assign picked = ((picked_card) & (1 << card)) >> card;
	
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

//============================================================
// M10K module
//============================================================
module M10K_32_5( 
    output reg [3:0] q,
    input [3:0] d,
    input [9:0] write_address, read_address,
    input we, clk
);
    // force M10K ram style
    // 307200 words of 8 bits
    reg [3:0] mem [511:0]  /* synthesis ramstyle = "no_rw_check, M10K" */;
	 
    always @ (posedge clk) begin
        if (we) begin
            mem[write_address] <= d;
		  end
		  
        q <= mem[read_address]; // q doesn't get d in this clock cycle
    end
endmodule

///////////////////////////////////////////////////////////////////
// SM BJ logic
///////////////////////////////////////////////////////////////////

/*
genvar i; // generation variable
generate // generate columns
	for (i = 0; i < (num_cols+1); i = i+1) begin: initCols // loop from 0 to number of columns
		// curr and prev M10k block instantiations
		M10K_32_5 m10k_curr(.q(q[i]), .d(d[i]), .write_address(write_addr[i]), .read_address(read_addr[i]), .we(we[i]), .clk(CLOCK_50));

		  //potential duplication always block that can be controlled by an if statement			
		  always @(posedge CLOCK_50) begin
			  if(i <= half_num_cols) begin
					if (~KEY[0] || arm_reset) begin
						array_step[i] <= 18'd0; 
					end
					else begin 
						if (i == 10'd0) begin
							array_step[i] <= 18'd0; 
						end
						if(i >= 1 && i <= half_num_cols) begin
							array_step[i] <= array_step[i - 1] + step_size; 
						end	
					end
				end
		  end
	
	//state 1 - is ARM done initializing shared M10k
	//state 2 - each monte carlo simulation then duplicates the shared M10k into its own M10k, parallely update the read's for figuring out what the dealer's hidden card might be
	//state 3 - assess state logic, initiate first read to pipeline potential HIT value, also update dealer's hidden read to get value
	//state 4 - Need to Buffer?, Read potential HIT value again?, store dealer's hidden 
	//state 5 - Enter HIT state and keep reading the value and run through logic
	//state 6 - Even though value may not be necessary, go into stand and then have to account for buffer states to random what the value of the hidden card might be
	//state 7 - combine into a result state to compute to the possibilities
	always @(posedge CLOCK_50) begin // handle SM for each drum node
	    if (~KEY[0] || arm_reset) begin // reset conditions
                drum_state[i] <= 3'd0;
            end
            else begin
                // State 0 - Reset
                if (drum_state[i] == 3'd0) begin
                    drum_state[i] <= 3'd1;
                    index_rows[i] <= 10'd0;
					
					read_addr_dealer[i] <= 10'd0; //actually should be the value generated by 
					write_addr_dealer[i] <= 10'd0;  // write 0 if picked
					
                end
                // State 1 - Init shared M10k
                else if (drum_state[i] == 3'd1) begin
				 //TOGO:duplication also happens here - might need another always block trigger by a sensitivity var adjusted here?
					drum_state[i] <= 3'd2;
					index_rows[i] <= 10'd0;
					we[i] <= 1'd0;
					
					write_addr[i] <= 10'd0; //random gen value
					read_addr[i] <= 10'd0; // randomly generated number could be 0?
					read_addr_dealer[i] <= 10'd1;
					
					// set prev read addr to index_rows value to read prev_u of the current node
					drum_state[i] <= 3'd1;				
                   
                end
                // State 2 - each monte carlo simulation then duplicates the shared M10k into its own M10k, parallely update the read's for figuring out what the dealer's hidden card might be
                else if (drum_state[i] == 3'd2) begin				
					read_addr[i] <= ;//random_num
					dealer_card[i] <= q[i];
					
					//drum_state[i] <= 3'd3;
					//state transition based on user's current value - stand or hit
                end
				else if (drum_state[i] == 3'd3) begin
					user_draw[i] <= q[i];
					read_addr[i] <= ;//random number
				end
                // State 3 - assess state logic, initiate first read to pipeline potential HIT value, also update dealer's hidden read to get value
                else if (drum_state[i] == 3'd3) begin //hit state
					
					//TOGO: Run through game logic check if the none of the stand conditions are met
					we[i] <= 1'd1;
					user_draw[i] <= q[i];
					if (user_draw[i] != 10'd0) begin
						//assess for transition and update read as necessary
					end
					else begin 
						//re-draw card
						//keep updating read address using random value
					end
					read_addr[i] <= //random number;
					//drum_state[i] <= 3'd4;
					
					//TOGO: state transition based on if conditions are met - stay or move to stand
					
                end
				// STATE 4 - Need to Buffer?, Read potential HIT value again?, store dealer's hidden - STAND STATE
                else if (drum_state[i] == 3'd4) begin //stand
					deal_card_draw[i] <= q[i];
					//TOGO add to dealer total and also handle potential bad draws
                end
				// STATE 5 - Enter HIT state and keep reading the value and run through logic
				else if (drum_state[i] == 3'd5) begin
					if (i == half_num_cols) begin // get output val from center of drum
						out_val <= intermed_val; 
					end
					  
					// audio done signal check
					if (sync_signal) begin
						drum_state[i] <= 3'd4;
					end
					else begin
						drum_state[i] <= 3'd5;
					end
				end
				// STATE 6 - Even though value may not be necessary, go into stand and then have to account for buffer states to random what the value of the hidden card might be
				else if () begin
					
				end
				// Combine into a result state to compute to the possibilities
				else if () begin
					
				end
            end
        end
    end
endgenerate
*/