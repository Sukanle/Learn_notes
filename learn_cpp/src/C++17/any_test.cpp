#include <iostream>
#include <any>
#include <string>
#include <complex>
#include <functional>

// 自定义类
class AA
{
private:
	int m_a;
public:
	constexpr AA(int a) : m_a(a) {}
	static AA* create(int a)
	{
		return new AA(a);
	}
	void print()
	{
		std::cout << m_a << std::endl;
	}
	auto get_a() -> decltype(m_a)
	{
		return m_a;
	}
};

int main()
{
	// 整数类型
	auto a = std::any(5);
	printf("a is %s: %d\n",a.type().name(), std::any_cast<int>(a));
	// 浮点数类型
	a = std::any(5.5);
	printf("a is %s: %f\n",a.type().name(), std::any_cast<double>(a));
	// 自定义类型
	a = std::any(AA(5));
	printf("a is %s: %d\n",a.type().name(),std::any_cast<AA>(a).get_a());

	// 标准库类型
	auto b0 = std::make_any<std::string>("Hello, std::any!\n");
	auto b1 = std::make_any<std::complex<double>>(1.0, 2.3);

	printf("b0 is %s: %s",b0.type().name(), std::any_cast<std::string>(b0).c_str());
	printf("b1 is %s: %f + %f i\n",b1.type().name(), std::any_cast<std::complex<double>>(b1).real(), std::any_cast<std::complex<double>>(b1).imag());

	// Lambda表达式
	using lfun = std::function<void(void)>;

	std::any c0 = [](){printf("Lambda #1.\n");};
	printf("c0 is %s\n",c0.type().name());

	try
	{
		// 无法转换,抛出异常,异常原因：数据类型不匹配
		std::any_cast<lfun>(c0)();
	}
	catch(const std::bad_any_cast& e)
	{
		std::cout << e.what() << std::endl;
	}

	// 使用std::make_any创建Lambda表达式,成功
	auto c1 = std::make_any<lfun>([](){printf("Lambda #2.\n");});
	printf("c1 is %s\n",c1.type().name());
	// 调用Lambda表达式，不会抛出异常
	std::any_cast<lfun>(c1)();

	return 0;
}
