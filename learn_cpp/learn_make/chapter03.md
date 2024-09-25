# *`Makefile`进阶语法*

## 隐式规则
* 隐式规则是`Makefile`中的一种特殊规则，它可以让`make`根据文件的扩展名自动推导出如何生成目标文件。
* 实际上，`make`在执行时会根据文件的扩展名自动查找对应的隐式规则，然后执行该规则生成目标文件。
* 因此，我们可以通过定义隐式规则来简化`Makefile`的编写，提高`Makefile`的可读性。
  ```make
  # 只保留生成的world.out的规则：
  world.out: hello.o main.o
    g++ -o world.out hello.o main.o

  clean:
    rm -f *.o world.out
  ```
* 然而有利必有弊，隐式规则的使用也会带来一些问题，对于C/C++程序员来说，最常见的问题就是`make`无法自动推导出头文件的依赖关系。

## 变量
* 当我们的`Makefile`中有大量的<font color="aqua">**重复**的</font><font color="orange">*文件名*</font>、<font color="orange">*路径名*</font>、<font color="orange">*编译器*</font>选项等内容时，一来容易写错，二来也不利于维护，如果要修改这些内容，需要修改多处，非常麻烦，三来也不利于`Makefile`的可读性，一大串的*路径名*、*编译器*选项等内容让人看起来非常不舒服。
* 编程语言中的变量可以解决这个问题，`Makefile`中也有变量的概念，我们可以使用变量来定义这些重复的内容，然后在需要使用的地方引用这些变量。
* 继续以前面的例子为例，编译的最终文件`world`重复出现了3次，因此，我们可以使用变量来定义这个文件名，然后在需要使用的地方引用这个变量。
  ```make
  # 定义变量
  TARGET = world.out

  # 使用变量
  $(TARGET): hello.o main.o
	g++ -o $(TARGET) hello.o main.o

  clean:
	rm -f *.o $(TARGET)
  ```
	* 变量定义用`变量名=值`或者`变量名:=值`，通常变量名用大写字母表示「跟`C/C++`中的宏一样」，值可以是任意字符串。
	* 变量引用用`$(变量名)`，`make`在执行时会自动将变量引用替换为变量的值。
  > 注意到这里的`:=`吗？这是一个高级语法，您大可认为它相当于`C/C++`中的`const`关键字，它表示这个变量的值是不可修改的，这样可以避免一些不必要的错误,但是，如果您实在要是更新这个变量的值，可以使用`:=`来重新定义变量，通常来说，使用`:=`表示这个变量的值是不可修改的，使用`=`表示这个变量的值是可修改的，这样可以提高`Makefile`的可读性,而且将常用且不可修改的变量定义为`:=`可以有效的提升`Makefile`的执行效率。

## 自动依赖关系
* 依赖关系的规则也是容易重复的，而且在大型项目中，头文件的依赖关系是非常复杂的，如果我们手动维护这些依赖关系，那么将会非常麻烦。`Makefile`也可以为重复的依赖关系定义变量，然后在需要使用的地方引用这些变量。
  ```make
  # 定义变量
  TARGET = world.out
  DEPS = hello.o main.o

  # 使用变量
  $(TARGET): $(DEPS)
  g++ -o $(TARGET) $(DEPS)

  clean:
  rm -f *.o $(TARGET)
  ```
* 但是，这样还是不够，因为我们还需要手动维护头文件的依赖关系，这样非常麻烦，而且容易出错。`Makefile`也可以自动生成`hello.o``main.o`这个依赖「列表」。因为每个`.o`文件是由对应的`.c`文件编译生成的，所以我们可以通过`.c`文件自动生成`.o`文件的依赖关系。
  ```make
  # 定义变量
  OBJS = $(patsubst %.c,%.o,$(wildcard *.c))
  TARGET = world

  # 使用变量
  $(TARGET): $(OBJS)
  g++ -o $(TARGET) $(OBJS)

  clean:
  rm -f *.o $(TARGET)
  ```
  1. `wildcard`函数用于获取当前目录下的所有`.c`文件，返回一个文件列表。
  2. `patsubst`函数用于将文件列表中的`.c`文件替换为`.o`文件，返回一个`.o`文件列表。
  3. `OBJS`变量保存了所有的`.o`文件，`TARGET`变量保存了最终的目标文件名。<br>
  这样，我们就可以自动生成`.o`文件的依赖关系了，而且不用手动维护，非常方便。

## 内置变量
* `Makefile`中有一些内置变量，可以帮助我们简化`Makefile`的编写.
* `Makefile`中的内置变量有很多，这里只介绍一些常用的内置变量.
  | 变量名          | 描述             | 变量名       | 描述         |
  | :----:          | :---             | :----:       | :---         |
  | `CC`            | C/C++编译器      | `VPATH`      | 搜索路径     |
  | `CFLAGS`        | C/C++编译器选项  | `OSTYPE`     | 操作系统类型 |
  | `LDFLAGS`       | 链接器选项       | `PATH`       | 环境变量     |
  | `LDLIBS`        | 链接库           | `TERM`       | 终端类型     |
  | `RM`            | 删除文件命令     | `SHELL`      | Shell类型    |
  | `AR`            | 静态库命令       | `SHELL`      | Shell命令    |
  | `ARFLAGS`       | 静态库选项       | `SHELLFLAGS` | Shell选项    |
  | `MAKE`          | `make`命令       | `MACHTYPE`   | 机器类型     |
  | `MAKEFLAGS`     | `make`选项       | `HOSTNAME`   | 主机名       |
  | `MAKELEVEL`     | `make`的递归层次 | `USER`       | 用户名       |
  | `MAKECMDGOALS`  | `make`的目标列表 | `LOGNAME`    | 登录名       |
  | `MAKE_VERSION`  | `make`版本       | `HOME`       | 用户目录     |
  | `MAKEFILE_LIST` | `make`文件列表   | `PWD`        | 当前工作目录 |
  | `MAKEFILES`     | `make`文件       | `CURDIR`     | 当前目录     |
  | `MFLAGS`        | `make`选项       | `SUFFIXES`   | 后缀列表     |
* 比如我们可以用变量$(CC)来表示C/C++编译器，$(CFLAGS)来表示编译器选项，$(LDFLAGS)来表示链接器选项，$(LDLIBS)来表示链接库等等。
  ```make
  # 定义变量
  OBJS = $(patsubst %.c,%.o,$(wildcard *.c))
  TARGET = world

  # 使用变量
  $(TARGET): $(OBJS)
  $(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)

  clean:
  $(RM) -f *.o $(TARGET)
  ```
* 即使我们没有定义这些变量，`make`也会自动定义这些变量，比如`CC`变量默认是`gcc`，`CFLAGS`变量默认是`-g -O2`等等，我们可以直接使用这些变量，而不用手动定义；当然，我们也可以手动定义这些变量，这样可以覆盖`make`自动定义的变量。
  ```make
  CC = g++
  ...
  ```

## 自动变量
* 如果你看过一些开源项目的`Makefile`文件，肯定会发现有一些特殊的变量，比如`$@`、`$<`、`$^`等等，这些就是`make`的自动变量，它们在一个规则中自动替换为对应的值。<br>
  下面是一些常用的自动变量：
  * `$@`：表示规则中的目标文件名。
  * `$<`：表示规则中的第一个依赖文件名。
  * `$^`：表示规则中的所有依赖文件名。
  * `$?`：表示规则中所有比目标文件更新的依赖文件名。<br>
  通过这些自动变量，我们可以进一步地简化`Makefile`的编写。
  ```make
  world: hello.o main.o
  cc -o $@ $^
  ```
  > <font color="red">注意：</font>自动变量只能在规则中使用，不能在规则外使用，否则`make`会报错，且在没有<font color="red">**歧义**</font>的情况下，自动变量可以不用加括号，有歧义的情况下，需要加括号，比如`$(CC)`和`$CC`，前者表示`CC`变量的值，后者表示`$CC`变量的值。

* 通过本章的学习，我们了解了`Makefile`的进阶语法，包括隐式规则、变量、自动依赖关系、内置变量和自动变量等等，这些内容可以帮助我们更好地编写`Makefile`，提高`Makefile`的可读性和可维护性，同时也可以提高`Makefile`的执行效率，减少不必要的错误，提高编译的速度，是`Makefile`的进阶知识。
* 下一章我们将学习`Makefile`的高级语法:[Makefile高级语法](chapter04.md)
> [上一章: 构建C/C++项目](chapter02.md)<br>
> [返回目录](README.md) 
