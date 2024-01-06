// Numerically Controlled Oscillator module
//module nco #(parameter cnt_val = 25_000,cnt_val_half = 12_500,add_div=10_400)(
module nco #(parameter cnt_val=125000, cnt_val_half=62500, add_div=10400)(                   
        input clk,
        input div_enable,
        output out_sin,
        output out_cos,
        input sync,
        input phase,
        output [3:0] LED
        //output reg NCO_clk

    );
    reg NCO_clk = 'd0;
    reg [23:0] counter = 'd0; 
    reg [7:0] f_correct = 'd0;

    always @(posedge clk) begin
        if (div_enable == 1) begin
            //f_correct <= 1;//100;
            f_correct <= 1;
            end
            else begin
                f_correct <= 0;
            end
        if (counter >= cnt_val) begin
            NCO_clk <= 1;
            counter <= counter - cnt_val + add_div + f_correct;
            end
        else begin
            counter <= counter + add_div + f_correct;
            // Preklop med visoko in nizko uro
            if(counter >= cnt_val_half) begin
                NCO_clk <= 0;
            end
        end
    end

    vco VCO1(
        .clk(NCO_clk),
        .out_sin(out_sin),
        .out_cos(out_cos),
        .LED(LED),
        .sync(sync),
        .phase(phase)
    );
endmodule

module vco (
    input clk,
    input div,
    input phase,
    input sync,
    output out_sin,
    output out_cos,
    output [3:0] LED
);

    reg r [1:0];
    wire [3:0] q;
    shift_reg #(.bit_count(4),.init_val(4'b0011)) U1 (
        .sync(sync),
        .clk(clk),
        .data(q[0]),
        .q(q)
    );

    reg sin_data;
    reg cos_data;
    assign out_sin = sin_data;
    assign out_cos = cos_data;
    assign LED = q;
    
    always @(posedge clk) begin
        case (phase)
            0: begin
                sin_data <= q[3];
                cos_data <= q[0];
            end
            1: begin
                sin_data <= q[0];
                cos_data <= q[1];
            end
            2: begin
                sin_data <= q[1];
                cos_data <= q[2];
            end
            3: begin
                sin_data <= q[2];
                cos_data <= q[3];
            end
            default: begin
                sin_data <= q[3];
                cos_data <= q[0];
            end
        endcase
    end

endmodule