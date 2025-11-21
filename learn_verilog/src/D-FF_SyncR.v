// sync reset D flip-flop
module DflipflopSyncReset(
    input clk,
    input rst, // synchronous reset, just like `C/C++` bool flag
    input [3:0] d,
    output reg [3:0] q
);
    always @(posedge clk) begin
        // `if-else` just like `C/C++` if-else
        // Difference is that in Verilog, `if-else` has priority over other statements
        // So, if we have multiple `if-else` statements, the first one has the highest priority,
        // and you need to make sure that the first statement is the often selected

        // Here, we give `rst` the highest priority, causing the flip-flop to reset first when `rst` is high
        if (rst) begin
            q <= 4'b0000; // reset output to 0
        end
        else begin
            q <= d; // update output with input
        end
        // Attention:
        // Reset operation will use one clock cycle, and it will be empty immediately after reset
    end
endmodule