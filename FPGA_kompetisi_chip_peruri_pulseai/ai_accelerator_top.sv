module ai_accelerator_top (
    input  logic clk_50mhz,  // Clock 50MHz (Pin 27)
    input  logic btn_rst,    // Pin 87 (Active Low Reset)
    output logic led_pin,    // Pin 92 (Debug LED)
    
    // SPI Slave Interface (connected to STM32 SPI3)
    input  logic spi_sck,    // SCK  (Pin 99)
    input  logic spi_mosi,   // MOSI (Pin 101)
    output logic spi_miso,   // MISO (Pin 93)
    input  logic spi_cs_n    // CS_n (Pin 112)
);

    // Reset synchronizer
    logic rst_n_sync1 = 1, rst_n = 1;
    always_ff @(posedge clk_50mhz) begin
        rst_n_sync1 <= btn_rst;
        rst_n <= rst_n_sync1;
    end

    // SPI Slave Module Signals
    logic [783:0] rx_data;
    logic rx_done;
    logic [1:0] tx_data;
    logic tx_ready;

    // Instantiate SPI Slave
    spi_slave spi_inst (
        .clk(clk_50mhz),
        .rst_n(rst_n),
        .sck(spi_sck),
        .mosi(spi_mosi),
        .miso(spi_miso),
        .cs_n(spi_cs_n),
        
        .rx_data(rx_data),
        .rx_done(rx_done),
        .tx_data(tx_data),
        .tx_ready(tx_ready)
    );

    // Instantiate BNN Accelerator
    bnn_accelerator bnn_inst (
        .clk(clk_50mhz),
        .rst_n(rst_n),
        .start(rx_done),       // Start inference when 98 bytes received
        .img_in(rx_data),
        .class_out(tx_data),   // Output to SPI Slave (2 bits)
        .done(tx_ready)
    );

    // Heartbeat LED (Toggles roughly every 0.33 seconds at 50MHz)
    logic [24:0] hb_counter = 0;
    always_ff @(posedge clk_50mhz) begin
        if (!rst_n) begin
            hb_counter <= 0;
        end else begin
            hb_counter <= hb_counter + 1;
        end
    end
    
    assign led_pin = hb_counter[24]; 
    
endmodule
