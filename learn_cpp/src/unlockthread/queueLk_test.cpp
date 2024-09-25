#include "queueLK.h"

class MyClass
{
public:
	MyClass():_data(0) {

	}

	MyClass(int data):_data(data){

	}

	MyClass(const MyClass& mc) {
		//std::cout << "called MyClass const MyClass& version " << std::endl;
		_data = mc._data;
	}

	MyClass(MyClass&& mc) noexcept {
		//std::cout << "called MyClass  MyClass&& version " << std::endl;
		_data = std::move(mc._data);
	}

	friend std::ostream &operator << (std::ostream & os, const MyClass& my) {
		os << "MyClass Data is " << my._data ;
		return os;
	}

	MyClass& operator = (const MyClass& other) {
		_data = other._data;
		return *this;
	}

private:
	int _data;
};

void TestCircularQue()
{
    //最大容量为10
    CircularQueueLK<MyClass, 5> cq_lk;
    MyClass mc1(1);
    MyClass mc2(2);
    cq_lk.push(mc1);
    cq_lk.push(std::move(mc2));
    for (int i = 3; i <= 5; i++) {
        MyClass mc(i);
        auto res = cq_lk.push(mc);
        if (res == false) {
            break;
        }
    }

    cq_lk.push(mc2);

    for (int i = 0; i < 5; i++) {
        MyClass mc1;
        auto res = cq_lk.pop(mc1);
        if (!res) {
            break;
        }
        std::cout << "pop success, " << mc1 << std::endl;
    }

    auto res = cq_lk.pop(mc1);
}

void TestCircularQueSeq()
{
	CircularQueueSeq<MyClass, 3> cq_seq;
	for(int i = 0; i < 4; i++)
	{
		MyClass mc1(i);
		auto res = cq_seq.push(mc1);
		if(!res)
		{
			break;
		}
	}

	for(int i = 0; i < 4; i++)
	{
		MyClass mc1;
		auto res = cq_seq.pop(mc1);
		if(!res)
		{
			break;
		}

		std::cout << "pop success, " << mc1 << std::endl;
	}

	for (int i = 0; i < 4; i++)
	{
		MyClass mc1(i);
		auto res = cq_seq.push(mc1);
		if (!res)
		{
			break;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		MyClass mc1;
		auto res = cq_seq.pop(mc1);
		if (!res)
		{
			break;
		}

		std::cout << "pop success, " << mc1 << std::endl;
	}
}

int main()
{
	TestCircularQue();
	printf("\n\n\n");
	TestCircularQueSeq();
	return 0;
}
