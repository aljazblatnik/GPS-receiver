/*
 * Miscellaneous functions, commonly used in the project
 */

// Shift register with adjustable bit count
module shift_reg #(parameter bit_count = 8, init_val = 8'b11111111)(
    input sync, 
    input clk, 
    input data,
    output reg [bit_count-1:0] q
    );
    
    always @(posedge clk, posedge sync)
    begin
        if(sync == 1) begin
            q <= init_val;
        end
        else begin
            q <= {data, q[bit_count - 1:1]};
        end
    end  
endmodule

// XOR signal mixer
module xor_mixer (
    input f1,     // E.g. external function generator input
    input f2,     // E.g. a local oscillator
    output f_out  // Subtracted input signals
);
    // Input is a square wave. Expect many harmonics (multiples
    // of the mixed signal) on the output.
    assign f_out = f1 ^ f2;
endmodule
