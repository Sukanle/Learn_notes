// FSM 
module drivideby3FSM (
    input clk,
    input rst,
    output out_bit
);
parameter S0 = 2'b00;
parameter S1 = 2'b01;
parameter S2 = 2'b10;

reg [1:0] current_state, next_state;

// State register
always @(posedge clk, posedge rst) begin
    if (rst) current_state <= S0;
    else current_state <= next_state;
end

always @(*) begin
    case(current_state)
        S0: next_state = S1;
        S1: next_state = S2;
        S2: next_state = S0;
        default: next_state = S0;
    endcase
end

// Output logic
assign out_bit = (current_state == S0);
endmodule