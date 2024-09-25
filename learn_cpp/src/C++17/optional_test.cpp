#include <iostream>
#include <optional>

std::optional<int> divide(int a, int b)
{
	if(b==0)
		return std::nullopt;
	return a/b;
}

int main()
{
	auto result=divide(10,2);
	if(result.has_value())
		printf("Result: %d\n", result.value());
	else
		printf("Result: Division by zero\n");

	auto result1=divide(10,0);
	if(result1.has_value())
		printf("Result: %d\n", result1.value());
	else
		printf("Result: Division by zero\n");

	return 0;
}
