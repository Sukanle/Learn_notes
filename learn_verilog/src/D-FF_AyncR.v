// async reset D flip-flop
module DflipflopAsyncReset(
    input clk,
    input rst, // asynchronous reset, just like `C/C++` bool flag
    input [3:0] d,
    output reg [3:0] q
);
    always @(posedge clk, posedge rst)
        if (rst) begin
            q <= 4'b0000; // reset output to 0
        end else begin
            q <= d; // update output with input
        end

    // In hardware works, `always` block is used to assign values to `reg` type variables.
    // That means, if you want to use `always` block to build a combinational logic,
    // you also need to use `reg` type signal which is not a physical register.
    // And you need to use `=` operator to assign value, `<=` is not allowed in combinational logic.
endmodule