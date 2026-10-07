module spi_slave (
    input  logic clk,       // System clock
    input  logic rst_n,
    
    // SPI Pins
    input  logic sck,
    input  logic mosi,
    output logic miso,
    input  logic cs_n,
    
    // Interface to BNN
    output logic [783:0] rx_data, // 98 bytes (784 bits) of image
    output logic rx_done,         // Pulse when 98 bytes are received
    input  logic [1:0] tx_data,   // 2-bit result (0 to 3) to send back
    input  logic tx_ready         // High when inference is done
);

    // ========================================================
    // 50 MHz CLOCK DOMAIN (System)
    // ========================================================
    logic [7:0] tx_buffer;
    
    // Latch inference result whenever it's ready.
    // This buffer is stable during the SPI transaction.
    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            tx_buffer <= 8'h00;
        end else if (tx_ready) begin
            // Marker 0xA8 (101010) + tx_data (2 bit)
            tx_buffer <= {6'b101010, tx_data};
        end
    end

    // Synchronize rx_done_toggle from SPI domain to 50MHz domain
    logic [2:0] rx_done_sync;
    logic rx_done_toggle;
    
    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            rx_done_sync <= 0;
            rx_done <= 0;
        end else begin
            rx_done_sync <= {rx_done_sync[1:0], rx_done_toggle};
            // Detect edge of toggle signal to generate a 1-cycle pulse
            rx_done <= (rx_done_sync[2] ^ rx_done_sync[1]);
        end
    end

    // ========================================================
    // SPI CLOCK DOMAIN (SCK)
    // ========================================================
    logic [9:0] bit_cnt;
    logic [783:0] shift_reg;
    logic [7:0] tx_shift_reg;

    // SCK Rising Edge: Sample MOSI
    always_ff @(posedge sck or posedge cs_n or negedge rst_n) begin
        if (!rst_n) begin
            bit_cnt <= 0;
            rx_done_toggle <= 0;
        end else if (cs_n) begin
            bit_cnt <= 0;
            // We do NOT clear rx_data or rx_done_toggle here, 
            // they must persist for the 50MHz domain to read them!
        end else begin
            shift_reg <= {shift_reg[782:0], mosi};
            bit_cnt <= bit_cnt + 1;
            
            if (bit_cnt == 783) begin
                rx_data <= {shift_reg[782:0], mosi};
                rx_done_toggle <= ~rx_done_toggle; // Toggle to signal 50MHz domain
            end
        end
    end

    // SCK Falling Edge: Shift MISO
    always_ff @(negedge sck or posedge cs_n or negedge rst_n) begin
        if (!rst_n) begin
            tx_shift_reg <= 8'h00;
        end else if (cs_n) begin
            // When CS goes high (Idle), load the shift register with the latest buffer!
            tx_shift_reg <= tx_buffer;
        end else begin
            // Shift left on every falling edge
            tx_shift_reg <= {tx_shift_reg[6:0], 1'b0};
        end
    end

    // MISO is sent MSB first, tri-stated when CS is high
    assign miso = cs_n ? 1'bZ : tx_shift_reg[7];

endmodule
