`timescale 1ns/1ps

module tb_ili9488_top;

    // Inputs
    logic clk_50mhz;
    logic btn_rst;
    logic ili_miso;

    // Outputs
    logic ili_led;
    logic ili_sck;
    logic ili_mosi;
    logic ili_dc;
    logic ili_rst;
    logic ili_cs;
    logic led_pin;

    // Instantiate the Unit Under Test (UUT)
    // Overriding MS_COUNT to 5 speeds up the simulation massively!
    // Instead of waiting 25,000,000 clock cycles for a 500ms boot wait,
    // it will only wait 2,500 clock cycles.
    ili9488_top #(
        .MS_COUNT(5)
    ) uut (
        .clk_50mhz(clk_50mhz),
        .btn_rst(btn_rst),
        .ili_miso(ili_miso),
        .ili_led(ili_led),
        .ili_sck(ili_sck),
        .ili_mosi(ili_mosi),
        .ili_dc(ili_dc),
        .ili_rst(ili_rst),
        .ili_cs(ili_cs),
        .led_pin(led_pin)
    );

    // 50 MHz clock generation (20 ns period)
    initial begin
        clk_50mhz = 0;
        forever #10 clk_50mhz = ~clk_50mhz;
    end

    // Test sequence
    initial begin
        // Initialize inputs
        btn_rst = 1;
        ili_miso = 0;

        $display("[%0t] Simulation Started", $time);

        // Apply reset button press
        #100;
        btn_rst = 0;
        #200;
        btn_rst = 1;
        $display("[%0t] Reset Button Released", $time);

        // Wait for FSM to reach ST_DONE (led_pin turns high)
        wait(led_pin == 1'b1);
        
        $display("[%0t] Simulation Finished Successfully! ST_DONE Reached.", $time);
        #1000;
        $finish;
    end

    // Monitor SPI transmissions
    always @(negedge ili_cs) begin
        $display("[%0t] SPI Transfer Block Started. DC = %b", $time, ili_dc);
    end

endmodule
