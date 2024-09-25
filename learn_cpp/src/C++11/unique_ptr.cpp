#include <iostream>
#include <memory>
#include <string.h>

int main()
{
    std::unique_ptr<char[]> str(new char[100+1]);
    strcpy(str.get(), "Hello, World!");
    printf("%s\n", str.get());
    return 0;
}
