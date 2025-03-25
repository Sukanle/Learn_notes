# *`Makefile`高级语法*

## 模板编写（模式匹配）
* 前面介绍了使用`Makefile`的隐式规则，这里介绍一种更加高级的用法：模式规则。它允许我们自定义一种规则，使得`make`可以根据文件名的模式来生成目标文件，即自定义的隐式规则（`make`内置的隐式规则是不能修改的）。
  * 源文件：[main.cpp](demo1/main.cpp) [hello.cpp](demo1/hello.cpp) [hello.h](demo1/hello.h)
  ```make
  OBJS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))
  TARGET = main

  $(TARGET): $(OBJS)
    cc -o $@ $^

  %.o: %.cpp
    @echo 'compiling $<...'
    cc -c -o $@ $<

  clean:
    rm -f *.o $(TARGET)
  ```
  当`make`执行`world: hello.o main.o`时，发现没有`hello.o`文件，于是执行`%.o: %.cpp`规则，生成`hello.o`文件；然后执行`main: hello.o main.o`规则，生成`main`文件，即跟编写`C++`的泛型编程类似，`make`根据模板模式规则为我们动态的创建了如下规则：
  ```make
  hello.o: hello.cpp
    @echo 'compiling $<...'
    cc -c -o $@ $<
  ```
  这样我们就可以根据文件名的模式来生成目标文件，而不需要为每个文件都写一条规则，以下就是`make`使用的模式匹配在终端的输出：
  ```shell
  $ make
  compiling hello.cpp...
  cc -c -o hello.o hello.cpp
  compiling main.cpp...
  cc -c -o main.o main.cpp
  cc -o world hello.o main.o
  ```
* 但是，模式规则仍然没有解决修改`hello.h`头文件不会触发`main.cpp`重新编译的问题，这个遗留许久的依赖问题在本章就会解决，而且模式规则是按需生成，如果我们在当前目录创建一个`man.o`文件，因为make并不会在执行过程中用到它，所以并不会自动生成`man.o: man.cpp`这个规则。

## 自动生成依赖
* 为了解决上面提到的依赖问题，我们可以使用`gcc`的`-MM`选项来生成依赖文件，然后在`Makefile`中引入这个依赖文件，这样就可以解决`hello.h`头文件修改不会触发`main.cpp`重新编译的问题。
  * 源文件：[main.cpp](demo2/main.cpp) [hello.cpp](demo2/hello.cpp) [hello.h](demo2/hello.h)
  ```make
  # 列出所有的.cpp文件：
  SRCS = $(wildcard *.cpp)

  # 根据SRCS生成目标文件：
  OBJS = $(SRCS:.cpp=.o)

  # 根据SRCS生成依赖文件：
  DEPS = $(SRCS:.cpp=.d)

  # 指定编译器：
  CC = g++

  TARGET = world

  # 默认目标：
  $(TARGET): $(OBJS)
    $(CC) -o $@ $^

  # 生成依赖规则：xyz.d 的规则由xyz.cpp生成
  %.d: %.cpp
    rm -f $@; \
    $(CC) -MM $< > $@.tmp; \
    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.tmp > $@; \
    rm -f $@.tmp

  # 模式规则：
  %.o: %.cpp
    @echo 'compiling $<...'
    $(CC) -c -o $@ $<

  clean:
    rm -f *.o *.d $(TARGET)

  # 引入依赖文件：
  include $(DEPS)
  ```
  * 这里使用了一个sed命令，sed是一个流编辑器，它是文本处理中非常中的工具，可以实现文本替换、删除、新增、选取等功能，这里简单的介绍一下它的语法：<br>
  sed 's,正则表达式,替换的内容,g' file.txt<br>
  其中s表示替换，g表示全局替换，file.txt是要处理的文件，正则表达式中表示匹配的文件名。
* 变量`$(SRCS)`通过扫描目录可以确定为`main.cpp``hello.cpp`，因此，变量`($SRCS)`赋值为`main.o``hello.o`，变量`$(SRCS)`赋值为`mian.d``hello.d`。
* 通过`include $(DEPS)`我们引入`hello.d`和`main.d`文件，但是这两个文件一开始并不存在，不过，`make`通过模式规则匹配到`%d: %.cpp`，这就给了我们一个机会，在这个模式规则内部，用`cc -MM`命令外加`sed`把`.d`文件创建出来。
* 运行`make`,首次输出如下：<br>
  ```sh
  $ make
  rm -f main.d; \
  g++ -MM main.cpp > main.d.tmp; \
  sed 's,\(main\)\.o[ :]*,\1.o main.d : ,g' < main.d.tmp > main.d; \
  rm -f main.d.tmp
  rm -f hello.d; \
  g++ -MM hello.cpp > hello.d.tmp; \
  sed 's,\(hello\)\.o[ :]*,\1.o hello.d : ,g' < hello.d.tmp > hello.d; \
  rm -f hello.d.tmp
  compiling hello.cpp...
  g++ -c -o hello.o hello.cpp
  compiling main.cpp...
  g++ -c -o main.o main.cpp
  g++ -o world hello.o main.o
  ```
  这里`make`首先生成了`hello.d`和`main.d`文件，然后引入这两个文件，这样就解决了`hello.h`头文件修改不会触发`main.cpp`重新编译的问题。
  > 注意：
  > 可能会出现`No such file or directory`错误，这是因为`make`在执行`include $(DEPS)`时，`hello.d`和`main.d`文件还不存在，所以会报错，但是这并不影响`make`的执行，因为`make`会继续执行`%.o: %.cpp`规则，生成`hello.o`和`main.o`文件，然后再次执行`make`就不会报错了。

## `Makefile`的条件判断
* 条件`if/else`
  ```make
  foo = ok
  all:
    ifeq ($(foo),ok)
      @echo 'foo is $(foo)'
    else
      @echo 'foo is not defined'
    endif
  ```
  这里`ifeq`是一个`make`内置的函数，用来判断`foo`是否等于`ok`，如果等于，则输出`foo is $(foo)`，否则输出`foo is not defined`。
* 检查是否定义了变量：
  ```make
  all:
    ifdef foo
      @echo 'foo is $(foo)'
    else
      @echo 'foo is not defined'
    endif
  ```
  这里`ifdef`是一个`make`内置的函数，用来检查是否定义了变量`foo`，如果定义了，则输出`foo is $(foo)`，否则输出`foo is not defined`。
* 检查变量是否空：
  ```make
  nullstring =
  foo = $(nullstring) # 行尾；这里有一个空格 

  all:
  ifeq ($(strip,$(foo)),)
    @echo 'foo is empty after being stripped'
  endif
  ifeq ($(nullstring),)
  @echo 'nullstring doesn't even have spaces'
  ```
  这里`strip`是一个`make`内置的函数，用来去掉变量`foo`的前后空格，如果`foo`是空的，则输出`foo is empty after being stripped`，否则输出`nullstring doesn't even have spaces`。
* 通过`Makefile`的条件判断，我们可以根据不同的条件执行不同的规则，比如可以根据不同的平台选择不同的编译器：
  ```make
  ifeq ($(OS),Windows_NT)
    CC = cl
  else
    CC = gcc
  endif

  all:
    @echo 'CC is $(CC)'
  ```
  这里我们使用`ifeq`条件判断，`$(OS)`是一个系统变量，如果是`Windows_NT`，则使用`cl`编译器，否则使用`gcc`编译器。

## `Makefile`函数
* `Makefile`提供了一些内置函数,下面是一些常用的函数：
  1. `patsubst`函数可以用来替换字符串：
  ```make
  foo := a.o b.o c.o
  bar := $(patsubst %.o,%.c,$(foo))
  all:
    @echo 'bar is $(bar)'
  ```
  这里`patsubst`函数用来替换`foo`中的`.o`为`.c`，输出`bar is a.c b.c c.c`。

  2. `subst`函数可以用来替换字符串：
  ```make
  foo := a.o b.o c.o
  bar := $(subst .o,.c,$(foo))
  all:
    @echo 'bar is $(bar)'
  ```

  3. `strip`函数可以用来去掉字符串前后的空格：
  ```make
  foo := a.o b.o c.o
  bar := $(strip $(foo))
  all:
    @echo 'bar is $(bar)'
  ```

  4. `wildcard`函数可以用来获取指定目录下的所有文件：
  ```make
  foo := $(wildcard *.cpp)
  all:
    @echo 'foo is $(foo)'
  ```
  这里`wildcard`函数用来获取当前目录下的所有`.cpp`文件，输出`foo is main.cpp hello.cpp`。

  5. `foreach`函数可以用来遍历列表：
  ```make
  foo := a b c
  bar := $(foreach n,$(foo),$(n).o)
  all:
    @echo 'bar is $(bar)'
  ```
  这里`foreach`函数用来遍历`foo`列表，输出`bar is a.o b.o c.o`。

  6. `filter`函数可以用来过滤列表：
  ```make
  foo := a.o b.c c.o
  bar := $(filter %.o,$(foo))
  all:
    @echo 'bar is $(bar)'
  ```
  这里`filter`函数用来过滤`foo`列表中的`.o`文件，输出`bar is a.o c.o`。
* 当然你也可以自定义函数：
  ```make
  # 定义一个打印参数的函数
  define PRINT_ARGS
    @echo "Arg1: $(1)"
    @echo "Arg2: $(2)"
    @echo "Arg3: $(3)"
  endef

  # 调用函数
  $(call PRINT_ARGS, hello, world, 123)
  ```

## `Makefile`的项目管理
* 先前的`Makefile`都是在当前目录下生成文件，但是在实际开发中，我们需要对项目文件结构进行管理，这里介绍一种`Makefile`的项目管理方式：
  * 项目结构：
  ```shell
  project
  ├── bin
  ├── build
  │   └── deps
  ├── include
  │   └── hello.h
  ├── lib
  ├── src
  │   ├── hello.cpp
  │   └── main.cpp
  └── Makefile
  ```
  * `Makefile`文件：
  ```make
  # 源文件目录
  SRCDIR = src
  # 头文件目录
  INCDIR = include
  # 目标文件目录
  BINDIR = bin
  # 库文件目录
  LIBDIR = lib
  # 依赖文件目录
  DEPDIR = build/deps

  # 源文件
  SRCS = $(wildcard $(SRCDIR)/*.cpp)
  # 目标文件
  OBJS = $(patsubst $(SRCDIR)/%.cpp, $(BINDIR)/%.o, $(SRCS))
  # 依赖文件
  DEPS = $(OBJS:.o=.d)

  # 编译器
  CC = g++
  # 编译选项
  CFLAGS = -I$(INCDIR) -std=c++11

  # 目标文件
  TARGET = $(BINDIR)/world

  # 默认目标
  all: $(TARGET)

  # 生成目标文件
  $(BINDIR)/%.o: $(SRCDIR)/%.cpp
    @echo 'compiling $<...'
    $(CC) -c $(CFLAGS) -o $@ $<

  # 生成依赖文件
  $(BINDIR)/%.d: $(SRCDIR)/%.cpp
    rm -f $@; \
    $(CC) -MM $(CFLAGS) $< > $@.tmp; \
    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.tmp > $@; \
    rm -f $@.tmp

  # 引入依赖文件
  include $(DEPS)

  # 生成目标文件
  $(TARGET): $(OBJS)
    $(CC) -o $@ $^

  # 清理
  clean:
    rm -f $(OBJS) $(DEPS) $(TARGET)
  ```
  * 这里我们把源文件放在`src`目录下，头文件放在`include`目录下，目标文件放在`bin`目录下，库文件放在`lib`目录下，依赖文件放在`build/deps`目录下，这样就可以更好的管理项目文件结构。

## 总结
* 本章介绍了`Makefile`的高级用法，包括模式匹配、自动生成依赖、条件判断、内置函数等，这些高级用法可以让我们更加灵活的使用`Makefile`，提高我们的工作效率。
* `Makefile`教程到此结束，希望通过本教程的学习，大家对`Makefile`有了更深入的了解，也希望大家能够在实际开发中灵活运用`Makefile`，提高我们的工作效率，若有不足之处，还请大家多多指正，谢谢！
## 参考资料
[返回目录](README.md)
[番外：`Makefile`美化输出](chapter05.md)
[快速构建：`cmake`](../learn_cmake/README.md)
