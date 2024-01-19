module spi_core (
    input spi_clk,
    output reg miso,
    input mosi,
    input spi_ss,


    input rst,
    output [7:0] spi_to_fpga,
    inout [7:0] fpga_to_spi,
    input tx_enable,
    input tx_flag // potrebuje začetno stanje!
    );
    reg [7:0] spi_to_fpga_reg;
    reg [3:0] stevec_in  = 0;
    reg [3:0] stevec_out = 0; 
    reg [7:0] reg_in;
    reg tx_flag_reg;
    //wire [7:0] reg_out;

    assign reg_out = fpga_to_spi;
    assign spi_to_fpga = spi_to_fpga_reg;
    assign tx_flag = tx_flag_reg;

    always @(posedge spi_clk) begin

        if (tx_enable == 1) begin 
                //Tx part of spi_core
                if (tx_flag_reg == 1) begin
                    miso <= reg_out[stevec_out];
                    stevec_out <= stevec_out + 1;
                    if (stevec_out == 7) begin
                        tx_flag_reg <= 0;
                    end
                end
            
            
        end else begin
            //Rx part of spi_core
            reg_in[stevec_in] <= mosi;
            stevec_in <= stevec_in + 1;
            if (stevec_in == 7) begin
                spi_to_fpga_reg <= reg_in;
            end   
        end 

    end



endmodule