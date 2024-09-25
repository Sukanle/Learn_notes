#include <iostream>
#include <string>

struct man
{
    std::string name;
    int age;
    double height;
};
int main()
{
    man kb = {"kb",25,68.2};
    printf("Name: %s, Age: %d, Height: %.2f\n", kb.name.c_str(), kb.age, kb.height);
    return 0;
}
