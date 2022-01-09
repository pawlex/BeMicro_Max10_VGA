`timescale 1ns/100ps

module tb();
    reg reset_n=0;
    reg clk=0;

    initial begin
        forever #0.5 clk = ~clk;
    end

    initial begin
	    reset_n = 0;
	    #10;
	    reset_n = 1;
	    #600000 $finish;
    end

    initial begin
        $dumpfile ("dump.vcd"); 
        $dumpvars(); 
    end 


	wire hsync, vsync, vblank, hblank;
	horizontal_generator h_dut (
	    .clk  (clk),
	    .reset_n(reset_n),
	    .hsync(hsync),
	    .hblank(hblank)
	);
	vertical_generator v_dut (
		.clk(hsync),
		.reset_n(reset_n),
		.vsync(vsync),
		.vblank(vblank)
	);

endmodule

