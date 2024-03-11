// Computer_System_Audio.v

// Generated using ACDS version 18.1 625

`timescale 1 ps / 1 ps
module Computer_System_Audio (
		input  wire        audio_ADCDAT,              //               audio.ADCDAT
		input  wire        audio_ADCLRCK,             //                    .ADCLRCK
		input  wire        audio_BCLK,                //                    .BCLK
		output wire        audio_DACDAT,              //                    .DACDAT
		input  wire        audio_DACLRCK,             //                    .DACLRCK
		output wire        audio_clk_clk,             //           audio_clk.clk
		output wire        audio_irq_irq,             //           audio_irq.irq
		input  wire        audio_pll_ref_clk_clk,     //   audio_pll_ref_clk.clk
		input  wire        audio_pll_ref_reset_reset, // audio_pll_ref_reset.reset
		output wire        audio_reset_reset,         //         audio_reset.reset
		input  wire [1:0]  audio_slave_address,       //         audio_slave.address
		input  wire        audio_slave_chipselect,    //                    .chipselect
		input  wire        audio_slave_read,          //                    .read
		input  wire        audio_slave_write,         //                    .write
		input  wire [31:0] audio_slave_writedata,     //                    .writedata
		output wire [31:0] audio_slave_readdata,      //                    .readdata
		input  wire        sys_clk_clk,               //             sys_clk.clk
		input  wire        sys_reset_reset_n          //           sys_reset.reset_n
	);

	wire    rst_controller_reset_out_reset; // rst_controller:reset_out -> Audio:reset

	Computer_System_Audio_Audio audio (
		.clk         (sys_clk_clk),                    //                clk.clk
		.reset       (rst_controller_reset_out_reset), //              reset.reset
		.address     (audio_slave_address),            // avalon_audio_slave.address
		.chipselect  (audio_slave_chipselect),         //                   .chipselect
		.read        (audio_slave_read),               //                   .read
		.write       (audio_slave_write),              //                   .write
		.writedata   (audio_slave_writedata),          //                   .writedata
		.readdata    (audio_slave_readdata),           //                   .readdata
		.irq         (audio_irq_irq),                  //          interrupt.irq
		.AUD_ADCDAT  (audio_ADCDAT),                   // external_interface.export
		.AUD_ADCLRCK (audio_ADCLRCK),                  //                   .export
		.AUD_BCLK    (audio_BCLK),                     //                   .export
		.AUD_DACDAT  (audio_DACDAT),                   //                   .export
		.AUD_DACLRCK (audio_DACLRCK)                   //                   .export
	);

	Computer_System_Audio_Audio_PLL audio_pll (
		.ref_clk_clk        (audio_pll_ref_clk_clk),     //      ref_clk.clk
		.ref_reset_reset    (audio_pll_ref_reset_reset), //    ref_reset.reset
		.audio_clk_clk      (audio_clk_clk),             //    audio_clk.clk
		.reset_source_reset (audio_reset_reset)          // reset_source.reset
	);

	altera_reset_controller #(
		.NUM_RESET_INPUTS          (1),
		.OUTPUT_RESET_SYNC_EDGES   ("deassert"),
		.SYNC_DEPTH                (2),
		.RESET_REQUEST_PRESENT     (0),
		.RESET_REQ_WAIT_TIME       (1),
		.MIN_RST_ASSERTION_TIME    (3),
		.RESET_REQ_EARLY_DSRT_TIME (1),
		.USE_RESET_REQUEST_IN0     (0),
		.USE_RESET_REQUEST_IN1     (0),
		.USE_RESET_REQUEST_IN2     (0),
		.USE_RESET_REQUEST_IN3     (0),
		.USE_RESET_REQUEST_IN4     (0),
		.USE_RESET_REQUEST_IN5     (0),
		.USE_RESET_REQUEST_IN6     (0),
		.USE_RESET_REQUEST_IN7     (0),
		.USE_RESET_REQUEST_IN8     (0),
		.USE_RESET_REQUEST_IN9     (0),
		.USE_RESET_REQUEST_IN10    (0),
		.USE_RESET_REQUEST_IN11    (0),
		.USE_RESET_REQUEST_IN12    (0),
		.USE_RESET_REQUEST_IN13    (0),
		.USE_RESET_REQUEST_IN14    (0),
		.USE_RESET_REQUEST_IN15    (0),
		.ADAPT_RESET_REQUEST       (0)
	) rst_controller (
		.reset_in0      (~sys_reset_reset_n),             // reset_in0.reset
		.clk            (sys_clk_clk),                    //       clk.clk
		.reset_out      (rst_controller_reset_out_reset), // reset_out.reset
		.reset_req      (),                               // (terminated)
		.reset_req_in0  (1'b0),                           // (terminated)
		.reset_in1      (1'b0),                           // (terminated)
		.reset_req_in1  (1'b0),                           // (terminated)
		.reset_in2      (1'b0),                           // (terminated)
		.reset_req_in2  (1'b0),                           // (terminated)
		.reset_in3      (1'b0),                           // (terminated)
		.reset_req_in3  (1'b0),                           // (terminated)
		.reset_in4      (1'b0),                           // (terminated)
		.reset_req_in4  (1'b0),                           // (terminated)
		.reset_in5      (1'b0),                           // (terminated)
		.reset_req_in5  (1'b0),                           // (terminated)
		.reset_in6      (1'b0),                           // (terminated)
		.reset_req_in6  (1'b0),                           // (terminated)
		.reset_in7      (1'b0),                           // (terminated)
		.reset_req_in7  (1'b0),                           // (terminated)
		.reset_in8      (1'b0),                           // (terminated)
		.reset_req_in8  (1'b0),                           // (terminated)
		.reset_in9      (1'b0),                           // (terminated)
		.reset_req_in9  (1'b0),                           // (terminated)
		.reset_in10     (1'b0),                           // (terminated)
		.reset_req_in10 (1'b0),                           // (terminated)
		.reset_in11     (1'b0),                           // (terminated)
		.reset_req_in11 (1'b0),                           // (terminated)
		.reset_in12     (1'b0),                           // (terminated)
		.reset_req_in12 (1'b0),                           // (terminated)
		.reset_in13     (1'b0),                           // (terminated)
		.reset_req_in13 (1'b0),                           // (terminated)
		.reset_in14     (1'b0),                           // (terminated)
		.reset_req_in14 (1'b0),                           // (terminated)
		.reset_in15     (1'b0),                           // (terminated)
		.reset_req_in15 (1'b0)                            // (terminated)
	);

endmodule
