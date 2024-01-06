module TOP (
    input clk,
    input BUTTON1,
    input BUTTON2,
    input BUTTON3,
    output [9:0] LED,
    output BNC1,
    input BNC2
    );

    wire sync = !BUTTON3;
    wire data[7:0];
    //assign LED = data[5:0];
    spi_core spi(
        .spi_clk (LED[6]),
        .mosi (LED[8]),
        .spi_to_fpga (LED[5:0]),
        .tx_flag(0),
        .tx_enable(0)
    );
    /* 
    cagen CA1(.clk(clk),
              .en(BUTTON1),
              .rst(!BUTTON2),
              .t0(3),
              .t1(6),
              .q(LED),
              .code(BNC)
            );
    */
    /*nco NCO1(
        .clk(clk),
        .out_sin(f_lokalni),
        //.out_cos(BNC2),
        .div_enable(!BUTTON2),
        .LED(LED[3:0]),
        .sync(sync),
        .phase(!BUTTON1)
        //.LED(LED[3:0])
    );

    wire f_lokalni;
    xor_mixer XOR1(
        .f1(BNC2),
        .f2(f_lokalni),
        .f_out(mixed_signal)
    );

    wire mixed_signal;
    wire output_signal;
    shift_reg #(.bit_count(256), .init_val(256'd0)) FIFO1 (
        .sync(sync),
        .clk(clk),
        .data(mixed_signal),
        .q(output_signal));

    assign BNC1 = output_signal;*/

endmodule

