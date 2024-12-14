#任务域
## `task_arena`任务域
* `task_arena`任务域是一个线程池，用于执行任务。
* 通常使用它的默认构造函数创建一个`task_arena`对象，该对象使用默认的线程数。
* 常用的方法有`execute`和`enqueue`。
  1. `execute`方法用于执行一个任务，参数是一个函数对象，为阻塞调用。
  2. `enqueue`方法用于将一个任务放入任务队列，参数是一个函数对象，为非阻塞调用。
### 嵌套`for`循环
* `parallel_for`可以嵌套使用，特别是在任务数量远小于线程数量的情况下，可以提高并行效率，但是可能会导致死锁[^1]，因此需要注意。
* 为了提升性能且避免死锁，`TBB`官方提供了以下的解决方案：
1. 使用标准库里的递归锁`std::recursive_mutex`，但是这个锁的性能较差。
2. 创建另外一个任务域，用于执行内部的`parallel_for`，这样可以避免死锁，但是会增加线程间的通信开销。
3. 同一个任务域，但使用`isolate`方法隔离，禁止其内部的工作被偷取，效率有所提升，但是仍然会有一定的开销。(推荐)
  > `isolate`的使用方法：
  > ```cpp
  > tbb::this_task_arena::isolate(函数对象);
  > ```

#任务分配器
* 如何将任务均匀分配给每个线程，避免线程空闲浪费时间，是一个很重要的问题。
>因为理论上大多数情况下，每个任务的难度是不一样的，有的任务可能会比较耗时，有的任务可能会比较简单，如果不合理的分配任务，可能会导致某个线程一直在执行耗时任务，而其他线程一直在等待，这样就会导致线程空闲浪费时间。
* 常见的方案有下面几种：
  1. 线程数量超过`CPU`物理核心数，让操作系统调度保证各个核心始终饱和(默认)，但是线程过多会导致调度的`overhead`。
  > 因为线程数超过物理核心数，操作系统会在同时执行多个线程时（即并行执行）时，再执行并发策略调用，利用`CPU`的时间片轮转机制，让每个线程都有机会执行，从而保证各个核心始终饱和。
  2. 线程数量不变，但是用一个队列分发和认领任务，又名`thread pool`技术。
  > 通过一个队列，将任务分发给每个线程，线程执行完任务后，再从队列中认领任务，这样可以避免线程空闲浪费时间。但共享扔完队列只能在一定程度上的消减任务量的不均匀，但是无法完全解决。比如没有及时添加任务到队列，或者任务量过大，都会导致线程空闲浪费时间。
  3. `TBB`的`work-stealing`算法，即每个线程一个任务队列，做完本职工作后可以认领其他线程的任务。
  > 实际上只是将线程池的队列分成了多个队列（进一步细分任务），每个线程一个队列，线程执行完本职工作后，可以从其他线程的队列中偷取任务，这样可以解决线程池的队列分发和认领任务的问题，避免线程空闲浪费时间。但是这种方式也有缺点，就是可能会导致锁的反复上锁导致死锁。
  4. 随机分配（通过`hash`函数或者线性函数），避免线程空闲浪费时间。
  > 通过利用正态分布数量越大，方差越小的特性，这样分配后的每个线程的总工作量是相近的，而且还避免了维护任务队列的开销，如线程安全等问题。但常见的问题是，如果任务量过大，也会导致线程切换的开销，因此需要权衡。这种方法更常见于`GPU`的任务分配（网格跨步循环`grid-stride loop`）。
## `tbb::static_partitioner`静态分区
* `tbb::static_partitioner`会根据区间的大小（元素的个数），将任务均匀分配给每个线程的任务队列。
* `tbb::static_partitioner`的使用方法：
  > ```cpp
  > tbb::parallel_for(tbb::blocked_range<int>(0, n), 函数对象, tbb::static_partitioner());
  > ```
* 还可以指定区间的粒度：
  > ```cpp
  > tbb::parallel_for(tbb::blocked_range<int>(0, n, 1000), 函数对象, tbb::static_partitioner());
  > ```
* 不适用于循环体分布不均匀的情况。
## `tbb::simple_partitioner`单一分区
* `tbb::simple_partitioner`则会将所有任务放入一个任务队列，然后每个线程从队列中取任务执行。
* `tbb::simple_partitioner`的使用方法：
  > ```cpp
  > tbb::parallel_for(tbb::blocked_range<int>(0, n), 函数对象, tbb::simple_partitioner());
  > ```
* 指定区间的粒度：
  > ```cpp
  > tbb::parallel_for(tbb::blocked_range<int>(0, n, 1000), 函数对象, tbb::simple_partitioner());
  > ```
* 适用于循环体分布不均匀的情况，或者矩阵乘法等任务。（会进行缓存访问优化）[^2]
## `tbb::auto_partitioner`自动分区
* `tbb::auto_partitioner`会根据任务的数量和线程的数量，自动根据填入的函数对象选择合适分配方法。
* `tbb::auto_partitioner`的使用方法：
  > ```cpp
  > tbb::parallel_for(tbb::blocked_range<int>(0, n), 函数对象, tbb::auto_partitioner());
  > ```
## `tbb::affinity_partitioner`学习性分区
* `tbb::affinity_partitioner`记录历史，下次执行时，会根据历史记录，将任务分配自动负载均衡。
* `tbb::affinity_partitioner`的使用方法：
  > ```cpp
  > tb::affinity_partitioner ap;
  > tbb::parallel_for(tbb::blocked_range<int>(0, n), 函数对象, ap);
  > ```
* 不能直接使用，需要先创建一个`affinity_partitioner`对象，然后传入`parallel_for`函数。
[^1]:`TBB`使用了`work-stealing`算法，当一个线程的任务执行完毕后，会从其他线程的任务队列中偷取任务，避免线程空闲浪费时间，但如果是使用了嵌套`parallel_for`，内部的线程可能在执行完后，会去偷取外部线程的任务，导致`mutex`重复上锁。
[^2]:`simple_partitioner`会根据指定的粒度设置，将矩阵进行分块，内部按照常规双重循环访问，块外则会按照`Z`形式访问，这样可以提高缓存的命中率，从而提高性能，类似于矩阵乘法的`cache blocking`技术。
