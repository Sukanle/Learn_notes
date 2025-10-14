// full adder example
// {count, over} = a + b + cin
// `{}` is a concatenation operator, it concatenates multiple signals into a single signal
// That like `C++ 17 structured binding`:
// using FullCount = int;
// using OverFlow = bool;
// std::pair<FullCount, OverFlow> func();
// auto [x, y] = func(); 

// In this part, we will learn how to define `wire`
// `wire` is a type of signal, it is used to connect different modules
// `wire` can be used in `assign` statements and module instantiations
// `wire` cannot be used in `always` blocks
// `wire` is a combinational signal, it means the value of the signal is determined
module full_adder (
    input a,
    input b,
    input cin,
    output count,
    output over
);
    wire p,g;

    assign p = a ^ b; // propagate
    assign g = a & b; // generate
    assign count = g | (p ^ cin); // sum
    assign over = p ^ cin; // carry out
endmodule