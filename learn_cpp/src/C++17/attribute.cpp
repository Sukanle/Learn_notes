#include <iostream>

using IntFn_2int = int(int,int);
using VoidFn_int = void(int);

[[nodiscard]] IntFn_2int add;
VoidFn_int processNumber;

int main()
{
    int num=add(1, 2);
    add(1, 2);
    printf("num=%d\n", num);
    printf("add(1, 2)=%d\n", add(1, 2));

    printf("\n");

    [[maybe_unused]] int num2;
    int a;
    printf("Please input a number: ");
    scanf("%d", &num2);

    processNumber(num2);

    return 0;
}

void processNumber(int num) {
    switch (num) {
        case 1:
            std::cout << "Case 1" << std::endl;
            [[fallthrough]]; // 显式地表示贯穿到下一个 case
        case 2:
            std::cout << "Case 2" << std::endl;
            break;
        default:
            std::cout << "Default case" << std::endl;
            break;
    }
}

[[nodiscard]] int add(int a, int b)
{
    return a + b;
}
