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
// If you not set the value type, Verilog will treat it as `wire` by default
// A wire is basically just a piece of wire with no extra logic—it simply passes the input signal straight to the output
// This type of data is used to represent combinational logic signals assigned in assign statements
module fullAdder (
    input a,
    input b,
    input cin,
    output sum,
    output cout
);
    // wire default initial value is 'z' (high impedance)
    wire p,g;
    // Verilog has 4 logic values: 0, 1, x, z
    // `0` means logic low
    // `1` means logic high
    // `x` means unknown, it means the signal is driven by multiple sources with different values
    // `z` means high impedance, it means the signal is not driven by any source, it is like a power cut (extreme case of `0v`)
    // Attention: 
    // In fact, `z` is nearly `0v`, `0` just work in a range like `0~0.3VCC`, `1` work in a range like `0.7VCC~VCC`
    // That means, logic low(`0`) is a range that poeple say it's `low power`, not absolute `0v`
    // Aslo, logic high(`1`) is a range that poeple say it's `high power`, not absolute `5v`
    // In real world, People like to say `0~0.3VCC` is logic low, `0.7VCC~VCC` is logic high, `0.3VCC~0.7VCC` is undefined
    // So, People need a value can represent `0v`(input resistance is very high, it is ideal 0v), that is `z`

    assign p = a ^ b; // propagate
    assign g = a & b; // generate
    assign sum = g | (p ^ cin); // sum
    assign cout = p ^ cin; // carry out
endmodule