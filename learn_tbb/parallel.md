#无损并行
* 下面的并行模式是无损的，即在不改变工作量的情况下，将时间缩短。
##映射并行*`parallel_for`*
* `parallel_for`是一个用于没有数据依赖的并行循环的模板函数。
* `parallel_for`有多个重载版本，其中最常用的版本为：
  1. `void parallel_for(const Range& range, const CallBakeFn& fn)`
    * `range`：循环的范围。
    > 类型为`tbb::blocked_range<Index>`的`range`对象可以用来表示一个范围，其中`Index`是一个整数类型，其中在`range`单词后面尾随一个`Xd`，其中`X`是一个整数，表示`range`的维度，比如：`tbb::blocked_range2d<Index>`表示一个二维的范围。
    * `fn`：`tbb`会在并行的线程中调用这个函数对象。
  2. `void parallel_for(Index first, Index last, const CallBakeFn& fn)`
    * `first`：循环的起始索引。
    * `last`：循环的结束索引。
    * `fn`：`tbb`会在并行的线程中调用这个函数对象。<br>
* 除了上述的版本外，还有一些其他的版本：
  * 支持迭代器的版本：`parallel_for_each(Iterator first, Iterator last, const CallBakeFn& fn)`
* 常用用于简单且没有数据依赖的任务，比如：计算sin(x)的值，x为已知且不受任务运行结果影响，并存储在一个数组中。

##缩并并行`parallel_reduce`
* `parallel_reduce`是一个用于缩并操作的模板函数。
* 为了解决`parallel_for`无法处理的有数据依赖的任务，`tbb`提供了`parallel_reduce`函数。
* 比如：计算sin(1)~sin(8)的和，这个任务无法使用`parallel_for`来实现，因为这个任务有数据依赖，即计算sin(1)~sin(8)的和需要先计算sin(1)~sin(7)的和，以此类推。
* 实际上缩并就是利用结合律，类似于递归的思想，`parallel_reduce`会将一个大的任务分解为多个小的任务，然后将这些小的任务的结果合并起来，从而实现并行计算。`GPU`[^1]的`CUDA`编程模型中的`reduce`操作也是类似的思想。
* `T parallel_reduce(const Range& range, const T& identity, const ReduceFn& reduce, const JoinFn& join)`
    * `range`：循环的范围。
    * `identity`：缩并操作的单位元。
    * `reduce`：`tbb`会在并行的线程中调用这个函数对象，用于将一个小的任务的结果合并到一个大的任务中。
    * `join`：`tbb`会在并行的线程中调用这个函数对象，用于将多个小的任务的结果合并到一个大的任务中。
    > 一般情况下，是将2个小的任务结果合并到一个较大的任务中，但是也可以将多个小的任务结果合并到一个较大的任务中，`tbb`会根据任务的数量自动选择合适的合并方式。
* `parallel_deterministic_reduce`：
  * 这个版本的`parallel_reduce`是一个确定性的版本，即不会改变任务的执行顺序，避免编译器优化导致的错误，如在计算浮点数的时候，`CPU`的乱序执行导致计算结果不正确（精度丢失）。例如：sin(1)~sin(8)的和float类型，这个版本的结果是确定的：1.5352609，而`parallel_reduce`的结果是不确定的，但在不需要高精度的情况下，`parallel_reduce`的效率更高，而且精度的损失是可以接受的（小数点后4位）。

#有损并行
* 下面的并行模式是有损的，即在改变工作量的情况下，将时间缩短，请平衡工作量和时间（因为可能增大了2倍的工作量，但是时间只缩短了1.3倍）。
##扫描并行`parallel_scan`
* `parallel_scan`是一个用于扫描操作的模板函数。
* 扫描和缩并类似，不同点是会将中间结果保存下来，而不是只保存最终结果。比如，将sin(0)的值保存至`num[0]`，将sin(0)~sin(1)的和保存至`num[1]`，以此类推。
* `void parallel_scan(const Range& range, const T& identity, const ScanFn& scan, const CombineFn& combine)`
    * `range`：循环的范围。
    * `identity`：扫描操作的单位元。
    * `scan`：`tbb`会在并行的线程中调用这个函数对象，用于将一个小的任务的结果合并到一个大的任务中。
    * `combine`：`tbb`会在并行的线程中调用这个函数对象，用于将多个小的任务的结果合并到一个大的任务中。

[^1]:`GPU`的运算单元功能较为羸弱，只能进行简单的计算，但是`GPU`的运算单元数量较多，因此可以通过上述的缩并将复杂但是可以分解的任务分解为多个简单的任务，然后将这些简单的任务的结果合并起来，从而实现并行计算，但是`CPU`的运算单元功能较为强大，可以进行复杂的计算，因此`CPU`的并行计算不需要无限的分解任务，只需要将任务分解为适当的数量即可。
