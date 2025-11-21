// Two ways switch between bytes

module QuadBitsSwitch (
    input [3:0] src,
    input [3:0] dst,
    input sel,
    output [3:0] y
);

    assign y = sel ? src : dst;
endmodule

module byteSwitch (
    input [7:0] src,
    input [7:0] dst,
    input sel,
    output [7:0] y
);
// To use module, just like to call OOP class constructor, just like C++/Java object instantiation
// Also, you can use C/C++(C99/C++20) designated initializer style to instantiate module
// Example:
// Traditional style(Constructor style):
// Attention: the arguments order must be the same as the module definition
QuadBitsSwitch lsbmux(src[3:0], dst[3:0], sel, y[3:0]);
// Designated initializer style:
// Different order is allowed, but the names must be correct
QuadBitsSwitch msbmux(
    .src    (src[7:4]),
    .dst    (dst[7:4]),
    .sel    (sel),
    .y      (y[7:4])
);
// Bits-by-bit splice
// Example:
// wire a[3:0] = 4'b1010;
// wire b = 1'b1;
// wire y[15:0];
// assign y = {a[3:0], {3{b[0]}}, a[0], 8'b1010_1010};
// -> y = a[3] a[2] a[1] a[0] b[0] b[0] b[0] a[0] 1 0 1 0 1 0 1 0
// -> y = 1    0    1    0    1    1    1    0    1 0 1 0 1 0 1 0
// -> y = 12'b1010_1110_1010_1010
// -> y = 0xAEA (hexadecimal)
// -> y = 0b1010111010101010 (binary)
// -> y = 0o12752 (octal)
// -> y = 0d4474 (decimal)
endmodule