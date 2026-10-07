import os

output_file = 'bnn_accelerator.sv'
with open(output_file, 'w') as f:
    f.write('''module bnn_accelerator (
    input  logic clk,
    input  logic rst_n,
    input  logic start,
    input  logic [783:0] img_in,
    output logic [1:0] class_out,
    output logic done
);

    `include "weights.vh"

    // FSM States
    typedef enum logic [3:0] {
        IDLE,
        L1_READ,
        L1_WAIT,
        L1_CALC,
        L2_READ,
        L2_WAIT,
        L2_CALC,
        L3_READ,
        L3_WAIT,
        L3_CALC,
        DONE
    } state_t;
    state_t state;

    logic [6:0] counter;
    
    // Accumulators & Outputs
    logic [31:0] l1_out;
    logic [15:0] l2_out;
    int l3_popcount [0:3];

    // Pipeline Registers for ROM
    logic [783:0] l1_w_reg;
    int l1_t_reg;
    logic [31:0] l2_w_reg;
    int l2_t_reg;
    logic [15:0] l3_w_reg;
    int l3_t_reg;

    always_ff @(posedge clk) begin
        // Layer 1 ROM (Synthesized to LEs since MAX10 SC does not support ERAM)
        case (counter[4:0])
''')
    for i in range(32):
        f.write(f"            {i}: begin l1_w_reg <= L1_N{i}_W; l1_t_reg <= L1_N{i}_T; end\n")
    f.write('''            default: begin l1_w_reg <= 0; l1_t_reg <= 0; end
        endcase
        
        // Layer 2 ROM
        case (counter[3:0])
''')
    for i in range(16):
        f.write(f"            {i}: begin l2_w_reg <= L2_N{i}_W; l2_t_reg <= L2_N{i}_T; end\n")
    f.write('''            default: begin l2_w_reg <= 0; l2_t_reg <= 0; end
        endcase

        // Layer 3 ROM
        case (counter[1:0])
''')
    for i in range(4):
        f.write(f"            {i}: begin l3_w_reg <= L3_N{i}_W; l3_t_reg <= L3_N{i}_T; end\n")
    f.write('''            default: begin l3_w_reg <= 0; l3_t_reg <= 0; end
        endcase
    end

    // Popcount logic for L1
    logic [783:0] l1_xnor;
    int l1_pop;
    assign l1_xnor = ~(img_in ^ l1_w_reg);
    
    always_comb begin
        l1_pop = 0;
        for (int i=0; i<784; i++) begin
            l1_pop += l1_xnor[i];
        end
    end

    // Popcount logic for L2
    logic [31:0] l2_xnor;
    int l2_pop;
    assign l2_xnor = ~(l1_out ^ l2_w_reg);
    
    always_comb begin
        l2_pop = 0;
        for (int i=0; i<32; i++) begin
            l2_pop += l2_xnor[i];
        end
    end

    // Popcount logic for L3
    logic [15:0] l3_xnor;
    int l3_pop;
    assign l3_xnor = ~(l2_out ^ l3_w_reg);
    
    always_comb begin
        l3_pop = 0;
        for (int i=0; i<16; i++) begin
            l3_pop += l3_xnor[i];
        end
    end

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state <= IDLE;
            counter <= 0;
            done <= 0;
            class_out <= 0;
            l1_out <= 0;
            l2_out <= 0;
        end else begin
            case (state)
                IDLE: begin
                    done <= 0;
                    counter <= 0;
                    if (start) state <= L1_READ;
                end
                
                L1_READ: state <= L1_WAIT;
                L1_WAIT: state <= L1_CALC;
                L1_CALC: begin
                    if (l1_pop >= l1_t_reg) l1_out[counter] <= 1'b1;
                    else l1_out[counter] <= 1'b0;
                    
                    if (counter == 31) begin
                        counter <= 0;
                        state <= L2_READ;
                    end else begin
                        counter <= counter + 1;
                        state <= L1_READ;
                    end
                end
                
                L2_READ: state <= L2_WAIT;
                L2_WAIT: state <= L2_CALC;
                L2_CALC: begin
                    if (l2_pop >= l2_t_reg) l2_out[counter] <= 1'b1;
                    else l2_out[counter] <= 1'b0;
                    
                    if (counter == 15) begin
                        counter <= 0;
                        state <= L3_READ;
                    end else begin
                        counter <= counter + 1;
                        state <= L2_READ;
                    end
                end
                
                L3_READ: state <= L3_WAIT;
                L3_WAIT: state <= L3_CALC;
                L3_CALC: begin
                    l3_popcount[counter] <= l3_pop;
                    
                    if (counter == 3) begin
                        counter <= 0;
                        state <= DONE;
                    end else begin
                        counter <= counter + 1;
                        state <= L3_READ;
                    end
                end
                
                DONE: begin
                    if (l3_popcount[0] >= l3_popcount[1] && l3_popcount[0] >= l3_popcount[2] && l3_popcount[0] >= l3_popcount[3]) class_out <= 2'd0;
                    else if (l3_popcount[1] >= l3_popcount[0] && l3_popcount[1] >= l3_popcount[2] && l3_popcount[1] >= l3_popcount[3]) class_out <= 2'd1;
                    else if (l3_popcount[2] >= l3_popcount[0] && l3_popcount[2] >= l3_popcount[1] && l3_popcount[2] >= l3_popcount[3]) class_out <= 2'd2;
                    else class_out <= 2'd3;
                    
                    done <= 1;
                    if (!start) state <= IDLE;
                end
            endcase
        end
    end
endmodule
''')
