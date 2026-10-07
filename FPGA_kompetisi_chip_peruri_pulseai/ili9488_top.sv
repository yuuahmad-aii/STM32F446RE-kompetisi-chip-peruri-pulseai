module ili9488_top #(
    parameter MS_COUNT = 50_000 // 50MHz -> 50,000 cycles = 1ms
)(
    input  logic clk_50mhz,  // Clock 50MHz (Pin 27)
    input  logic btn_rst,    // Pin 87 (Active Low Reset)
    input  logic ili_miso,   // Pin 93 (not used internally for TX only)
    output logic ili_led,    // Pin 97
    output logic ili_sck,    // Pin 99
    output logic ili_mosi,   // Pin 101
    output logic ili_dc,     // Pin 105
    output logic ili_rst,    // Pin 110
    output logic ili_cs,     // Pin 112
    output logic led_pin,    // Pin 92 (Debug LED)
    
    // OV7670 Camera Pins
    inout  wire  cam_sda,
    output logic cam_scl,
    input  logic cam_vs,
    input  logic cam_hs,
    input  logic cam_pclk,
    output logic cam_xclk,
    input  logic [7:0] cam_d,
    output logic cam_ret,
    output logic cam_pwdn
);

    // Turn on backlight
    assign ili_led = 1'b1;

    // SPI Master Instantiation
    logic spi_start;
    logic [7:0] spi_data_out;
    logic spi_ready;
    
    // Button synchronizer
    logic rst_n_sync1 = 1;
    logic rst_n = 1;
    always_ff @(posedge clk_50mhz) begin
        rst_n_sync1 <= btn_rst;
        rst_n <= rst_n_sync1;
    end
    logic sys_reset;
    assign sys_reset = ~rst_n; // Active high internal reset
    
    spi_master spi_inst (
        .clk(clk_50mhz),
        .reset(sys_reset),
        .data_in(spi_data_out),
        .start(spi_start),
        .ready(spi_ready),
        .sck(ili_sck),
        .mosi(ili_mosi)
    );

    // Camera XCLK Generation (25MHz)
    logic xclk_reg = 0;
    always_ff @(posedge clk_50mhz) begin
        xclk_reg <= ~xclk_reg;
    end
    assign cam_xclk = xclk_reg;
    assign cam_pwdn = 1'b0;
    assign cam_ret = 1'b1;
    
    // OV7670 I2C Init
    logic cam_sda_out, cam_sda_oe;
    logic ov_init_done;
    ov7670_init cam_init (
        .clk(clk_50mhz),
        .reset(sys_reset),
        .sda_out(cam_sda_out),
        .sda_oe(cam_sda_oe),
        .scl(cam_scl),
        .done(ov_init_done)
    );
    assign cam_sda = cam_sda_oe ? cam_sda_out : 1'bz;
    
    // OV7670 Capture
    ov7670_capture cam_cap (
        .pclk(cam_pclk),
        .reset(sys_reset),
        .vsync(cam_vs),
        .href(cam_hs),
        .d(cam_d),
        .frame_sync(frame_sync),
        .rd_clk(clk_50mhz),
        .rd_en(fifo_rd_en),
        .rd_data(fifo_rd_data),
        .rd_empty(fifo_rd_empty)
    );
    
    logic frame_sync;
    logic frame_sync_sync1 = 0, frame_sync_sync2 = 0, frame_sync_old = 0;
    logic frame_sync_pulse;
    
    always_ff @(posedge clk_50mhz) begin
        frame_sync_sync1 <= frame_sync;
        frame_sync_sync2 <= frame_sync_sync1;
        frame_sync_old <= frame_sync_sync2;
    end
    assign frame_sync_pulse = (frame_sync_sync2 && !frame_sync_old);
    
    logic [15:0] fifo_rd_data;
    logic fifo_rd_empty;
    logic fifo_rd_en = 0;
    logic [15:0] latched_pixel = 0;
    logic [16:0] pixels_drawn = 0;

    // Timers
    logic [31:0] timer = 0;
    logic [15:0] ms_timer = 0;

    // FSM States
    typedef enum logic [4:0] {
        ST_BOOT_WAIT,
        ST_RESET_1, ST_RESET_1_WAIT,
        ST_RESET_2, ST_RESET_2_WAIT,
        ST_RESET_3, ST_RESET_3_WAIT,
        ST_INIT, ST_INIT_WAIT, ST_INIT_DELAY,
        ST_CLEAR_SETUP, ST_CLEAR_FILL,
        ST_WAIT_FRAME,
        ST_WIN_CMD, ST_WIN_CMD_DONE,
        ST_STREAM_POP,
        ST_STREAM_PIXELS,
        ST_DONE
    } state_t;
    
    state_t state = ST_BOOT_WAIT;
    state_t return_state = ST_STREAM_POP;
    
    logic [2:0] repeat_line_cnt = 0;

    // Init ROM (66 commands/data/delays)
    localparam INIT_LEN = 66;
    logic [9:0] init_rom [0:INIT_LEN-1];
    
    initial begin
        // Format: Bit 9: Delay Flag, Bit 8: DC (Data/Cmd), Bit 7:0: Value
        init_rom[0] = 10'h0E0; init_rom[1] = 10'h100; init_rom[2] = 10'h103; init_rom[3] = 10'h109; 
        init_rom[4] = 10'h108; init_rom[5] = 10'h116; init_rom[6] = 10'h10A; init_rom[7] = 10'h13F; 
        init_rom[8] = 10'h178; init_rom[9] = 10'h14C; init_rom[10]= 10'h109; init_rom[11]= 10'h10A; 
        init_rom[12]= 10'h108; init_rom[13]= 10'h116; init_rom[14]= 10'h11A; init_rom[15]= 10'h10F;
        
        init_rom[16]= 10'h0E1; init_rom[17]= 10'h100; init_rom[18]= 10'h116; init_rom[19]= 10'h119; 
        init_rom[20]= 10'h103; init_rom[21]= 10'h10F; init_rom[22]= 10'h105; init_rom[23]= 10'h132; 
        init_rom[24]= 10'h145; init_rom[25]= 10'h146; init_rom[26]= 10'h104; init_rom[27]= 10'h10E; 
        init_rom[28]= 10'h10D; init_rom[29]= 10'h135; init_rom[30]= 10'h137; init_rom[31]= 10'h10F;
        
        init_rom[32]= 10'h0C0; init_rom[33]= 10'h117; init_rom[34]= 10'h115;
        init_rom[35]= 10'h0C1; init_rom[36]= 10'h141;
        init_rom[37]= 10'h0C5; init_rom[38]= 10'h100; init_rom[39]= 10'h112; init_rom[40]= 10'h180;
        init_rom[41]= 10'h036; init_rom[42]= 10'h108; // Portrait BGR (MV=0, BGR=1)
        init_rom[43]= 10'h03A; init_rom[44]= 10'h166; // 18-bit pixel format
        init_rom[45]= 10'h0B0; init_rom[46]= 10'h100;
        init_rom[47]= 10'h0B1; init_rom[48]= 10'h1A0;
        init_rom[49]= 10'h0B4; init_rom[50]= 10'h102;
        init_rom[51]= 10'h0B6; init_rom[52]= 10'h102; init_rom[53]= 10'h102; init_rom[54]= 10'h13B;
        init_rom[55]= 10'h0B7; init_rom[56]= 10'h1C6;
        init_rom[57]= 10'h0F7; init_rom[58]= 10'h1A9; init_rom[59]= 10'h151; init_rom[60]= 10'h12C; init_rom[61]= 10'h182;
        init_rom[62]= 10'h011; init_rom[63]= 10'h278; // Exit sleep, Delay 120ms (0x78)
        init_rom[64]= 10'h029; init_rom[65]= 10'h219; // Display on, Delay 25ms (0x19)
    end

    logic [6:0] init_idx = 0;

    // Window Setup
    logic [3:0] win_idx = 0;

    // Drawing context
    logic [17:0] clear_pixel_cnt = 0;
    logic [1:0]  color_byte_idx = 0;
    logic [8:0]  pixel_x = 0;
    logic [8:0]  pixel_y = 0;
    
    // String Data
    logic [7:0] string_rom [0:10];
    initial begin
        string_rom[0] = "h"; string_rom[1] = "e"; string_rom[2] = "l"; string_rom[3] = "l";
        string_rom[4] = "o"; string_rom[5] = " "; string_rom[6] = "w"; string_rom[7] = "o";
        string_rom[8] = "r"; string_rom[9] = "l"; string_rom[10]= "d";
    end
    
    logic [3:0] char_idx = 0;
    logic [2:0] font_row = 0;
    logic [2:0] font_col = 0;
    logic [1:0] scale_x = 0;
    logic [1:0] scale_y = 0;

    // Font generator function (8x8 simplified)
    function [63:0] get_font(input [7:0] char);
        case (char)
            "h": return 64'h80_80_80_E0_90_90_90_90;
            "e": return 64'h00_00_60_90_F0_80_80_70;
            "l": return 64'h40_40_40_40_40_40_40_60;
            "o": return 64'h00_00_60_90_90_90_90_60;
            " ": return 64'h00_00_00_00_00_00_00_00;
            "w": return 64'h00_00_88_88_88_A8_A8_50;
            "r": return 64'h00_00_B0_C0_80_80_80_80;
            "d": return 64'h08_08_08_68_98_98_98_68;
            default: return 64'hFF_81_81_81_81_81_81_FF;
        endcase
    endfunction

    // Pre-calculate X coordinates for character drawing
    logic [15:0] calc_x0, calc_x1;
    assign calc_x0 = 16'd64 + {12'd0, char_idx} * 16'd32;
    assign calc_x1 = calc_x0 + 16'd31;
    
    // Fetch current character's font data continuously
    logic [63:0] current_char_data;
    assign current_char_data = get_font(string_rom[char_idx]);

    assign led_pin = (state == ST_DONE);

    // Main FSM
    always_ff @(posedge clk_50mhz) begin
        if (sys_reset) begin
            state <= ST_BOOT_WAIT;
            spi_start <= 1'b0;
            ili_cs <= 1'b1;
            ili_rst <= 1'b1;
            ili_dc <= 1'b1;
            init_idx <= 0;
            char_idx <= 0;
            ms_timer <= 500; // 500ms power-on boot wait
            timer <= 0;
        end else begin
            spi_start <= 1'b0; // Default off
            
            case (state)
                // 0. Power-on Wait
                ST_BOOT_WAIT: begin
                    if (timer == MS_COUNT) begin
                        timer <= 0; 
                        if (ms_timer <= 1) state <= ST_RESET_1;
                        else ms_timer <= ms_timer - 16'd1;
                    end else timer <= timer + 1;
                end
                
                // 1. Hardware Reset Sequence
                ST_RESET_1: begin
                    ili_rst <= 1'b1; ili_cs <= 1'b1;
                    ms_timer <= 5; timer <= 0;
                    state <= ST_RESET_1_WAIT;
                end
            ST_RESET_1_WAIT: begin
                if (timer == MS_COUNT) begin
                    timer <= 0; 
                    if (ms_timer <= 1) state <= ST_RESET_2;
                    else ms_timer <= ms_timer - 16'd1;
                end else timer <= timer + 1;
            end
            
            ST_RESET_2: begin
                ili_rst <= 1'b0;
                ms_timer <= 20; timer <= 0;
                state <= ST_RESET_2_WAIT;
            end
            ST_RESET_2_WAIT: begin
                if (timer == MS_COUNT) begin
                    timer <= 0; 
                    if (ms_timer <= 1) state <= ST_RESET_3;
                    else ms_timer <= ms_timer - 16'd1;
                end else timer <= timer + 1;
            end
            
            ST_RESET_3: begin
                ili_rst <= 1'b1;
                ms_timer <= 150; timer <= 0;
                state <= ST_RESET_3_WAIT;
            end
            ST_RESET_3_WAIT: begin
                if (timer == MS_COUNT) begin
                    timer <= 0; 
                    if (ms_timer <= 1) begin
                        state <= ST_INIT;
                        init_idx <= 0;
                        ili_cs <= 1'b0; // Assert CS for the rest of operation
                    end
                    else ms_timer <= ms_timer - 16'd1;
                end else timer <= timer + 1;
            end
            
            // 2. Initialization Commands
            ST_INIT: begin
                if (init_idx == INIT_LEN) begin
                    state <= ST_CLEAR_SETUP; // Done init, go to clear screen
                end else begin
                    if (init_rom[init_idx][9]) begin
                        ms_timer <= {8'h00, init_rom[init_idx][7:0]}; timer <= 0;
                        state <= ST_INIT_DELAY;
                        init_idx <= init_idx + 7'd1;
                    end else begin
                        ili_dc <= init_rom[init_idx][8];
                        spi_data_out <= init_rom[init_idx][7:0];
                        spi_start <= 1'b1;
                        state <= ST_INIT_WAIT;
                    end
                end
            end
            ST_INIT_WAIT: begin
                if (spi_ready && !spi_start) begin
                    init_idx <= init_idx + 7'd1;
                    state <= ST_INIT;
                end
            end
            ST_INIT_DELAY: begin
                ili_cs <= 1'b1; // CS high during init delays (important for sleep out / display on commands)
                if (timer == MS_COUNT) begin
                    timer <= 0; 
                    if (ms_timer <= 1) begin
                        state <= ST_INIT;
                        ili_cs <= 1'b0; // Pull low again
                    end
                    else ms_timer <= ms_timer - 16'd1;
                end else timer <= timer + 1;
            end
            
            // 2. Clear Screen
            ST_CLEAR_SETUP: begin
                win_idx <= 0;
                clear_pixel_cnt <= 0;
                pixel_x <= 0;
                color_byte_idx <= 0;
                return_state <= ST_CLEAR_FILL;
                state <= ST_WIN_CMD;
            end
            
            ST_CLEAR_FILL: begin
                if (clear_pixel_cnt == 18'd153600 && spi_ready && !spi_start) begin
                    state <= ST_WAIT_FRAME; // Start camera capture
                    ili_cs <= 1'b1;
                end else if (clear_pixel_cnt < 18'd153600 && spi_ready && !spi_start) begin
                    spi_data_out <= 8'h00; // Black background
                    spi_start <= 1'b1;
                    
                    if (color_byte_idx == 2) begin
                        color_byte_idx <= 0;
                        clear_pixel_cnt <= clear_pixel_cnt + 18'd1;
                    end else begin
                        color_byte_idx <= color_byte_idx + 2'd1;
                    end
                end
            end
            
            // 3. Camera Render Loop
            ST_WAIT_FRAME: begin
                if (frame_sync_pulse) begin
                    win_idx <= 0;
                    return_state <= ST_STREAM_POP;
                    state <= ST_WIN_CMD;
                    ili_cs <= 1'b1;
                    pixels_drawn <= 0;
                end
            end
            
            ST_WIN_CMD: begin
                if (win_idx == 11 && spi_ready && !spi_start) begin
                    state <= ST_WIN_CMD_DONE;
                    ili_cs <= 1'b1; // Pulse CS High after window command
                    ili_dc <= 1'b1; // Switch to Data for pixel sending
                end else if (win_idx < 11 && spi_ready && !spi_start) begin
                    case (win_idx)
                        0:  begin ili_dc <= 1'b0; spi_data_out <= 8'h2A; end
                        1:  begin ili_dc <= 1'b1; spi_data_out <= 8'h00; end
                        2:  begin ili_dc <= 1'b1; spi_data_out <= 8'h00; end
                        3:  begin ili_dc <= 1'b1; spi_data_out <= 8'h01; end
                        4:  begin ili_dc <= 1'b1; spi_data_out <= 8'h3F; end // 319
                        5:  begin ili_dc <= 1'b0; spi_data_out <= 8'h2B; end
                        6:  begin ili_dc <= 1'b1; spi_data_out <= 8'h00; end
                        7:  begin ili_dc <= 1'b1; spi_data_out <= (return_state == ST_CLEAR_FILL) ? 8'h00 : 8'd120; end
                        8:  begin ili_dc <= 1'b1; spi_data_out <= (return_state == ST_CLEAR_FILL) ? 8'h01 : 8'h01; end
                        9:  begin ili_dc <= 1'b1; spi_data_out <= (return_state == ST_CLEAR_FILL) ? 8'hDF : 8'h67; end // 120 + 239 = 359
                        10: begin ili_dc <= 1'b0; spi_data_out <= 8'h2C; end
                    endcase
                    spi_start <= 1'b1;
                    win_idx <= win_idx + 4'd1;
                end
            end
            ST_WIN_CMD_DONE: begin
                ili_cs <= 1'b0; // Back to low for memory write
                state <= return_state;
            end
            
            ST_STREAM_POP: begin
                fifo_rd_en <= 0;
                if (frame_sync_pulse) begin
                    win_idx <= 0;
                    return_state <= ST_STREAM_POP;
                    state <= ST_WIN_CMD;
                    ili_cs <= 1'b1;
                    pixels_drawn <= 0;
                end else if (pixels_drawn == 17'd76800) begin
                    // Frame finished (320x240)
                    state <= ST_WAIT_FRAME;
                    ili_cs <= 1'b1;
                end else if (!fifo_rd_empty) begin
                    // Data is valid!
                    latched_pixel <= fifo_rd_data;
                    fifo_rd_en <= 1; // Pop it for the next iteration
                    state <= ST_STREAM_PIXELS;
                    color_byte_idx <= 0;
                end
            end
            
            ST_STREAM_PIXELS: begin
                fifo_rd_en <= 0; // De-assert!
                if (frame_sync_pulse) begin
                    win_idx <= 0;
                    return_state <= ST_STREAM_POP;
                    state <= ST_WIN_CMD;
                    ili_cs <= 1'b1;
                    pixels_drawn <= 0;
                end else if (spi_ready && !spi_start) begin
                    if (color_byte_idx == 0) begin
                        spi_data_out <= {latched_pixel[15:11], 3'b000};
                        spi_start <= 1'b1;
                        color_byte_idx <= 1;
                    end else if (color_byte_idx == 1) begin
                        spi_data_out <= {latched_pixel[10:5], 2'b00};
                        spi_start <= 1'b1;
                        color_byte_idx <= 2;
                    end else if (color_byte_idx == 2) begin
                        spi_data_out <= {latched_pixel[4:0], 3'b000};
                        spi_start <= 1'b1;
                        color_byte_idx <= 3;
                    end else if (color_byte_idx == 3) begin
                        pixels_drawn <= pixels_drawn + 17'd1;
                        state <= ST_STREAM_POP;
                    end
                end
            end
            
            ST_DONE: begin
                // Display finished, stay in idle state
            end
        endcase
        end // end of sys_reset else
    end
endmodule

// ==========================================
// SPI Master (Mode 0) 
// Slower speed for ILI9488 compatibility (~8.33 MHz at 50MHz sys clock)
// ==========================================
module spi_master (
    input  logic clk,
    input  logic reset,
    input  logic [7:0] data_in,
    input  logic start,
    output logic ready,
    output logic sck,
    output logic mosi
);
    logic [7:0] shift_reg = 0;
    logic [3:0] bit_count = 0;
    logic [7:0] clk_div = 0;
    logic active = 0;

    always_ff @(posedge clk) begin
        if (reset) begin
            ready <= 1'b1; active <= 1'b0; sck <= 1'b0; mosi <= 1'b0;
        end else begin
            if (start && ready) begin
                shift_reg <= {data_in[6:0], 1'b0};
                mosi <= data_in[7]; // Setup first bit immediately
                bit_count <= 0;
                ready <= 1'b0;
                active <= 1'b1;
                clk_div <= 0;
                sck <= 1'b0;
            end else if (active) begin
                clk_div <= clk_div + 8'd1;
                if (clk_div == 8'd2) begin
                    sck <= 1'b1; // Rising edge (Slave samples)
                end else if (clk_div == 8'd5) begin
                    sck <= 1'b0; // Falling edge (Master shifts)
                    mosi <= shift_reg[7]; 
                    shift_reg <= {shift_reg[6:0], 1'b0};
                    bit_count <= bit_count + 4'd1;
                    if (bit_count == 7) begin
                        active <= 1'b0;
                        ready <= 1'b1;
                    end
                    clk_div <= 0; // Reset divider
                end
            end
        end
    end
endmodule
