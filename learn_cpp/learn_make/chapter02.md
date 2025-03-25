# 构建`C/C++`项目

## 编译器
* clang 或 gcc
## 构建平台
* Linux

## 编译步骤
* C/C++程序的编译步骤
  * 预处理
  * 编译
  * 汇编
  * 链接
* 编译命令
  * `gcc -E hello.c -o hello.i`
  * `gcc -S hello.i -o hello.s`
  * `gcc -c hello.s -o hello.o`
  * `gcc hello.o -o hello`
* 实际上，`gcc hello.c -o hello`即可完成以上所有步骤，但是，在实际开发中，我们只会使用`gcc -c hello.c -o hello.o`和`gcc hello.o -o hello`两个命令，因为这两个命令分别对应了汇编和链接两个步骤。
* `gcc -c hello.c -o hello.o`：编译`hello.c`文件，生成`hello.o`目标文件, `-c`选项表示只编译不链接，这个`.o`文件是一个二进制文件，里面包含了`hello.c`文件的二进制，但是还没有链接到其他的库文件(`.o或.so`文件)。
* `gcc hello.o -o hello`：链接`hello.o`文件，生成可执行文件`hello`，这个命令会将`hello.o`文件链接到`C`标准库文件，生成可执行文件`hello`，这里只有一个`hello.o`文件，所以不需要指定其他的库文件，实际开发中，我们会链接很多的库文件，比如`-lm`表示链接`math`库文件。

## Makefile
* 在上面我们简单的介绍了`C/C++`程序的编译步骤，接下来我们就使用`Makefile`来管理我们的项目。
  * 构建所需的文件
    * `hello.h` [hello.h](./demo/hello.h)
    * `hello.cpp` [hello.cpp](./demo/hello.cpp)
    * `main.cpp` [main.cpp](./demo/main.cpp)
* 他们的关系如下：
  * `hello.h`：声明了`hello.cpp`文件中的函数
  * `hello.cpp`：实现了`hello.h`文件中的函数
  * `main.cpp`：调用了`hello.cpp`文件中的函数<br>
所以我们需要先将`hello.cpp`文件编译成目标文件`hello.o`，然后再将`main.cpp`和`hello.h`文件编译成目标文件`main.o`，最后将`hello.o`和`main.o`链接成可执行文件`world`。
* 我们已经理清了构建关系，接下来我们就可以编写`Makefile`文件了。
  ```make
  # 生成可执行文件
  world: hello.o main.o
    g++ hello.o main.o -o world

  # 生成hello.o文件
  hello.o: hello.cpp
    g++ -c hello.cpp -o hello.o

  # 生成main.o文件
  main.o: main.cpp hello.h
    g++ -c main.cpp -o main.o

  # 清理文件
  clean:
    rm -f hello.o main.o world
  ```
* 保存为`Makefile`文件，然后执行`make`命令，即可生成可执行文件`world`。
  ```sh
  $ make
  g++ -c hello.cpp -o hello.o
  g++ -c main.cpp -o main.o
  g++ hello.o main.o -o world
  ```
* 执行`./world`命令，即可运行`world`程序。
  ```sh
  $ ./world
  Start...
  Hello, World!
  End...
  ```
* 修改`hello.cpp`文件，重新执行`make`命令，即可重新生成`world`可执行文件，但是只会重新编译`hello.cpp`文件，`main.cpp`文件不会重新编译，这就是`Makefile`的作用，只编译需要重新编译的文件，提高编译速度：
  ```sh
  $ make
  g++ -c hello.cpp -o hello.o
  g++ hello.o main.o -o world
  $ ./world
  Start...
  What can I say!
  End...
  ```
## 总结
* 通过`Makefile`文件，我们可以管理我们的项目，只编译需要重新编译的文件，提高编译速度,但是，随着越来越多的源文件被添加进来，如何高效维护Makefile的规则？
> 在下一章我们会解决这些问题,并通过这些问题了解`Makefile`的一些核心语法[Makefile进阶语法](chapter03.md)
> [上一章：Makefile基础](chapter01.md)<br>
> [返回目录](README.md)
