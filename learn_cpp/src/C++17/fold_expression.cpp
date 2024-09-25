#if __cplusplus < 201703L
#error "C++17 is required to compile this file, please use -std=c++17"
#else
#include <iostream>
#include <string>

template <typename... Args>
auto str_con(Args&& ...args) -> std::string
{
    return (... + static_cast<std::string>(args));
}

int main()
{
    printf("%s\n",str_con("Hello ","World","!").c_str());
    printf("%s\n",str_con("Man, ","I ","love ","C++","!").c_str());

    return 0;
}
#endif
