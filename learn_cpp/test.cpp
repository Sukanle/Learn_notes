#include <execution>

// template <typename T>
// using std::is_execution_policy_v=std::is_execution_policy<T>::value;

int main() {
    if constexpr (std::is_execution_policy_v<std::execution::unsequenced_policy>) {
        puts("This is an execution policy");
    }
    else{
        puts("This is not an execution policy");
    }

    if constexpr (std::is_execution_policy_v<int>) {
        puts("This is an execution policy");
    }
    else{
        puts("This is not an execution policy");
    }
    return 0;
}
