// Blackjack COMBINED code

module DE1_SoC_Computer (
	////////////////////////////////////
	// FPGA Pins
	////////////////////////////////////

	// Clock pins
	CLOCK_50,
	CLOCK2_50,
	CLOCK3_50,
	CLOCK4_50,

	// ADC
	ADC_CS_N,
	ADC_DIN,
	ADC_DOUT,
	ADC_SCLK,

	// Audio
	AUD_ADCDAT,
	AUD_ADCLRCK,
	AUD_BCLK,
	AUD_DACDAT,
	AUD_DACLRCK,
	AUD_XCK,

	// SDRAM
	DRAM_ADDR,
	DRAM_BA,
	DRAM_CAS_N,
	DRAM_CKE,
	DRAM_CLK,
	DRAM_CS_N,
	DRAM_DQ,
	DRAM_LDQM,
	DRAM_RAS_N,
	DRAM_UDQM,
	DRAM_WE_N,

	// I2C Bus for Configuration of the Audio and Video-In Chips
	FPGA_I2C_SCLK,
	FPGA_I2C_SDAT,

	// 40-Pin Headers
	GPIO_0,
	GPIO_1,
	
	// Seven Segment Displays
	HEX0,
	HEX1,
	HEX2,
	HEX3,
	HEX4,
	HEX5,

	// IR
	IRDA_RXD,
	IRDA_TXD,

	// Pushbuttons
	KEY,

	// LEDs
	LEDR,

	// PS2 Ports
	PS2_CLK,
	PS2_DAT,
	
	PS2_CLK2,
	PS2_DAT2,

	// Slider Switches
	SW,

	// Video-In
	TD_CLK27,
	TD_DATA,
	TD_HS,
	TD_RESET_N,
	TD_VS,

	// VGA
	VGA_B,
	VGA_BLANK_N,
	VGA_CLK,
	VGA_G,
	VGA_HS,
	VGA_R,
	VGA_SYNC_N,
	VGA_VS,

	////////////////////////////////////
	// HPS Pins
	////////////////////////////////////
	
	// DDR3 SDRAM
	HPS_DDR3_ADDR,
	HPS_DDR3_BA,
	HPS_DDR3_CAS_N,
	HPS_DDR3_CKE,
	HPS_DDR3_CK_N,
	HPS_DDR3_CK_P,
	HPS_DDR3_CS_N,
	HPS_DDR3_DM,
	HPS_DDR3_DQ,
	HPS_DDR3_DQS_N,
	HPS_DDR3_DQS_P,
	HPS_DDR3_ODT,
	HPS_DDR3_RAS_N,
	HPS_DDR3_RESET_N,
	HPS_DDR3_RZQ,
	HPS_DDR3_WE_N,

	// Ethernet
	HPS_ENET_GTX_CLK,
	HPS_ENET_INT_N,
	HPS_ENET_MDC,
	HPS_ENET_MDIO,
	HPS_ENET_RX_CLK,
	HPS_ENET_RX_DATA,
	HPS_ENET_RX_DV,
	HPS_ENET_TX_DATA,
	HPS_ENET_TX_EN,

	// Flash
	HPS_FLASH_DATA,
	HPS_FLASH_DCLK,
	HPS_FLASH_NCSO,

	// Accelerometer
	HPS_GSENSOR_INT,
		
	// General Purpose I/O
	HPS_GPIO,
		
	// I2C
	HPS_I2C_CONTROL,
	HPS_I2C1_SCLK,
	HPS_I2C1_SDAT,
	HPS_I2C2_SCLK,
	HPS_I2C2_SDAT,

	// Pushbutton
	HPS_KEY,

	// LED
	HPS_LED,
		
	// SD Card
	HPS_SD_CLK,
	HPS_SD_CMD,
	HPS_SD_DATA,

	// SPI
	HPS_SPIM_CLK,
	HPS_SPIM_MISO,
	HPS_SPIM_MOSI,
	HPS_SPIM_SS,

	// UART
	HPS_UART_RX,
	HPS_UART_TX,

	// USB
	HPS_CONV_USB_N,
	HPS_USB_CLKOUT,
	HPS_USB_DATA,
	HPS_USB_DIR,
	HPS_USB_NXT,
	HPS_USB_STP
);

//=======================================================
//  PARAMETER declarations
//=======================================================


//=======================================================
//  PORT declarations
//=======================================================

////////////////////////////////////
// FPGA Pins
////////////////////////////////////

// Clock pins
input						CLOCK_50;
input						CLOCK2_50;
input						CLOCK3_50;
input						CLOCK4_50;

// ADC
inout						ADC_CS_N;
output					ADC_DIN;
input						ADC_DOUT;
output					ADC_SCLK;

// Audio
input						AUD_ADCDAT;
inout						AUD_ADCLRCK;
inout						AUD_BCLK;
output					AUD_DACDAT;
inout						AUD_DACLRCK;
output					AUD_XCK;

// SDRAM
output 		[12: 0]	DRAM_ADDR;
output		[ 1: 0]	DRAM_BA;
output					DRAM_CAS_N;
output					DRAM_CKE;
output					DRAM_CLK;
output					DRAM_CS_N;
inout			[15: 0]	DRAM_DQ;
output					DRAM_LDQM;
output					DRAM_RAS_N;
output					DRAM_UDQM;
output					DRAM_WE_N;

// I2C Bus for Configuration of the Audio and Video-In Chips
output					FPGA_I2C_SCLK;
inout						FPGA_I2C_SDAT;

// 40-pin headers
inout			[35: 0]	GPIO_0;
inout			[35: 0]	GPIO_1;

// Seven Segment Displays
output		[ 6: 0]	HEX0;
output		[ 6: 0]	HEX1;
output		[ 6: 0]	HEX2;
output		[ 6: 0]	HEX3;
output		[ 6: 0]	HEX4;
output		[ 6: 0]	HEX5;

// IR
input						IRDA_RXD;
output					IRDA_TXD;

// Pushbuttons
input			[ 3: 0]	KEY;

// LEDs
output		[ 9: 0]	LEDR;

// PS2 Ports
inout						PS2_CLK;
inout						PS2_DAT;

inout						PS2_CLK2;
inout						PS2_DAT2;

// Slider Switches
input			[ 9: 0]	SW;

// Video-In
input						TD_CLK27;
input			[ 7: 0]	TD_DATA;
input						TD_HS;
output					TD_RESET_N;
input						TD_VS;

// VGA
output		[ 7: 0]	VGA_B;
output					VGA_BLANK_N;
output					VGA_CLK;
output		[ 7: 0]	VGA_G;
output					VGA_HS;
output		[ 7: 0]	VGA_R;
output					VGA_SYNC_N;
output					VGA_VS;



////////////////////////////////////
// HPS Pins
////////////////////////////////////
	
// DDR3 SDRAM
output		[14: 0]	HPS_DDR3_ADDR;
output		[ 2: 0]  HPS_DDR3_BA;
output					HPS_DDR3_CAS_N;
output					HPS_DDR3_CKE;
output					HPS_DDR3_CK_N;
output					HPS_DDR3_CK_P;
output					HPS_DDR3_CS_N;
output		[ 3: 0]	HPS_DDR3_DM;
inout			[31: 0]	HPS_DDR3_DQ;
inout			[ 3: 0]	HPS_DDR3_DQS_N;
inout			[ 3: 0]	HPS_DDR3_DQS_P;
output					HPS_DDR3_ODT;
output					HPS_DDR3_RAS_N;
output					HPS_DDR3_RESET_N;
input						HPS_DDR3_RZQ;
output					HPS_DDR3_WE_N;

// Ethernet
output					HPS_ENET_GTX_CLK;
inout						HPS_ENET_INT_N;
output					HPS_ENET_MDC;
inout						HPS_ENET_MDIO;
input						HPS_ENET_RX_CLK;
input			[ 3: 0]	HPS_ENET_RX_DATA;
input						HPS_ENET_RX_DV;
output		[ 3: 0]	HPS_ENET_TX_DATA;
output					HPS_ENET_TX_EN;

// Flash
inout			[ 3: 0]	HPS_FLASH_DATA;
output					HPS_FLASH_DCLK;
output					HPS_FLASH_NCSO;

// Accelerometer
inout						HPS_GSENSOR_INT;

// General Purpose I/O
inout			[ 1: 0]	HPS_GPIO;

// I2C
inout						HPS_I2C_CONTROL;
inout						HPS_I2C1_SCLK;
inout						HPS_I2C1_SDAT;
inout						HPS_I2C2_SCLK;
inout						HPS_I2C2_SDAT;

// Pushbutton
inout						HPS_KEY;

// LED
inout						HPS_LED;

// SD Card
output					HPS_SD_CLK;
inout						HPS_SD_CMD;
inout			[ 3: 0]	HPS_SD_DATA;

// SPI
output					HPS_SPIM_CLK;
input						HPS_SPIM_MISO;
output					HPS_SPIM_MOSI;
inout						HPS_SPIM_SS;

// UART
input						HPS_UART_RX;
output					HPS_UART_TX;

// USB
inout						HPS_CONV_USB_N;
input						HPS_USB_CLKOUT;
inout			[ 7: 0]	HPS_USB_DATA;
input						HPS_USB_DIR;
input						HPS_USB_NXT;
output					HPS_USB_STP;

//=======================================================
//  REG/WIRE declarations
//=======================================================

wire			[15: 0]	hex3_hex0;
//wire			[15: 0]	hex5_hex4;

//assign HEX0 = ~hex3_hex0[ 6: 0]; // hex3_hex0[ 6: 0]; 
//assign HEX1 = ~hex3_hex0[14: 8];
//assign HEX2 = ~hex3_hex0[22:16];
//assign HEX3 = ~hex3_hex0[30:24];
assign HEX4 = 7'b1111111;
assign HEX5 = 7'b1111111;

HexDigit Digit0(HEX0, hex3_hex0[3:0]);
HexDigit Digit1(HEX1, hex3_hex0[7:4]);
HexDigit Digit2(HEX2, hex3_hex0[11:8]);
HexDigit Digit3(HEX3, hex3_hex0[15:12]);

// add here:
//assign hex3_hex = max_time;
// M10k memory control and data
wire 		[7:0] 	M10k_out [15:0];
wire 		[7:0] 	write_data [15:0];
wire 		[18:0] 	write_address [15:0];
reg 		[18:0] 	read_address ;
wire 					write_enable [15:0];
wire	         write_enable_2;
//Second M10k block
wire 		[7:0] 	M10k_out_2 ;
wire 		[7:0] 	write_data_2;// stores color data
wire 		[18:0] 	write_address_2 ;
wire 		[18:0] 	read_address_2 ;

// M10k memory clock
wire 					M10k_pll /*synthesis keep */;
wire 					M10k_pll_locked ;

// pios for dealer and player
wire [7:0] dealer_top_pio;
wire [7:0] player_init_hand_pio;
//assign color_in_VGA = M10k_out[next_x[3:0]];

// TODO after init_done: dealer pio needs to be assigned to dealer hands right away, player needs to go to player hands
// TODO replace the init with values from SRAM memory

// Instantiate VGA driver		
/*			
vga_driver DUT   (	.clock(vga_pll), 
							.reset(pio_reset_external_connection_export[0] || ~KEY[0]),
							.color_in(color_in_VGA),	// Pixel color (8-bit) from memory
							.next_x(next_x),		// This (and next_y) used to specify memory read address
							.next_y(next_y),		// This (and next_x) used to specify memory read address
							.hsync(VGA_HS),
							.vsync(VGA_VS),
							.red(VGA_R),
							.green(VGA_G),
							.blue(VGA_B),
							.sync(VGA_SYNC_N),
							.clk(VGA_CLK),
							.blank(VGA_BLANK_N)
);*/

// SRAM variables
wire [31:0] sram_readdata;
reg [31:0] data_buffer, sram_writedata;
reg [8:0] sram_address; // (52*6)*4
reg sram_write;
wire sram_clken = 1'b1;
wire sram_chipselect = 1'b1;

reg [3:0] sram_state_seed = 4'd0;
wire [31:0] sram_readdata_seed;
reg [31:0] data_buffer_seed, sram_writedata_seed;
reg [10:0] sram_address_seed; // (52*6)*4
reg sram_write_seed;
wire sram_clken_seed = 1'b1;
wire sram_chipselect_seed = 1'b1;

reg [3:0] sram_state = 4'd0;
reg shared_mem_done;
reg [9:0] read_addr_init;
reg [9:0] write_addr_init;

reg [10:0] read_addr_init_seed;
reg [10:0] write_addr_init_seed;
reg seed_init;
reg data_ready;
reg data_ready_seed;

wire [7:0] init_done /*synthesis keep */;

always @(posedge CLOCK_50) begin
	if (init_done == 8'd0) begin
		read_addr_init_seed <= 11'd0;
		seed_init <= 1'd0;
		sram_address_seed <= 11'd0;
	end
	else if (init_done == 8'd1) begin
		if (sram_state_seed == 4'd0) begin
			if (read_addr_init_seed < 11'd2000) begin
				sram_address_seed <= read_addr_init_seed;
				sram_state_seed <= 4'd1;
				data_ready_seed <= 1'd0;
			end else begin
				seed_init <= 1'd1;
				data_ready_seed <= 1'd0;
			end
		end else if (sram_state_seed == 4'd1) begin // buffer state
			sram_state_seed <= 4'd2;
		end else if (sram_state_seed == 4'd2) begin
			data_buffer_seed <= sram_readdata_seed;
			read_addr_init_seed <= read_addr_init_seed + 11'd1;
			sram_state_seed <= 4'd0;
			data_ready_seed <= 1'd1;
		end
	end
end

always @(posedge CLOCK_50) begin
	if (init_done == 8'd0) begin
		read_addr_init <= 10'b0;
		shared_mem_done <= 1'd0;
	end
	else if (init_done == 8'd1) begin
		if (sram_state == 4'd0) begin
			if (read_addr_init < 10'd52) begin
				sram_address <= read_addr_init;
				sram_state <= 4'd1;
				data_ready <= 1'd0;
			end else begin
				shared_mem_done <= 1'd1;
				data_ready <= 1'd0;
			end
		end else if (sram_state == 4'd1) begin // buffer state
			sram_state <= 4'd2;
		end else if (sram_state == 4'd2) begin
			data_buffer <= sram_readdata;
			read_addr_init <= read_addr_init + 1;
			sram_state <= 4'd0;
			data_ready <= 1'd1;
		end
	end
end

parameter [9:0] num_simul = 3;//half of the number of columns
wire [7:0] output_random [num_simul-1:0] /*synthesis keep */; 
//wire [63:0] seed_samples [5:0];

reg [2:0] read_write_offset;
reg [7:0] write_addr_inter;

wire unsigned [3:0] q [num_simul-1:0];
reg unsigned  [3:0] d [num_simul-1:0];
reg [9:0] write_addr [num_simul-1:0];
reg [9:0] read_addr [num_simul-1:0];
reg we[num_simul-1:0];

reg [3:0] drum_state [num_simul-1:0];
reg [9:0] index_rows [num_simul-1:0];

reg tie_check [num_simul-1:0];

//Shared m10k

reg write_init_done [num_simul-1:0] /*synthesis keep */;
reg [4:0] card_itr [num_simul-1:0];
reg [1:0] player_result[num_simul-1:0];

wire [31:0] num_ties_pio;
wire [31:0] num_wins_pio;
reg [31:0] num_ties;
reg [31:0] num_wins;

assign num_ties_pio = output_random[0];
assign num_wins_pio = drum_state[0];

reg [10:0] result_counter;

reg checked_card [num_simul - 1:0] /*synthesis keep */;
always @(posedge CLOCK_50) begin
	if (init_done == 8'd0) begin
		num_wins <= 32'd0;
		num_ties <= 32'd0;
		result_counter <= 11'd0;
	end
	else begin
		if (result_counter == num_simul) begin
			//result_counter <= 11'd0;
		end
		else begin 
			if (prob_result[result_counter] == 1'd1 /*&& checked_card[result_counter] == 1'd0*/) begin
				if (player_result[result_counter] == 2'd1) begin
					num_wins <= num_wins + 32'd1;
				end
				else if (player_result[result_counter] == 2'd2) begin
					num_ties <= num_ties + 32'd1;
				end
				/*checked_card[result_counter] <= 1'd1;*/
				result_counter <= result_counter + 11'd1;
			end else begin
				result_counter <= result_counter;
			end
			
		end
	end
end

reg [4:0] player_hands [num_simul - 1:0];
reg [4:0] dealer_hands [num_simul - 1:0];


reg prob_result [num_simul - 1: 0];
wire [4:0] result_record_test;
//assign result_record_test = {prob_result[0], prob_result[1], prob_result[2], prob_result[3], prob_result[4]};

// test pios
wire [31:0] test1_pio; //
wire [31:0] test_1 [num_simul-1:0];
assign test1_pio = test_1[0];

wire [31:0] test2_pio; //
wire [31:0] test_2 [num_simul-1:0]; 
assign test2_pio = test_2[0];

wire [31:0] test3_pio; //
wire [31:0] test_3 [num_simul-1:0];
assign test3_pio = player_init_hand_pio;

wire [31:0] test4_pio; //
wire [31:0] test_4 [num_simul-1:0];
assign test4_pio = dealer_top_pio;

genvar i;
generate 
	for (i = 0; i < num_simul; i = i+1) begin: initCols  
		//rand127 random_num(.rand_out(output_random[i]), .seed_in(64'h54555555 ^ i), .clock_in(CLOCK_50), .reset_in(((init_done == 8'd0) ? 1'd1 : 1'd0)));
		reg [3:0] array_hit_card [11:0]; // track index of card
		reg [255:0] hit_card_reg;
		reg [3:0] drawn_card_val [11:0]; // track the value of card being drawn
		reg [3:0] dealer_card;
		wire [5:0] output_random_vals[5:0];
		wire picked; 

		// reg internal_state;
		
		//assign hit_card_reg = ;
		
		reg [5:0] seed_in [2:0];
		//assign seed_in[0] = (data_ready_seed == 1'd1 && read_addr_init_seed == i*3) ? data_buffer_seed : 6'd0;
		//assign seed_in[1] = (data_ready_seed == 1'd1 && read_addr_init_seed == (i*3) + 1) ? data_buffer_seed : 6'd0;
		//assign seed_in[2] = (data_ready_seed == 1'd1 && read_addr_init_seed == (i*3) + 2) ? data_buffer_seed : 6'd0;
		//assign seed_test[i] = seed_in[0];
		reg internal_reset_1;
		reg internal_reset_2;
		reg internal_reset_3;
		
		//TODO: on chip SRAM does not work because seed_in does not pick up value
		
		always @(posedge CLOCK_50) begin
			if (init_done == 8'd0) begin
				seed_in[0] <= 6'd0;
				seed_in[1] <= 6'd0;
				seed_in[2] <= 6'd0;
				internal_reset_1 <= 1'd0;
				internal_reset_2 <= 1'd0;
				internal_reset_3 <= 1'd0;

			end
			if (~seed_init && init_done == 8'd1) begin
				if ((data_ready_seed == 1'd1 && sram_address_seed == i*3)) begin
					seed_in[0] <= data_buffer_seed;
					internal_reset_1 <= 1'd1;
				end
				if (internal_reset_1 == 1'd1) begin
					internal_reset_1 <= 1'd0;
				end
				
				if ((data_ready_seed == 1'd1 && sram_address_seed == (i*3 + 1))) begin
					seed_in[1] <= data_buffer_seed;
					internal_reset_2 <= 1'd1;
				end
				if (internal_reset_2 == 1'd1) begin
					internal_reset_2 <= 1'd0;
				end
				
				if ((data_ready_seed == 1'd1 && sram_address_seed == (i*3 + 2))) begin
					seed_in[2] <= data_buffer_seed;
					internal_reset_3 <= 1'd1;
				end
				if (internal_reset_3 == 1'd1) begin
					internal_reset_3 <= 1'd0;
				end
			end
		end
		
		assign output_random[i] = {output_random_vals[5][0],output_random_vals[4][0],output_random_vals[3][0],output_random_vals[2][0],output_random_vals[1][0],output_random_vals[0][0]} ;
		//rand127 random_num(.rand_out(output_random), .seed_in(64'h54555555), .clock_in(clk_50), .reset_in(reset));
		
		assign test_2[i] = seed_init;
		assign test_1[i] = output_random_vals[0];
		
		rand6 random_num(.rand_out(output_random_vals[0]), .seed_in(seed_in[0]), .clock_in(CLOCK_50), .reset_in(internal_reset_1));
		rand6 random_num_2(.rand_out(output_random_vals[1]), .seed_in(seed_in[1]), .clock_in(CLOCK_50), .reset_in(internal_reset_2));
		rand6 random_num_3(.rand_out(output_random_vals[2]), .seed_in(seed_in[2]), .clock_in(CLOCK_50), .reset_in(internal_reset_3));
		rand6 random_num_4(.rand_out(output_random_vals[3]), .seed_in(seed_in[0] ^ seed_in[1]), .clock_in(CLOCK_50), .reset_in(internal_reset_2));
		rand6 random_num_5(.rand_out(output_random_vals[4]), .seed_in(seed_in[1] ^ seed_in[2]), .clock_in(CLOCK_50), .reset_in(internal_reset_3));
		rand6 random_num_6(.rand_out(output_random_vals[5]), .seed_in(seed_in[2] ^ seed_in[0]), .clock_in(CLOCK_50), .reset_in(internal_reset_3));
	
		M10K_32_5 m10k_curr(.q(q[i]), .d(d[i]), .write_address(write_addr[i]), .read_address(read_addr[i]), .we(we[i]), .clk(CLOCK_50));
		Search_hit_card card_check( .picked_card(hit_card_reg), 
									.card(output_random[i]), // TODO we don't want generated number all the time
									.picked(picked));
									
		always @ (posedge CLOCK_50) begin
			if (init_done == 8'd0) begin // reset conditions
				drum_state[i] <= 4'd0;
				index_rows[i] <= 10'd0;
				we[i] <= 1'd1; 
				write_addr[i] <= 10'b0;
				write_init_done[i] <= 1'b0;
				read_addr[i] <= 10'd0;
				
				hit_card_reg <= 256'd0;
				
				player_hands[i] <= player_init_hand_pio;
				dealer_hands[i] <= dealer_top_pio;
			
				card_itr[i] <= 5'd0;
				
				prob_result[i] <= 1'd0;
			end
			else begin
				// STATE 0: Initialization
				if (drum_state[i] == 4'd0) begin // check if initialization has occurred
					if (data_ready) begin
						d[i] <= data_buffer[3:0];
						write_addr[i] <= write_addr[i] + 10'b1;
					end
					if (shared_mem_done && seed_init) begin
						drum_state[i] <= 4'd1;
						write_init_done[i] <= 1'b1;

					end
				end
				// STATE 1: Dealer's card
				else if (drum_state[i] == 4'd1) begin //initiate dealers cards
					drum_state[i] <= 4'd2;
					read_addr[i] <= output_random[i];
					array_hit_card[card_itr[i]] <= output_random[i];	
					//card_itr <= card_itr + 4'd1;						
				end
				// STATE 2: Buffer state
				else if (drum_state[i] == 4'd2) begin
					drum_state[i] <= 4'd3;
				end
				// STATE 3: Get dealer hidden card
				else if (drum_state[i] == 4'd3) begin
					drawn_card_val[card_itr[i]] <= q[i];
					dealer_hands[i] <= dealer_hands[i] + q[i];
					hit_card_reg <= hit_card_reg | (1 << array_hit_card[card_itr[i]]);
					card_itr[i] <= card_itr[i] + 5'd1;
					drum_state[i] <= 4'd4;
				end
				// STATE 4: Check dealer blackjack
				else if (drum_state[i] == 4'd4) begin // check if dealer gets BJ
					if (dealer_hands[i] == 5'd21) begin		
						drum_state[i] <= 4'd10; // if dealer gets BJ, no need to move on, check result right away
					end
					else begin
						drum_state[i] <= 4'd5;
					end
				end
				// STATE 5: Player's turn
				else if (drum_state[i] == 4'd5) begin
					if (~picked) begin
						read_addr[i] <= output_random[i];
						array_hit_card[card_itr[i]] <= output_random[i];
						drum_state[i] <= 4'd6;
					end else begin
						drum_state[i] <= 4'd5;
					end
				end 
				// STATE 6: Buffer state
				else if (drum_state[i] == 4'd6) begin
					drum_state[i] <= 4'd7;
				end
				// STATE 7: Player playing til stand
				else if (drum_state[i] == 4'd7) begin
					if ((player_hands[i] == 5'd12 && drawn_card_val[0] >= 5'd4 && drawn_card_val[0] <= 5'd6) || (player_hands[i] >= 5'd13 && player_hands[i] < 5'd17 && drawn_card_val[0] >= 5'd2 && drawn_card_val[0] <= 5'd6) || (player_hands[i] >= 5'd17)) begin
						drum_state[i] <= 4'd8; 
					end
					else begin
						drum_state[i] <= 4'd5;	//HIT	
						hit_card_reg <= hit_card_reg | (1 << array_hit_card[card_itr[i]]);
						
						drawn_card_val[card_itr[i]] <= q[i];
						card_itr[i] <= card_itr[i] + 5'd1;
						player_hands[i] <= player_hands[i] + q[i];
					end
				end
				// STATE 8: Dealer's turn
				else if (drum_state[i] == 4'd8) begin
					if (~picked) begin
						read_addr[i] <= output_random[i];
						array_hit_card[card_itr[i]] <= output_random[i];
						drum_state[i] <= 4'd9;
					end else begin
						drum_state[i] <= 4'd8;
					end	
				end
				// STATE 9: Buffer state
				else if (drum_state[i] == 4'd9) begin
					drum_state[i] <= 4'd10;
				end
				// STATE 10: Dealer playing until >=17
				else if (drum_state[i] == 4'd10) begin
					if (dealer_hands[i] >= 5'd17) begin
						drum_state[i] <= 4'd11; // Result
					end
					else begin
						drawn_card_val[card_itr[i]] <= q[i];
						card_itr[i] <= card_itr[i] + 5'd1;
						dealer_hands[i] <= dealer_hands[i] + q[i];
						drum_state[i] <= 4'd8 ;	//HIT	
						hit_card_reg <= hit_card_reg | (1 << array_hit_card[card_itr[i]]);
					end
				end
				// STATE 11: Check final result 
				else if (drum_state[i] == 4'd11) begin
					if (player_hands[i] > 5'd21) begin
						player_result[i] <= 2'd0;
						drum_state[i] <= 4'd12;
					end
					else if (player_hands[i] < 5'd21 && dealer_hands[i] <= 5'd21 && player_hands[i] < dealer_hands[i]) begin
						player_result[i] <= 2'd0;
						drum_state[i] <= 4'd12;
					end
					else if (player_hands[i] == dealer_hands[i]) begin
						player_result[i] <= 2'd2;
						tie_check[i] <= 1'd1;
						drum_state[i] <= 4'd12;
					end
					else if (((player_hands[i] > dealer_hands[i]) && player_hands[i] <= 5'd21 && dealer_hands[i] < 5'd21) || (dealer_hands[i] > 5'd21)) begin
						player_result[i] <= 2'd1;
						drum_state[i] <= 4'd12;
					end
				end

				// STATE 12: record the result for Prob
				else if (drum_state[i] == 4'd12) begin
					prob_result[i] <= 1'd1; 
				end
			end
		end 
		
	end
endgenerate

//=======================================================
//  Structural coding
//=======================================================

Computer_System The_System (
	////////////////////////////////////
	// FPGA Side
	////////////////////////////////////
	// cards
	.onchip_sram_cards_s1_address(sram_address),                
	.onchip_sram_cards_s1_clken(sram_clken),
	.onchip_sram_cards_s1_chipselect(sram_chipselect),
	.onchip_sram_cards_s1_write(sram_write),                
	.onchip_sram_cards_s1_readdata(sram_readdata),               
	.onchip_sram_cards_s1_writedata(sram_writedata),              
	.onchip_sram_cards_s1_byteenable(4'b1111),   
	
	// seed
	.onchip_sram_seed_s1_address(sram_address_seed),                 
	.onchip_sram_seed_s1_clken(sram_clken_seed),                   
	.onchip_sram_seed_s1_chipselect(sram_chipselect_seed),              
	.onchip_sram_seed_s1_write(sram_write_seed),            
	.onchip_sram_seed_s1_readdata(sram_readdata_seed),
   .onchip_sram_seed_s1_writedata(sram_writedata_seed),	
	.onchip_sram_seed_s1_byteenable(4'b1111),

	// m10k
	.m10k_pll_locked_export			(M10k_pll_locked),
	.m10k_pll_outclk0_clk			(M10k_pll),
	
	// Global signals
	.system_pll_ref_clk_clk					(CLOCK_50),
	.system_pll_ref_reset_reset			(1'b0),

	// AV Config
	.av_config_SCLK							(FPGA_I2C_SCLK),
	.av_config_SDAT							(FPGA_I2C_SDAT),

	// Audio Subsystem
	.audio_pll_ref_clk_clk					(CLOCK3_50),
	.audio_pll_ref_reset_reset				(1'b0),
	.audio_clk_clk								(AUD_XCK),
	.audio_ADCDAT								(AUD_ADCDAT),
	.audio_ADCLRCK								(AUD_ADCLRCK),
	.audio_BCLK									(AUD_BCLK),
	.audio_DACDAT								(AUD_DACDAT),
	.audio_DACLRCK								(AUD_DACLRCK),

	// Slider Switches
	.slider_switches_export					(SW),

	// Pushbuttons (~KEY[3:0]),
	.pushbuttons_export						(~KEY[3:0]),

	// Expansion JP1
	//.expansion_jp1_export					({GPIO_0[35:19], GPIO_0[17], GPIO_0[15:3], GPIO_0[1]}),

	// Expansion JP2
	//.expansion_jp2_export					({GPIO_1[35:19], GPIO_1[17], GPIO_1[15:3], GPIO_1[1]}),

	// LEDs
	.leds_export								(LEDR),
	
	// Seven Segs
	.hex3_hex0_export							(hex3_hex0),
	//.hex5_hex4_export							(hex5_hex4),
	
	// PS2 Ports
	//.ps2_port_CLK								(PS2_CLK),
	//.ps2_port_DAT								(PS2_DAT),
	//.ps2_port_dual_CLK						(PS2_CLK2),
	//.ps2_port_dual_DAT						(PS2_DAT2),

	// IrDA
	//.irda_RXD									(IRDA_RXD),
	//.irda_TXD									(IRDA_TXD),

	// VGA Subsystem
	.vga_pll_ref_clk_clk 					(CLOCK2_50),
	.vga_pll_ref_reset_reset				(1'b0),
	.vga_CLK										(VGA_CLK),
	.vga_BLANK									(VGA_BLANK_N),
	.vga_SYNC									(VGA_SYNC_N),
	.vga_HS										(VGA_HS),
	.vga_VS										(VGA_VS),
	.vga_R										(VGA_R),
	.vga_G										(VGA_G),
	.vga_B										(VGA_B),
	
	// Video In Subsystem
//	.video_in_TD_CLK27 						(TD_CLK27),
//	.video_in_TD_DATA							(TD_DATA),
//	.video_in_TD_HS							(TD_HS),
//	.video_in_TD_VS							(TD_VS),
//	.video_in_clk27_reset					(),
//	.video_in_TD_RESET						(TD_RESET_N),
//	.video_in_overflow_flag					(),
	
	// SDRAM
	.sdram_clk_clk								(DRAM_CLK),
   .sdram_addr									(DRAM_ADDR),
	.sdram_ba									(DRAM_BA),
	.sdram_cas_n								(DRAM_CAS_N),
	.sdram_cke									(DRAM_CKE),
	.sdram_cs_n									(DRAM_CS_N),
	.sdram_dq									(DRAM_DQ),
	.sdram_dqm									({DRAM_UDQM,DRAM_LDQM}),
	.sdram_ras_n								(DRAM_RAS_N),
	.sdram_we_n									(DRAM_WE_N),
	
	////////////////////////////////////
	// HPS Side
	////////////////////////////////////
	// DDR3 SDRAM
	.memory_mem_a			(HPS_DDR3_ADDR),
	.memory_mem_ba			(HPS_DDR3_BA),
	.memory_mem_ck			(HPS_DDR3_CK_P),
	.memory_mem_ck_n		(HPS_DDR3_CK_N),
	.memory_mem_cke		(HPS_DDR3_CKE),
	.memory_mem_cs_n		(HPS_DDR3_CS_N),
	.memory_mem_ras_n		(HPS_DDR3_RAS_N),
	.memory_mem_cas_n		(HPS_DDR3_CAS_N),
	.memory_mem_we_n		(HPS_DDR3_WE_N),
	.memory_mem_reset_n	(HPS_DDR3_RESET_N),
	.memory_mem_dq			(HPS_DDR3_DQ),
	.memory_mem_dqs		(HPS_DDR3_DQS_P),
	.memory_mem_dqs_n		(HPS_DDR3_DQS_N),
	.memory_mem_odt		(HPS_DDR3_ODT),
	.memory_mem_dm			(HPS_DDR3_DM),
	.memory_oct_rzqin		(HPS_DDR3_RZQ),
		  
	// Ethernet
	.hps_io_hps_io_gpio_inst_GPIO35	(HPS_ENET_INT_N),
	.hps_io_hps_io_emac1_inst_TX_CLK	(HPS_ENET_GTX_CLK),
	.hps_io_hps_io_emac1_inst_TXD0	(HPS_ENET_TX_DATA[0]),
	.hps_io_hps_io_emac1_inst_TXD1	(HPS_ENET_TX_DATA[1]),
	.hps_io_hps_io_emac1_inst_TXD2	(HPS_ENET_TX_DATA[2]),
	.hps_io_hps_io_emac1_inst_TXD3	(HPS_ENET_TX_DATA[3]),
	.hps_io_hps_io_emac1_inst_RXD0	(HPS_ENET_RX_DATA[0]),
	.hps_io_hps_io_emac1_inst_MDIO	(HPS_ENET_MDIO),
	.hps_io_hps_io_emac1_inst_MDC		(HPS_ENET_MDC),
	.hps_io_hps_io_emac1_inst_RX_CTL	(HPS_ENET_RX_DV),
	.hps_io_hps_io_emac1_inst_TX_CTL	(HPS_ENET_TX_EN),
	.hps_io_hps_io_emac1_inst_RX_CLK	(HPS_ENET_RX_CLK),
	.hps_io_hps_io_emac1_inst_RXD1	(HPS_ENET_RX_DATA[1]),
	.hps_io_hps_io_emac1_inst_RXD2	(HPS_ENET_RX_DATA[2]),
	.hps_io_hps_io_emac1_inst_RXD3	(HPS_ENET_RX_DATA[3]),

	// Flash
	.hps_io_hps_io_qspi_inst_IO0	(HPS_FLASH_DATA[0]),
	.hps_io_hps_io_qspi_inst_IO1	(HPS_FLASH_DATA[1]),
	.hps_io_hps_io_qspi_inst_IO2	(HPS_FLASH_DATA[2]),
	.hps_io_hps_io_qspi_inst_IO3	(HPS_FLASH_DATA[3]),
	.hps_io_hps_io_qspi_inst_SS0	(HPS_FLASH_NCSO),
	.hps_io_hps_io_qspi_inst_CLK	(HPS_FLASH_DCLK),

	// Accelerometer
	.hps_io_hps_io_gpio_inst_GPIO61	(HPS_GSENSOR_INT),

	//.adc_sclk                        (ADC_SCLK),
	//.adc_cs_n                        (ADC_CS_N),
	//.adc_dout                        (ADC_DOUT),
	//.adc_din                         (ADC_DIN),

	// General Purpose I/O
	.hps_io_hps_io_gpio_inst_GPIO40	(HPS_GPIO[0]),
	.hps_io_hps_io_gpio_inst_GPIO41	(HPS_GPIO[1]),

	// I2C
	.hps_io_hps_io_gpio_inst_GPIO48	(HPS_I2C_CONTROL),
	.hps_io_hps_io_i2c0_inst_SDA		(HPS_I2C1_SDAT),
	.hps_io_hps_io_i2c0_inst_SCL		(HPS_I2C1_SCLK),
	.hps_io_hps_io_i2c1_inst_SDA		(HPS_I2C2_SDAT),
	.hps_io_hps_io_i2c1_inst_SCL		(HPS_I2C2_SCLK),

	// Pushbutton
	.hps_io_hps_io_gpio_inst_GPIO54	(HPS_KEY),

	// LED
	.hps_io_hps_io_gpio_inst_GPIO53	(HPS_LED),

	// SD Card
	.hps_io_hps_io_sdio_inst_CMD	(HPS_SD_CMD),
	.hps_io_hps_io_sdio_inst_D0	(HPS_SD_DATA[0]),
	.hps_io_hps_io_sdio_inst_D1	(HPS_SD_DATA[1]),
	.hps_io_hps_io_sdio_inst_CLK	(HPS_SD_CLK),
	.hps_io_hps_io_sdio_inst_D2	(HPS_SD_DATA[2]),
	.hps_io_hps_io_sdio_inst_D3	(HPS_SD_DATA[3]),

	// SPI
	.hps_io_hps_io_spim1_inst_CLK		(HPS_SPIM_CLK),
	.hps_io_hps_io_spim1_inst_MOSI	(HPS_SPIM_MOSI),
	.hps_io_hps_io_spim1_inst_MISO	(HPS_SPIM_MISO),
	.hps_io_hps_io_spim1_inst_SS0		(HPS_SPIM_SS),

	// UART
	.hps_io_hps_io_uart0_inst_RX	(HPS_UART_RX),
	.hps_io_hps_io_uart0_inst_TX	(HPS_UART_TX),

	// USB
	.hps_io_hps_io_gpio_inst_GPIO09	(HPS_CONV_USB_N),
	.hps_io_hps_io_usb1_inst_D0		(HPS_USB_DATA[0]),
	.hps_io_hps_io_usb1_inst_D1		(HPS_USB_DATA[1]),
	.hps_io_hps_io_usb1_inst_D2		(HPS_USB_DATA[2]),
	.hps_io_hps_io_usb1_inst_D3		(HPS_USB_DATA[3]),
	.hps_io_hps_io_usb1_inst_D4		(HPS_USB_DATA[4]),
	.hps_io_hps_io_usb1_inst_D5		(HPS_USB_DATA[5]),
	.hps_io_hps_io_usb1_inst_D6		(HPS_USB_DATA[6]),
	.hps_io_hps_io_usb1_inst_D7		(HPS_USB_DATA[7]),
	.hps_io_hps_io_usb1_inst_CLK		(HPS_USB_CLKOUT),
	.hps_io_hps_io_usb1_inst_STP		(HPS_USB_STP),
	.hps_io_hps_io_usb1_inst_DIR		(HPS_USB_DIR),
	.hps_io_hps_io_usb1_inst_NXT		(HPS_USB_NXT),
	
	// added pios
	.dealer_top_external_connection_export(dealer_top_pio),
	.init_done_external_connection_export(init_done),
	.player_init_hand_external_connection_export(player_init_hand_pio),
	.num_ties_pio_external_connection_export(num_ties_pio),
	.num_wins_pio_external_connection_export(num_wins_pio),
/*	
	.test1_external_connection_export(test1_pio),
	.test2_external_connection_export(test2_pio),
	.test3_external_connection_export(test3_pio),
	.test4_external_connection_export(test4_pio)*/
);

endmodule

//////////////////////////////////////////////////////////////
////////////	Search for hit card		  	    //////////////
//////////////////////////////////////////////////////////////

module Search_hit_card(picked_card, card, picked);
	input [255:0] picked_card;
	input [7:0]card;
	output picked;
					
	assign picked = card > 52 || ((picked_card) & (1 << card)) >> card;
endmodule

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
    reg [3:0] mem [511:0]  /* synthesis ramstyle = "no_rw_check, M10K" */; // TODO 256 bits
	 
    always @ (posedge clk) begin
        if (we) begin
            mem[write_address] <= d;
		  end
		  
        q <= mem[read_address]; // q doesn't get d in this clock cycle
    end
endmodule
