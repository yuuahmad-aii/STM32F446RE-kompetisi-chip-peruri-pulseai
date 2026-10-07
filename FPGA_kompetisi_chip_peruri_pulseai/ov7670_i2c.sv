module i2c_sender(
    input  logic clk,
    input  logic reset,
    input  logic [7:0] dev_addr,
    input  logic [7:0] reg_addr,
    input  logic [7:0] reg_data,
    input  logic start,
    output logic ready,
    output logic sda_out,
    output logic sda_oe,
    output logic scl
);
    logic [7:0] divider = 0;
    logic i2c_clk = 0; // slower clock for I2C (e.g. 50MHz / 256 = 195kHz)

    always_ff @(posedge clk) begin
        divider <= divider + 1;
        if (divider == 0) i2c_clk <= ~i2c_clk;
    end

    logic [28:0] shift_reg;
    logic [5:0]  state = 0;
    
    // 29-bit transmission:
    // [28] = Start bit (0)
    // [27:20] = Device Addr + Write bit (dev_addr)
    // [19] = ACK (Z)
    // [18:11] = Reg Addr
    // [10] = ACK (Z)
    // [9:2] = Data
    // [1] = ACK (Z)
    // [0] = Stop bit (0 -> 1)

    initial begin
        ready = 1;
        sda_out = 1;
        sda_oe = 0;
        scl = 1;
    end

    always_ff @(posedge i2c_clk or posedge reset) begin
        if (reset) begin
            state <= 0;
            ready <= 1;
            sda_out <= 1;
            sda_oe <= 0;
            scl <= 1;
        end else begin
            if (state == 0) begin
                ready <= 1;
                scl <= 1;
                sda_oe <= 0; // High-Z (pull-up to 1)
                if (start) begin
                    shift_reg <= {1'b0, dev_addr, 1'b0, reg_addr, 1'b0, reg_data, 1'b0, 1'b0};
                    state <= 1;
                    ready <= 0;
                end
            end else if (state == 1) begin // Start condition SDA low while SCL high
                sda_out <= 0;
                sda_oe <= 1;
                state <= 2;
            end else if (state >= 2 && state <= 30) begin // 29 data/ack bits
                if (scl) begin
                    scl <= 0;
                end else begin
                    // Shift out next bit
                    if (state == 11 || state == 20 || state == 29) begin
                        // ACK slots, let slave drive SDA
                        sda_oe <= 0;
                    end else if (state == 30) begin
                        // Stop condition prep
                        sda_out <= 0;
                        sda_oe <= 1;
                    end else begin
                        sda_out <= shift_reg[28 - (state - 2)];
                        sda_oe <= 1;
                    end
                    scl <= 1;
                    state <= state + 1;
                end
            end else if (state == 31) begin // Stop condition SDA high while SCL high
                if (scl) begin
                    scl <= 0;
                end else begin
                    sda_out <= 1;
                    sda_oe <= 0; // high Z
                    scl <= 1;
                    state <= 32;
                end
            end else if (state == 32) begin
                ready <= 1;
                state <= 0;
            end
        end
    end
endmodule

module ov7670_init (
    input  logic clk,
    input  logic reset,
    output logic sda_out,
    output logic sda_oe,
    output logic scl,
    output logic done
);

    logic i2c_start;
    logic i2c_ready;
    logic [7:0] i2c_reg_addr;
    logic [7:0] i2c_reg_data;
    
    i2c_sender i2c (
        .clk(clk),
        .reset(reset),
        .dev_addr(8'h42), // OV7670 Write address
        .reg_addr(i2c_reg_addr),
        .reg_data(i2c_reg_data),
        .start(i2c_start),
        .ready(i2c_ready),
        .sda_out(sda_out),
        .sda_oe(sda_oe),
        .scl(scl)
    );

    logic [7:0] rom_addr = 0;
    logic [15:0] rom_data;

    // OV7670 Configuration ROM for QVGA RGB565
    always_comb begin
        case (rom_addr)
            // Reset
            0: rom_data = 16'h12_80; // COM7 Reset
            // Delay added manually via state machine if needed, or just let it run (I2C is slow anyway)
            
            // Format QVGA, RGB
            1: rom_data = 16'h12_14; // COM7 QVGA, RGB
            2: rom_data = 16'h40_D0; // COM15 RGB565
            3: rom_data = 16'h8C_00; // RGB444 disable
            4: rom_data = 16'h11_0F; // CLKRC prescaler (Divide by 16 to get ~2 FPS, prevents SPI FIFO overflow)
            5: rom_data = 16'h09_03; // COM2
            6: rom_data = 16'h15_20; // COM10 PCLK reverse
            
            // Frame rate / Window
            7: rom_data = 16'h32_80; // HREF
            8: rom_data = 16'h17_16; // HSTART
            9: rom_data = 16'h18_04; // HSTOP
            10: rom_data = 16'h19_02; // VSTART
            11: rom_data = 16'h1A_7A; // VSTOP
            12: rom_data = 16'h03_0A; // VREF
            
            // Color matrix (RGB565)
            13: rom_data = 16'h4F_B3;
            14: rom_data = 16'h50_B3;
            15: rom_data = 16'h51_00;
            16: rom_data = 16'h52_3D;
            17: rom_data = 16'h53_A7;
            18: rom_data = 16'h54_E4;
            19: rom_data = 16'h58_9E;
            
            // Magic registers
            20: rom_data = 16'hB0_84; 
            21: rom_data = 16'hB1_0C; 
            22: rom_data = 16'hB2_0E; 
            23: rom_data = 16'hB3_82; 
            24: rom_data = 16'hB8_0A; 
            
            // End marker
            default: rom_data = 16'hFF_FF;
        endcase
    end

    typedef enum {S_IDLE, S_WAIT_RESET, S_SEND, S_WAIT} state_t;
    state_t state = S_WAIT_RESET;
    logic [23:0] delay_cnt = 0;

    always_ff @(posedge clk) begin
        if (reset) begin
            state <= S_WAIT_RESET;
            rom_addr <= 0;
            i2c_start <= 0;
            done <= 0;
            delay_cnt <= 0;
        end else begin
            case (state)
                S_WAIT_RESET: begin
                    if (delay_cnt == 24'd5_000_000) begin // wait a bit after reset
                        state <= S_SEND;
                    end else delay_cnt <= delay_cnt + 1;
                end
                S_SEND: begin
                    if (rom_data == 16'hFFFF) begin
                        done <= 1;
                        state <= S_IDLE;
                    end else if (i2c_ready) begin
                        i2c_reg_addr <= rom_data[15:8];
                        i2c_reg_data <= rom_data[7:0];
                        i2c_start <= 1;
                        state <= S_WAIT;
                    end
                end
                S_WAIT: begin
                    i2c_start <= 0;
                    if (!i2c_ready) begin
                        // Wait for it to become ready again
                    end else if (i2c_ready && !i2c_start) begin
                        rom_addr <= rom_addr + 1;
                        // Extra delay after reset command
                        if (rom_addr == 0) begin
                            state <= S_WAIT_RESET; 
                            delay_cnt <= 0;
                        end else begin
                            state <= S_SEND;
                        end
                    end
                end
                S_IDLE: begin
                    // Done
                end
            endcase
        end
    end
endmodule
