#include <iostream>
#include <type_traits>
#include <exception>

template <typename T>
void print(const T& t)
{
	// if constexpr编译器会根据条件表达式的结果来选择性的编译代码块，而不是在运行时选择性的执行代码块，这样可以提高程序的性能
	if constexpr (std::is_integral_v<T>)
		printf("This value is an integer: %d\n",t);
	else if constexpr (std::is_floating_point_v<T>)
		printf("This value is a floating point: %f\n",t);
	// 与C风格的条件编译不同，if constexpr可以在选择性编译的同时，使用C++的语法特性，如：异常处理，模板等
	else
		throw std::invalid_argument("This type is not supported");
	}

int main()
{
	try
	{
		print(5);
		print(5.5);
		// 不支持的类型，会抛出异常
		print("Hello");
	}
	catch(const std::exception& e)
	{
		std::cerr <<"Error:"<< e.what() << '\n';
	}
	return 0;
}
