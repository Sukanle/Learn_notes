// D flip-flop
module Dflipflop (
    input clk,
    input [3:0] d,
    output reg [3:0] q  // `reg` like CPU register, can hold value over time
                        // In Verilog, `reg` does not necessarily imply a physical register
                        // Only `reg` can be a physical register in Sequential logic
);
    // Always block triggered on the rising edge of the clock
    // `posedge` is a keyword, that means we are triggering on the rising edge of the clock
    // Also, if we have `rising edge of clk`, we can also have `falling edge of clk`, that is
    // `negedge`, which is keyword in Verilog.
    // If `always` block has `posedge` or `negedge`, it is sequential logic
    always @(posedge clk) begin
        q <= d; // Non-blocking assignment to update q with d
                // Just like `SIMD` in CPU, a CPU tick updates all registers(SIMD) at the same time
                // Example:
                // ```cpp
                // __m128i a = _mm_set1_epi32(1); // a = [1, 1, 1, 1]
                // __m128i b = _mm_set1_epi32(2); // b = [2, 2, 2, 2]
                // __m128i c = _mm_add_epi32(a, b); // c = a + b = [3, 3, 3, 3]
                // ```
                // In this example, all elements of `c` will be updated at the same time, not like other `C/C++` arguments
                // where the order of evaluation matters.
    end
    // `always` is a keyword, that means we are defining a block of code that will be executed
    // Different `assign`, `always` blocks run in parallel or multiple statements
    // `assign` is for combinational logic, `always` is for sequential logic
    // Example of `always` and `assign` difference:
    //                    `always @(*) begin`
    // `assign a = b;` -> ` a = b;`
    // `assign c = d;` -> ` c = d;`
    //                    ` end`

    // `begin` and `end` are keywords, that means we are defining a block of code
    // Just like `{}` in `C/C++`
    // Also, If you only have one statement in the block, you can omit `begin` and `end`
    // Just like `if-else`, `for`, `while`, `case` statement in `C/C++`
endmodule

// Some rule of `always` block:
// Only use `<=` in `always` block, and this module is sequential logic
// Only use `=` in `always @(*)` block, and this module is combinational logic
// Only one operator is allowed in `always` block, either `=` or `<=`
// One signal can only be assigned in one `always` block or one `assign` statement
// In principle, one `always` block can only operate on one signal or a kind of signal
// `always` block only operate on `reg` type signal, should not operate on `wire` type signal,
// and should not be designated module