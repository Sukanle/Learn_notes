// Multi bits logic gate
// y1 = a & b
// y2 = a | b
// y3 = a ^ b
// y4 = ~(a & b)
// y5 = ~(a | b)

// In this module, we have 2 inputs and 6 outputs
// In this part, we will learn how to define multi-bits signals
// In Verilog, multi-bits signals are defined by using [msb:lsb] after `input` or `output` keywords
// `msb` means most significant bit, `lsb` means least significant bit
// The width of the signal is `msb - lsb + 1`
module multBitsGate (
    input [2:0] a, // Signal bit width is 3, it is different from C/C++, the index starts from 0, end at 2.
                   // So the signal has 3 bits: a[2], a[1], a[0]. If this is defined in C/C++, it will be a[0], a[1], a[2].
                   // Verilog is designed for hardware description, so the index starts from 0 to n. Not like C/C++, it starts from 0 to n-1.
    input [2:1] b, // This signal bit also is 3, but this is not required verilog ISO standard.
    output [2:0] y1,
    output [2:0] y2,
    output [2:0] y3,
    output [2:0] y4,
    output [2:0] y5
);
    assign y1 = a & b; // AND gate
    assign y2 = a | b; // OR gate
    assign y3 = a ^ b; // XOR gate
    assign y4 = ~(a & b); // NAND gate
    assign y5 = ~(a | b); // NOR gate
endmodule