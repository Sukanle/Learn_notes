// byte(8 bits) AND gate
// y = a[0] & a[1] & a[2] & a[3] & a[4] & a[5] & a[6] & a[7]

module byteAND (
    input [7:0] a,
    output y
);
    // Use reduction AND operator (&) to reduce all bits of a to a single bit
    // assign y = a[0] & a[1] & a[2] & a[3] & a[4] & a[5] & a[6] & a[7];
    // Easily to use reduction AND operator (&)
    assign y = &a; // If only one operand, it will reduce all bits of the operand to a single bit
                   // It likes `for` loop in C/C++, but operation changes to bits, not numbers
                   // It is equivalent to:
                   // `11111111` -> `1`
                   // if any bit is `0`, the result is `0`
                   // `11111110` -> `0`
endmodule