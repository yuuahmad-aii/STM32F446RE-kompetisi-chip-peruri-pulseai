module ov7670_capture(
    input  logic pclk,
    input  logic reset,
    input  logic vsync,
    input  logic href,
    input  logic [7:0] d,
    
    output logic frame_sync, // pulses high when a new frame starts
    
    // FIFO Read interface
    input  logic rd_clk,
    input  logic rd_en,
    output logic [15:0] rd_data,
    output logic rd_empty
);

    logic [7:0] d_latch;
    logic href_latch;
    logic vsync_latch;
    
    logic [15:0] shift_reg;
    logic byte_toggle = 0;
    
    logic vsync_old = 0;
    logic href_old = 0;
    
    logic [15:0] pixel_data;
    logic pixel_valid;

    always_ff @(posedge pclk) begin
        if (reset) begin
            byte_toggle <= 0;
            pixel_valid <= 0;
            vsync_old <= 0;
            href_old <= 0;
            frame_sync <= 0;
        end else begin
            pixel_valid <= 0;
            frame_sync <= 0;
            
            d_latch <= d;
            href_latch <= href;
            vsync_latch <= vsync;
            
            vsync_old <= vsync_latch;
            href_old <= href_latch;
            
            // OV7670 VSYNC is active high. When it falls, active frame begins.
            // We pulse frame_sync on the falling edge of vsync to signal the LCD controller to reset its pointers.
            if (vsync_old && !vsync_latch) begin
                byte_toggle <= 0;
                frame_sync <= 1;
            end
            
            if (href_latch) begin
                shift_reg <= {shift_reg[7:0], d_latch};
                byte_toggle <= ~byte_toggle;
                
                if (byte_toggle) begin
                    pixel_data <= {shift_reg[7:0], d_latch};
                    pixel_valid <= 1;
                end
            end else begin
                byte_toggle <= 0;
            end
        end
    end

    // Altera DCFIFO Primitive (Asynchronous FIFO)
    // 2048 words deep, 16 bits wide
    dcfifo #(
        .intended_device_family("MAX 10"),
        .lpm_numwords(2048),
        .lpm_showahead("ON"),
        .lpm_type("dcfifo"),
        .lpm_width(16),
        .lpm_widthu(11),
        .overflow_checking("ON"),
        .rdsync_delaypipe(4),
        .underflow_checking("ON"),
        .use_eab("ON"),
        .wrsync_delaypipe(4)
    ) cam_fifo (
        .data(pixel_data),
        .rdclk(rd_clk),
        .rdreq(rd_en),
        .wrclk(pclk),
        .wrreq(pixel_valid),
        .q(rd_data),
        .rdempty(rd_empty),
        .wrfull(),
        .wrempty(),
        .rdfull(),
        .rdusedw(),
        .wrusedw(),
        .aclr(reset | frame_sync) 
    );
    
endmodule
