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

    // Synchronizers for SPI inputs
    logic sck_sync1, sck_sync2;
    logic mosi_sync1, mosi_sync2;
    logic cs_n_sync1, cs_n_sync2;
    
    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            {sck_sync2, sck_sync1} <= 2'b00;
            {mosi_sync2, mosi_sync1} <= 2'b00;
            {cs_n_sync2, cs_n_sync1} <= 2'b11;
        end else begin
            {sck_sync2, sck_sync1} <= {sck_sync1, sck};
            {mosi_sync2, mosi_sync1} <= {mosi_sync1, mosi};
            {cs_n_sync2, cs_n_sync1} <= {cs_n_sync1, cs_n};
        end
    end

    logic sck_rising_edge;
    logic sck_falling_edge;
    assign sck_rising_edge  = (sck_sync1 == 1'b1) && (sck_sync2 == 1'b0);
    assign sck_falling_edge = (sck_sync1 == 1'b0) && (sck_sync2 == 1'b1);

    logic [9:0] bit_cnt; // up to 784
    logic [783:0] shift_reg;
    logic [7:0] tx_shift_reg;

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            bit_cnt <= 0;
            rx_done <= 0;
            shift_reg <= 0;
            rx_data <= 0;
            tx_shift_reg <= 0;
        end else begin
            rx_done <= 0;
            
            if (cs_n_sync2) begin
                bit_cnt <= 0;
                // If inference is done, load it into tx register. 
                // We add 0xAA as a marker to show it's a valid result. (e.g. 101010xx)
                if (tx_ready) tx_shift_reg <= {6'b101010, tx_data};
                else tx_shift_reg <= 8'h00; 
            end else begin
                // Sample MOSI on SCK rising edge (SPI Mode 0: CPHA=0, CPOL=0)
                if (sck_rising_edge) begin
                    shift_reg <= {shift_reg[782:0], mosi_sync2};
                    bit_cnt <= bit_cnt + 1;
                    
                    if (bit_cnt == 783) begin
                        rx_data <= {shift_reg[782:0], mosi_sync2};
                        rx_done <= 1'b1;
                    end
                end
                
                // Shift MISO on SCK falling edge
                if (sck_falling_edge) begin
                    tx_shift_reg <= {tx_shift_reg[6:0], 1'b0};
                end
            end
        end
    end
    
    // MISO is sent MSB first
    assign miso = cs_n_sync2 ? 1'bZ : tx_shift_reg[7];

endmodule
