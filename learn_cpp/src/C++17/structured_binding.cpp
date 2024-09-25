#include <iostream>
#include <string>

struct man
{
    std::string name;
    int age;
};

man creative()
{
    return {"John", 25};
}

int main()
{
    auto [name, age] = creative();
    std::cout << "Name: " << name << std::endl;
    std::cout << "Age: " << age << std::endl;
    return 0;
}
