// Three-state logic in Verilog

module triState (
    input [3:0] a,
    input en,
    output [3:0] y
);
    // `4'bz` means 4-bit high impedance
    assign y = en ? a : 4'bz; // If enable is high, output a; else output high impedance (z)
    // Format style: <number_of_bits>'<base><value>
    // Base: b - binary, o - octal, d - decimal, h - hexadecimal(Uppercase or lowercase not matter)
    // Value: you can use `_` to separate the number for better readability
    // Example:
    //          4'b1010 -> 4-bit binary 1010(10 in decimal)
    //          8'hFF   -> 8-bit hexadecimal FF(255 in decimal)
    //          16'd255  -> 16-bit decimal 255
    //          4'o17    -> 4-bit octal 17(15 in decimal)
    //          8'b1010_1101 -> 8-bit binary 10101101(173 in decimal)
    // Note:
    // Default base is decimal if not specified
    // If you not specify the number of bits, 
    // it will be the minimum number of bits to represent the value(compiler will auto calculate)
    // Example: 'b1010 -> 4-bit binary 1010 (10 in decimal, minimum 4 bits to represent 10)
endmodule