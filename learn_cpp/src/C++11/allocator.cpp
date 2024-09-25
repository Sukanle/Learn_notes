#include <vector>
#include <iostream>
#include <memory>
#include <cstddef>
#include <stdlib.h>
#include <unistd.h>
#include <cinttypes>

template <typename type>
class MyAllocator
{
public:
	MyAllocator() = default;
	~MyAllocator() = default;

	// 类型别名
	using value_type = type;
	using pointer = type*;
	using const_pointer = const type*;

	using void_pointer = void*;
	using const_void_pointer = const void*;

	using size_type = size_t;
	using difference_type = std::ptrdiff_t;

	// 重新绑定函数类
	template<typename U>
	struct rebind{using other = MyAllocator<U>;};

	// 分配内存
	pointer allocate(size_type numObjs)
	{
		allocCount += numObjs;
		// 测试
		printf("MyAllocator::allocate，分配内存：%d\n", static_cast<int>(numObjs));
		return static_cast<pointer>(::operator new(sizeof(value_type)*numObjs));
	}
	// 分配内存
	pointer allocate(size_type numObjs, const_void_pointer hit)
	{
		return allocate(numObjs);
	}
	// 释放内存
	void deallocate(pointer p, size_type numObjs)
	{
		// 测试
		printf("MyAllocator::deallocate，释放内存：%d\n", static_cast<int>(numObjs));
		allocCount -= numObjs;
		::operator delete(p);
	}

	// 调用模板参数类型的构造函数
	template<typename U, typename... Args>
	void construct(U* p, Args&&... args)
	{
		::new(p) U(std::forward<Args>(args)...);
	}
	// 调用模板参数类型的析构函数
	template<typename U, typename... Args>
	void destroy(U* p)
	{
		p->~U();
	}

	size_type get_allocations() const
	{
		return allocCount;
	}
private:
	// 统计当前分配的内存, 用于测试
	size_type allocCount;
};

int main()
{
	std::vector<int,MyAllocator<int>> vec(0);

	for(size_t i = 0; i < 30; i++)
	{
		sleep(1);
		vec.push_back(i);
		printf("当前分配内存：%d\n", static_cast<int>(vec.get_allocator().get_allocations()));
	}

	return 0;
}
