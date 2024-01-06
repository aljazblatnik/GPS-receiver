module nco(
        input clk,
        input rst,
        input [8:0] in_f_correct,
        input phase [1:0], 
        output fout,
        output [3:0] LED
    );
    wire [3:0] q;
    
    reg [7:0] stevec;
    shift_reg #(.bit_count(4),.init_val(4'b1000)) U1(.sync(rst), .clk(clk_frac), .data(q[0]), .q(q));
    //assign fout = q[3];
    //assign clk_frac = stevec[3]
    assign fout = clk_frac;
    reg clk_frac = 0;
    //reg clk_internal = 0;
    assign LED = q;
    always @(posedge clk)
    begin
        stevec <= stevec+1;
    end
    assign clk_internal = (stevec=='d10) ? 1 : 0;
    always @(posedge clk_internal)
    begin
        clk_frac <= ~clk_frac;
        stevec <= 0;
    end
endmodule