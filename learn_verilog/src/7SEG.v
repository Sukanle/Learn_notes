// Seven Segment Display

// This 7SEG is configured for common cathode displays
// High on a segment lights it up
module SevenSEG (
    input [4:0] data,
    input dp, // decimal point
    output reg [7:0] seg
);
    // `case` statement to map 4-bit input to 7-segment output
    // Just like `switch` in other languages(C, Java, etc)
    // In combinational logic, we use `always @(*)` block, and should use `default` case to avoid latches
    // To use `default` case is avoid to prevent the integrated circuit of the latch
    always @(*) begin
        case(data)
            4'd0: seg = 8'b00111111; // 0
            4'd1: seg = 8'b00000110; // 1
            4'd2: seg = 8'b01011011; // 2
            4'd3: seg = 8'b01001111; // 3
            4'd4: seg = 8'b01100110; // 4
            4'd5: seg = 8'b01101101; // 5
            4'd6: seg = 8'b01111101; // 6
            4'd7: seg = 8'b00000111; // 7
            4'd8: seg = 8'b01111111; // 8
            4'd9: seg = 8'b01101111; // 9
            default: seg = 8'b00000000; // Off
        endcase
        if(dp)
            seg[7] = 1'b1; // Turn on decimal point
    end
endmodule