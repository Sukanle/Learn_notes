#ifndef QUEUELK_H
#define QUEUELK_H 1

#include <iostream>
#include <mutex>
#include <memory>
#include <atomic>

template <typename T,size_t Cap>
class CircularQueueLK : private std::allocator<T>
{
private:
	size_t m_max_size;
	T* m_data;
	std::mutex m_mutex;
	size_t m_head=0;
	size_t m_tail=0;
public:
	CircularQueueLK():m_max_size(Cap+1),m_data(std::allocator<T>::allocate(m_max_size)),m_head(0),m_tail(0){}
	CircularQueueLK(const CircularQueueLK&)=delete;
	CircularQueueLK& operator=(const CircularQueueLK&)=delete;
	CircularQueueLK& operator=(const CircularQueueLK&) volatile = delete;

	~CircularQueueLK()
	{
		// 循环销毁队列中的元素
		std::lock_guard<std::mutex> mtx_lock(m_mutex);
		// 调用内部的析构函数
		while(m_head!=m_tail)
		{
			std::allocator<T>::destroy(m_data+m_head);
			m_head=(m_head+1)%m_max_size;
		}
		std::allocator<T>::deallocate(m_data,m_max_size);
	}

	template <typename ...Args>
	bool emplace(Args&& ...arg)
	{
		// 保护共享数据
		std::lock_guard<std::mutex> mtx_lock(m_mutex);
		if((m_tail+1)%m_max_size==m_head)
		{
			printf("circular queue is full!\n");
			return false;
		}

		std::allocator<T>::construct(m_data+m_tail,std::forward<Args>(arg)...);

		m_tail=(m_tail+1)%m_max_size;
		return true;
	}

	bool push(const T& value)
	{
		printf("called push T& version.\n");
		return emplace(value);
	}

	bool push(const T&& value)
	{
		printf("called push T&& version.\n");
		return emplace(std::move(value));
	}

	bool pop(T &val)
	{
		std::lock_guard<std::mutex> mtx_lock(m_mutex);

		if(m_head==m_tail)
		{
			printf("circular queue is empty!\n");
			return false;
		}

		val=std::move(m_data[m_head]);

		m_head=(m_head+1)%m_max_size;
		return true;
	}
};

template <typename T,size_t Cap>
class CircularQueueSeq : private std::allocator<T>
{
private:
	size_t m_max_size;
	T* m_data;
	std::atomic<bool> m_atomic_using;
	size_t m_head=0;
	size_t m_tail=0;

	inline void compare_exchange_ft()
	{
		bool use_expected=false;
		bool use_desired=true;
		do{
			use_expected=false;
			use_desired=true;
		}while(!m_atomic_using.compare_exchange_strong(use_expected,use_desired));
	}
	inline void compare_exchange_tf()
	{
		bool use_expected=true;
		bool use_desired=false;
		do{
			use_expected=true;
			use_desired=false;
		}while(!m_atomic_using.compare_exchange_strong(use_expected,use_desired));
	}

public:
	CircularQueueSeq():m_max_size(Cap+1),m_data(std::allocator<T>::allocate(m_max_size)),m_head(0),m_tail(0),m_atomic_using(false){}
	CircularQueueSeq(const CircularQueueSeq&)=delete;
	CircularQueueSeq& operator=(const CircularQueueSeq&) =delete;
	CircularQueueSeq& operator=(const CircularQueueSeq&) volatile =delete;

	~CircularQueueSeq()
	{
		while(m_head!=m_tail)
		{
			std::allocator<T>::destroy(m_data+m_head);
			m_head=(m_head+1)%m_max_size;
		}

		std::allocator<T>::deallocate(m_data,m_max_size);
	}

	template<typename ...Args>
	bool emplace(Args &&...arg)
	{

		compare_exchange_ft();

		if((m_tail+1)%m_max_size==m_head)
		{
			printf("circular queue is full!\n");
			compare_exchange_tf();
			return false;
		}

		std::allocator<T>::construct(m_data+m_tail,std::forward<Args>(arg)...);

		m_tail=(m_tail+1)%m_max_size;

		compare_exchange_tf();
		return true;
	}

	bool push(const T& value)
	{
		printf("called push T& version.\n");
		return emplace(value);
	}
	bool push(T &&value)
	{
		printf("called push T&& version.\n");
		return emplace(std::move(value));
	}

	bool pop(T &val)
	{
		compare_exchange_ft();

		if(m_head==m_tail)
		{
			printf("circular queue is empty!\n");
			compare_exchange_tf();
			return false;
		}

		val=std::move(m_data[m_head]);

		m_head=(m_head+1)%m_max_size;

		compare_exchange_tf();

		return true;
	}
};

#endif
