#include <iostream>
#include <type_traits>
#include <string>
#include <memory>

template <typename T,typename T1>
bool numcmp(T a,T1 b)
{
    return a>b;
}

int getValue()
{
    char buf[20];
    scanf("%s",buf);
    std::string str(buf);

    if (str.find_first_not_of("0123456789") == std::string::npos) {
        return 1; // 全为数字
    }

    bool hasDigit = false;
    for (char ch : str) {
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            hasDigit = true;
            break;
        }
    }

    if (hasDigit) {
        return 0; // 包含数字但不是全为数字
    } else {
        return -1; // 不包含数字
    }
}

int main()
{
    if(int a=static_cast<int>(numcmp(6,1.2));a>0){
        printf("a>0\n");
        int b;

        switch(int i=getValue();i){
            case 1:
                printf("输入是全数字\n");
                break;
            case 0:
                printf("输入包含数字\n");
                break;
            default:
                printf("输入不包含数字\n");
                break;
        }
    }
    return 0;
}
