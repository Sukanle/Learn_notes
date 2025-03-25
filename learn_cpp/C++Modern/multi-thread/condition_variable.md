# `condition_variable`

* 隶属于`<condition_variable>`头文件
* 中文名又名`条件变量`，显然它跟`if-else`应该隶属于同一语义，用于条件判断；但不同的是，`condition_variable`是用于`thread`之间的判断，准确的说是相当于把`thread`当作了当满足了`condition_variable`时调用的分支内部功能，即`condition_variable`在当作`thread`之间的`if-else`语句时，也充当`if-else`语句判断依据的变量（或布尔表达式）。
* 公用函数：
    * 等待
        * `wait()`
        * `wait_for()`
        * `wait_until()`
    > 注意：所有等待函数都有一个重载版本，均为在原有本身之上，添加一个`lamdba`对象，用于判断阻塞是否为真结束[^1]
    * 通知
        * `notify_one()`
        * `notify_all()`

* 与`promise-future`的对比
| ****        | **condition_variable** | **promise-future** |
|-------------|------------------------|--------------------|
| **多次同步**    | √                      | ×                  |
| **接受端异常处理** | ×                      | √                  |
| **虚假唤醒**    | √                      | ×                  |
| **唤醒丢失**    | √                      | ×                  |
| **临界区**     | √                      | ×                  |


[^1]: 在`C++`中，`wait()`函数不仅仅在真的通知时结束阻塞，会存在着 **假唤醒** 现象，标准委员会出于性能高效的考虑，允许这样的 **假唤醒** ,为了防止这种情况，才诞生了接受`lamdba`对象用于判断的等待函数。
