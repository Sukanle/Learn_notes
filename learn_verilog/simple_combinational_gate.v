// Simple combinational logic gate example
// y = (!a)(!b)(!c) + a(!b)(!c) + a(!b)c

// `module` is a keyword, that means we are defining a module
// `module` such as `class` in C++, it comprises of data members and member functions
// `members` -> `inputs`, `outputs`, `wires`, `registers`,  `inouts`(`input` + `output`ss)
// `functions` -> `always`, `assign` statements
module example1 ( 
    // `input` and `output` are keywords, that means we are defining inputs and outputs
    // In this particular module, we have 3 inputs and 1 output
    input a,
    input b,
    input c,
    output y
);
    // `assign` is a keyword, that means we are assigning a value to a wire
    // The expression on the right side of the `assign` statement is evaluated and assigned to the
    // wire on the left side of the `assign` statement
    assign y = (~a & ~b & ~c) | (a & ~b & ~c) | (a & ~b & c);

    // Common operators and their Verilog priorities
    // From highest to lowest precedence:
    // 0. () (parentheses)
    // 1. ~ (NOT)
    // 2. *, /, % (mult, div, mod)
    // 3. +, - (add, sub)
    // 4. <<, >> (shift)
    // 5. <, <=, >, >= (relational)
    // 6. ==, != (equality)
    // 7. &, ~& (AND, NAND)
    // 8. ^, ~^ (XOR, XNOR)
    // 9. |, ~| (OR, NOR)
    // 10. ?: (ternary)
endmodule