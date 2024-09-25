# `C++ atomic`原子操作

## 1. 原子操作的基本概念
* 原子操作，重要在于原子性，即对于cpu来说，原子操作是不可分割的，要么全部执行，要么全部不执行。
  > 所有的高级语言为了方便开发，更贴近于自然语言而诞生的，但这样会导致将cpu的多个指令封装成一个高级语言的语句，这样就会出现多个cpu指令的原子性问题。<br>
  > 例如`i++`，实际上是三个cpu指令，读取i的值，i的值加1，将i的值写回内存，这三个指令组成的`i++`不是原子的，在多线程操作的时候，可能会被打断，导致数据不一致，比如在`i++`的时候，线程1读取了i的值，线程2读取了i的值，线程1加1，线程2加1，线程1写回i的值，线程2写回i的值，这样i的值本来应该加2，但是实际上只加了1，导致数据不一致。
* 在`C++11`前，`C++`没有提供语言级别的多线程支持，与传统的`C`语言一样，需要依赖于操作系统提供的线程库，例如`pthread`，`windows`线程等，这些线程库提供了一些原子操作的接口，例如`pthread_mutex_lock`，`pthread_mutex_unlock`等，但是这些接口使用起来比较繁琐，容易出错，而且不同的操作系统提供的接口不一样，不方便移植。
* 在`C++11`后，`C++`引入了语言级别的多线程支持，用于解决跨平台多线程的移植，提供了`std::thread`，`std::mutex`等多线程库，同时也提供了`std::atomic`模板类，用于支持原子操作，`std::atomic`模板类提供了一系列的原子操作接口，例如`load`，`store`，`exchange`，`compare_exchange`等，这些接口是原子的，不会被打断，保证了多线程操作的数据一致性。
* 原子操作相较于传统的锁机制，具有更高的性能，因为原子操作是硬件级别的支持，不需要进入内核态，不需要上下文切换，不需要线程调度，而锁机制需要进入内核态，需要上下文切换，需要线程调度，性能开销较大；而且不会出现死锁，阻塞等问题。

## 2. 原子操作的使用
* `std::atomic`模板类的使用非常简单，只需要包含头文件`<atomic>`，然后定义一个`std::atomic`对象，就可以使用原子操作接口。
* `std::atomic`模板类提供了一系列的原子操作接口，这些接口是原子的，不会被打断，保证了多线程操作的数据一致性，下面是一些常用的原子操作接口：
  * 通用接口：
    : `load`：读取原子对象的值
    : `store`：设置原子对象的值
    : `exchange`：交换原子对象的值
    : `is_lock_free`：判断原子对象是否是无锁的
    : `compare_exchange`：比较并交换原子对象的值
      > `compare_exchange_weak`：弱版本，可能会失败，常用于自旋锁和无锁队列，性能更好<br>
      > `compare_exchange_strong`：强版本，不会失败，常用于互斥锁和条件变量，即不需要多次重试，性能较差<br>
      >> 接收两个参数，第一个参数是期望值，第二个参数是设定值，如果原子对象的值等于期望值，则将原子对象的值设定为设定值，返回`true`，否则返回`false`，并将原子对象的值设定为期望值，即如果使用`while`循环，可以保证一定设置为设定值<br>
    > 下面是 *C20*新增的接口：<br>
    > `wait`：等待原子对象的值直至发生改变时返回，会挂起当前线程，会进行上下文切换，适用于长时间等待的场景<br>
    > `notify_one`：通知至少一个等待的线程<br>
    > `notify_all`：通知所有等待的线程
  * 整数、浮点数 *(C++20)*、指针接口：
    : `fetch_add`：原子的加法操作
    : `fetch_sub`：原子的减法操作
    : `+=`：原子的加法赋值操作
    : `-=`：原子的减法赋值操作
  * 整数、指针接口：
    : `++`：原子的自增操作
    : `--`：原子的自减操作
  * 整数接口：
    : `fetch_and`：原子的与操作
    : `fetch_or`：原子的或操作
    : `fetch_xor`：原子的异或操作
    : `&=`：原子的与赋值操作
    : `|=`：原子的或赋值操作
    : `^=`：原子的异或赋值操作

## 3. 内存顺序
* 由于编译器的优化，cpu的乱序执行，多核cpu的缓存一致性等原因，导致多线程操作的数据一致性问题，为了解决这个问题，`C++11`引入了内存顺序，用于规定多线程操作的数据一致性[^1:MESI]。
  > 编译器优化：编译器会对代码进行优化，例如指令重排，内联等，提升代码的执行效率，但是会导致多线程操作的数据一致性问题<br>
  > cpu乱序执行：cpu会对指令进行乱序执行，本意是为了提高cpu的执行效率，减少cpu的等待时间，但是会导致多线程操作的数据一致性问题<br>
  > 多核cpu的缓存一致性：多核cpu会有多个缓存，每个核心的缓存是独立的，当一个核心修改了缓存中的数据，其他核心的缓存中的数据就会失效，这时候就需要进行缓存一致性，这个过程会导致多线程操作的数据一致性问题
* 内存顺序是通过`std::memory_order`枚举类型来指定的，`std::memory_order`枚举类型有以下几种：
  : `std::memory_order_relaxed`：松散顺序，不保证任何顺序，只保证原子操作的原子性,为原子操作的缺省值
  : `std::memory_order_consume`：消费顺序，保证读取操作的顺序，但不保证写入操作的顺序
  : `std::memory_order_acquire`：获取顺序，保证读取操作的顺序，同时保证写入操作的顺序
  : `std::memory_order_release`：释放顺序，保证写入操作的顺序，同时保证读取操作的顺序
  : `std::memory_order_acq_rel`：获取释放顺序，同时保证读取操作和写入操作的顺序
  : `std::memory_order_seq_cst`：顺序一致性，保证所有操作的顺序
* 通过`std::memory_order`枚举类型作为参数，可以组合出4种常见的同步模型：
  1. *Relaxed ordering*：非同步模型，不保证任何顺序，只保证原子性，适用于纯计算的场景
  2. *Acquire-release ordering*：半同步模型，会同时保证读写操作的数据同步，即会强制要求前后的操作不会被重排序
  > 例如下面的代码：<br>
  > ```cpp
  > bool x = false;
  > atomic<bool> y=false;
  >
  > // 线程1
  > x = true;// A
  > y.store(true, memory_order_release);// B
  >
  > // 线程2
  > while(!y.load(memory_order_acquire));// C
  > assert(x);// D
  > ```
  > 在上面的代码中，线程1中的`A`和`B`之间的顺序是有关系的，`A`和`B`之间的顺序是有关系的，需要保证`A`在`B`之前执行，所以此时可以使用`memory_order_release`，因为`memory_order_release`会保证`A`和`B`之间的顺序，而`memory_order_acquire`会保证`C`和`D`之间的顺序。<br>
  > 但是`Acquire-release ordering`模型有这明显的副作用：线程1中发生在`B`之前的所有操作都会被强制要求在之前执行，线程2则是`C`操作之后的所有操作都会被强制要求在这之后执行,这样就导致与其他线程不会产生数据竞争的操作被强制执行，而且会导致编译器无法对其进行优化，这时候就是`Consume-release ordering`模型的使用场合了。
  3. *Consume-release ordering*：半同步模型，只保证写操作的数据同步，即只会要求依赖关系的操作不会被重排序
  > 例如下面的代码：<br>
  > ```cpp
  > bool x = false;
  > atomic<bool> g=false;
  >
  > // 线程1
  > x = true;// A
  > g.store(true, memory_order_release);// B
  >
  > // 线程2
  > while(!g.load(memory_order_acquire));// C
  > assert(x);// D
  > ```
  > 在上面的代码中，线程1中的`A`和`B`之间的顺序是没有关系的，`A`和`B`之间的顺序是没有关系的，`C`和`D`之间的顺序也是没有关系的，所以此时可以使用`memory_order_relaxed`，因为`memory_order_relaxed`不会保证`A`和`B`之间的顺序，也不会保证`C`和`D`之间的顺序。<br>
  > 但如果将`g.store(true,memory_order_release)`改为`g.store(x,memory_order_release)`，则需要保证`A`和`B`之间的顺序，因为`B`依赖于`A`，而这时`memory_order_release`就会保证`A`和`B`之间的顺序。<br>
  > `memory_order_consume`与`memory_order_relaxed`的区别在`C`和`D`之间的顺序，`memory_order_consume`会保证`C`和`D`之间的顺序，而`memory_order_relaxed`不会保证`C`和`D`之间的顺序。
  4. *Sequentially consistent ordering*：全同步模型，保证所有操作的顺序，`atomic`的默认内存顺序，最强的保证，但是性能开销最大（特别是arm等`weak order`平台），适用于需要强一致性的场景
  > 例如下面的代码：<br>
  > ```cpp
  > atomic<int> x(0), y(0);
  > int r1=0, r2=0;
  > 
  > void thread1()
  > {
  >     x.store(1, memory_order_seq_cst);// A
  >     r1 = y.load(memory_order_seq_cst);// B
  > }
  >
  > void thread2()
  > {
  >     y.store(1, memory_order_seq_cst);// C
  >     r2 = x.load(memory_order_seq_cst);// D
  > }
  > ```
  > `Sequentially consistent ordering`与`Acquire-release ordering`的区别在于`A`和`B`之间的顺序，`C`和`D`之间的顺序，`Sequentially consistent ordering`会保证`A`和`B`之间的顺序，`C`和`D`之间的顺序，而`Acquire-release ordering`只会保证`A`和`B`之间的顺序，`C`和`D`之间的顺序，即`Sequentially consistent ordering`会保证所有线程对共享变量的读写操作按照全局顺序执行。
  > 简而言之，`Sequentially consistent ordering`就是程序在多核心上执行时就像是在单核心上执行一样，并且原先单核心上的指令执行顺序保持不变。

[^1:MESI]:`MESI`协议是一种缓存一致性协议，用于保证多核cpu的缓存一致性，`MESI`协议有四种状态：<br>
    `M`（修改）用来告诉其他cpu已经修改完成；<br>
    `E`（独占）用来告诉其他cpu不能读取数据；<br>
    `S`（共享）用来告诉其他cpu可以读取数据；<br>
    `I`（失效）用来告诉其他cpu数据已经失效；<br>
    `MESI`协议通过这四种状态来保证多核cpu的缓存一致性，保证多核cpu的缓存中的数据一致。
