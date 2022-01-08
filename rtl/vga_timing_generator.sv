`timescale 1ns/100ps

// Horizontal Timing Control
module horizontal_generator
(
	input clk,
	input reset_n,
	output hsync,
	output hblank,
	output wire [7:0] x_counter
);
	localparam LINE_DONE 	= 'd264;
	localparam FP_START 		= 'd200;
	localparam SYNC_START 	= 'd210;
	localparam BP_START 		= 'd242;

	// TODO: change to $clog2(LINE_DONE)+1
	reg [8:0] pixel_count; /* synthesis syn_keep */
	always @(posedge clk or negedge reset_n) begin
		if(!reset_n) pixel_count <= 0;
		else
		pixel_count <= start_line_done ? 0 : pixel_count + 1;
	end

	assign x_counter 				= ( pixel_count >  FP_START-1) ? FP_START-1 : pixel_count[7:0];
	assign start_hsync		   = ( pixel_count == SYNC_START);
	assign start_front_porch	= ( pixel_count == FP_START );
	assign start_back_porch 	= ( pixel_count == BP_START );
	assign start_line_done		= ( pixel_count == LINE_DONE );

	reg hsync_p, hblank_p;
	reg [3:0] hsync_ff;
	reg [3:0] hblank_ff;	
	
// Flop the output signals to accomodate the SRAM.
// This should really be a generate block.
	parameter SYNC_FLOP = 'd2;
	assign hsync 	= ~hsync_ff[SYNC_FLOP];
	assign hblank 	= hblank_ff[SYNC_FLOP]; 
	always @(posedge clk or negedge reset_n)
		if(!reset_n) begin
			hsync_ff  <= 0;
			hblank_ff <= 0;
		end else begin
			hsync_ff[0] <= hsync_p; // latch the 1st val.
			hsync_ff[1] <= hsync_ff[0];
			hsync_ff[2] <= hsync_ff[1];
			hsync_ff[3] <= hsync_ff[2];
			//
			hblank_ff[0] <= hblank_p;
			hblank_ff[1] <= hblank_ff[0];
			hblank_ff[2] <= hblank_ff[1];
			hblank_ff[3] <= hblank_ff[2];
		end
// END SYNC FLOP
	always @* begin
		if(start_hsync) hsync_p = 1;
		else if(start_back_porch) hsync_p = 0;
		if(start_front_porch) hblank_p = 1;
		else if(start_line_done)  hblank_p = 0;
	end
	

endmodule

// Here we're generating our vertical sync, really this could use the previous
// module, and just take the horizontal hsync as the clock to drive the
// counter.  The reset of the combi logic would work fine.

module vertical_generator(
	input clk,
	input reset_n,
	output vsync,
	output vblank,
	output wire [9:0] y_counter
);

	reg [9:0] line_count; /* synthesis syn_keep */
	always @(posedge clk or negedge reset_n) begin // clk = hblank
		if(!reset_n) line_count <= 0;
		else
		line_count <= (start_frame_done) ? 0 : line_count + 1;
	end

	localparam FP_START 		= 'd600;
	localparam SYNC_START 	= 'd601;
	localparam BP_START 		= 'd605;
	localparam FRAME_DONE	= 'd628;

	assign y_counter				= ( line_count >  FP_START-1) ? 0 : line_count;
	assign start_vsync		   = ( line_count == SYNC_START);
	assign start_front_porch	= ( line_count == FP_START );
	assign start_back_porch 	= ( line_count == BP_START );
	assign start_frame_done		= ( line_count == FRAME_DONE );

	
	reg vsync_p, vblank_p; // Sometimes, VSYNC is positive
	assign vsync = ~vsync_p;
	assign vblank = vblank_p;

	always @* begin
		if(start_vsync) 	vsync_p = 1;
		else if(start_back_porch) 	vsync_p = 0;
		if(start_front_porch)	vblank_p = 1;
		else if(start_frame_done)	vblank_p = 0;
	end
	
endmodule

