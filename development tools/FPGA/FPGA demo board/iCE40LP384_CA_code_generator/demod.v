module demod (
    input signal,
);
    cagen CAGEN (

    );

    xor_mixer XOR (
        .f1(signal),
        .f2(code)
    );

endmodule