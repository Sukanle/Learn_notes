#include <iostream>
#include <functional>

#if __cplusplus < 201703L
#error "C++17 support required, because noexcept become part of the type system only in C++17 and later"
#else

using VoidFn_v_n = void()noexcept;

VoidFn_v_n foo;

int main()
{
    auto p = foo;
    p();
    return 0;
}


void foo()noexcept {
    std::cout << "foo() noexcept\n";
    return;
}
#endif
