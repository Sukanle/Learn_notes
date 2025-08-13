#include <iostream>
#include <string>
#include <utility>

namespace identifier {
struct MyTypeA {
    // int value;
    // bool operator==(const MyType& other){
    //     return value==other.value;
    // }
};
struct MyTypeB {};
void swap(MyTypeA& A1, MyTypeA& A2) noexcept {
    std::cout << "Custom Swap Function\n";
}
}   // namespace identifier
template<typename T, typename F> void do_something(T& a, F& b) {
    // If you don't have `using std::swap;`, this code will never match
    // `std::swap` because type "A" or "B" is not namespace "std" memmber.
    // In this way, it called a "Customization Point" by C++ ISO.
    using std::swap;
    swap(a, b);
}
template<typename T, typename F> void do_something_cpp20(T& a, F& b) {
    // In C++20, The `Ranges` lib will bring some special objects, which ISO
    // calls them "Customization Point Objects", or "CPO" for short.
    // This is not a function, this is a `Callable Objects`.
    std::ranges::swap(a, b);
    // Why did C++ ISO introduce 'CPO'? This is to avoid the selection of
    // user-defined overloaded functions (template specialization) in the
    // 'Argument Dependent Lookup', thus skipping the type detection (SFINAE)
    // and constraints of the paradigm function.
}
int main() {
    identifier::MyTypeA a1;
    identifier::MyTypeA a2;
    identifier::MyTypeB b1;
    identifier::MyTypeB b2;

    do_something(a1, a2);
    do_something(b1, b2);

    std::string str1{"str1"};
    std::string str2{"str2"};
    do_something(str1, str2);
}
