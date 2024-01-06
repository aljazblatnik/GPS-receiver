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
    assign fout = q[3];
    //assign clk_frac = stevec[3]
    reg clk_frac = 0;
    assign LED = q;
    always @(posedge clk)
    begin
        //stevec <= stevec + 1'b1;
        if (stevec < 'd4) begin
            stevec <= stevec+1;
        end else begin
            stevec <= 0;
            clk_frac <= ~clk_frac;
        end    
    end
endmodule