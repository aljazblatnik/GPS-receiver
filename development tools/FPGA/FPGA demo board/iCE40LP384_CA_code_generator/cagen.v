// CA Code Generator module
module cagen (
    input clk,
    input en,
    input rst,
    input [3:0] t0,
    input [3:0] t1,
    output [9:0] q,
    output code
    );
    
    reg stanje = 0;
    wire data1, data2;
    wire [9:0] q_G1;
    wire [9:0] q_G2;
    reg [4:0] clk_divider = 5'b11111;

    shift_reg #(.bit_count(10),.init_val(10'b1111111111)) G1 (
        .sync(rst),
        .clk(clk_divider[4]),
        .data(data1),
        .q(q_G1)
    );

    shift_reg #(.bit_count(10),.init_val(10'b1111111111)) G2 (
        .sync(rst),
        .clk(clk_divider[4]),
        .data(data2),
        .q(q_G2)
    );

    always @(posedge clk)
    begin
        clk_divider <= clk_divider + 1'b1;
    end

    assign data1 = q_G1[2]^q_G1[9];
    assign data2 = ((q_G2[9]^q_G2[8])^(q_G2[5]^q_G2[7]))^(q_G2[1]^q_G2[2]); 

    assign code = en & ((q_G2[t0]^q_G2[t1])^q_G1[9]);
    assign q = en ? q_G1 : 0;
endmodule
