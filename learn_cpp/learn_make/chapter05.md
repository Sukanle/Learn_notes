# 美化Makefile的输出

* 如果使用过`cmake`，你可能会注意到它的输出非常漂亮，而`make`的输出则相对较丑，这是因为`cmake`在生成`Makefile`时，会将一些信息写入到`Makefile`中，而`make`则会读取这些信息并显示出来，在这一节中，我们将通过`Makefile`的编写来达到`cmake`的效果。

## 显示构建的进度
* 即便是使用了打印信息，`make`的输出也是非常难看的，因为我在构建的时候，更想知道当地构建的进度，而`cmake`生成的`Makefile`就很好的满足了我们的需求，在`Makefile`中可以通过`echo`命令来显示构建的进度，如下所示：
  ```make
  .PHONY: all clean

  CC = g++
  CFLAGS = -Wall -Wextra -O2

  SRCS = $(wildcard *.cpp)
  OBJS = $(SRCS:.cpp=.o)
  TARGET = world

  # 设置总构建次数
  TOTAL_STEPS = $(words $(SRCS)+$(OBJS)+$(TARGET))
  # 设置当前构建次数
  CURRENT_STEP = 0

  all: $(TARGET)

  $(TARGET): $(OBJS)
    @$(eval CURRENT_STEP=$(shell expr $(CURRENT_STEP) + 1))
    @echo '[$(shell expr $(CURRENT_STEP) \* 100 / $(TOTAL_STEPS))%%] Linking $@...$'
    $(CC) $(CFLAGS) -o $@ $^
    @echo '[$(shell expr $(CURRENT_STEP) \* 100 / $(TOTAL_STEPS))%%] Built target $(TARGET)'

  %.o: %.cpp
    @$(eval CURRENT_STEP=$(shell expr $(CURRENT_STEP) + 1))
    @echo '[$(shell expr $(CURRENT_STEP) \* 100 / $(TOTAL_STEPS))%%] Building $(notdir $<.o)...'
    $(CC) $(CFLAGS) -c -o $@ $<

  clean:
    @echo 'Cleaning...'
    @rm -f $(OBJS) $(TARGET)
  ```
* 在这个`Makefile`中，我们定义了两个变量`TOTAL_STEPS`和`CURRENT_STEP`，分别用来表示总构建次数和当前构建次数，然后在每次构建的时候，我们都会将`CURRENT_STEP`加一，并通过`echo`命令显示当前构建的进度，这样我们就可以清楚的看到当前构建的进度了。
* 这个`Makefile`的输出效果如下所示：
  ```sh
  $ make
  [33%] Building hello.cpp.o...
  [66%] Building main.cpp.o...
  [100%] Linking world...
  [100%] Built target world
  ```
* 通过这个`Makefile`，我们可以清楚的看到当前构建的进度，这样就可以更好的了解当前构建的状态了,但是这个`Makefile`还是有一些问题的，比如有一些重复的代码，还有显示进度未对齐的问题，我们可以通过上一节中介绍的`Makefile`的函数来解决这个问题：
  ```make
  .PHONY: all clean

  CC = g++
  CFLAGS = -Wall -Wextra -O2

  SRCS = $(wildcard *.cpp)
  OBJS = $(SRCS:.cpp=.o)
  TARGET = world

  NUMBER_WIDTH = 3

  TOTAL_STEPS = $(words $(SRCS)+$(OBJS)+$(TARGET))
  CURRENT_STEP = 0

  all: $(TARGET)
    $(call print_aligned_number,100)
    $(call print_built_target)

  $(TARGET): $(OBJS)
    $(call construct_progress)
    $(call print_linked_target, CXX)
    $(CC) $(CFLAGS) -o $@ $^

  %.o: %.cpp
  $(call construct_progress)
  $(call print_building_source, CXX)
  $(CC) $(CFLAGS) -c -o $@ $<

  clean:
    @echo 'Cleaning...'
    @rm -f $(OBJS) $(TARGET)

  # 计算构建进度
  define construct_progress
    @$(eval CURRENT_STEP=$(shell expr $(CURRENT_STEP) + 1))
  endef
  # 输出对齐的数字函数，
  define print_aligned_number
    @printf "[%*d%%]" $(NUMBER_WIDTH) $1
  endef
  # 输出构建进度
  ## 链接目标文件
  define print_linked_target
  $(call print_aligned_number, $(shell expr $(CURRENT_STEP)) \* 100 / $(TOTAL_STEPS)))
  @echo -n 'Linking '
  @echo -n '$1'
  @echo ' executable $@...'
  endef
  ## 编译源文件函数
  define print_building_source
    $(call print_aligned_number, $(shell expr $(CURRENT_STEP)) \* 100 / $(TOTAL_STEPS)))
    @echo -n 'Building '
    @echo -n '$1'
    @echo' object $(notdir $<.o)...'
  endef
  ## 构建成功函数
  define print_built_target
    $(call print_aligned_number, $(shell expr $(CURRENT_STEP)) \* 100 / $(TOTAL_STEPS)))
    @echo 'Built target $(TARGET)'
  ```
* 在这个`Makefile`中，我们定义了三个函数`construct_progress`、`print_aligned_number`和`print_linked_target`，分别用来计算构建进度、输出对齐的数字和输出构建进度，这样我们就可以更好的显示构建的进度了。

## 颜色化输出
* `cmake`生成的`Makefile`中，还有一个很好的特性就是颜色化输出，这样可以更好的区分不同的信息，我们可以通过`printf`函数来和`ANSI`转义码来实现颜色化输出，如下所示：
  ```make
  MAKE_COLOR_NORMAL   := \033[0m       # 设置结束颜色
  MAKE_COLOR_GREEN    := \033[38;5;40m # 设置绿色，表示成功构建
  MAKE_COLOR_DRAGREEN := \033[38;5;34m # 设置深绿色，表示正在构建
  MAKE_COLOR_YELLOW   := \033[33;01m   # 设置黄色，表示警告
  MAKE_COLOR_RED      := \033[31;01m   # 设置红色，表示错误

  .PHONY: all clean
  .DEFAULT_GOAL := all

  CC := g++
  CFLAGS := -Wall -Wextra -O2

  SRCS = $(wildcard *.cpp *.c)
  OBJS = $(SRCS:.cpp=.o)
  TARGET := world

  NUMBER_WIDTH := 3

  CURRENT_STEP = 0
  TOTAL_STEPS := $(words $(SRCS)+$(OBJS)+$(TARGET))

  all: $(TARGET)
    $(call print_aligned_number, 100)
    $(call print_built_target)

  $(TARGET): $(OBJS)
    $(call construct_progress)
    $(call print_linked_target,CXX)
    @$(CC) $(LDFLAGS) -o $@ $^

  %.o: %.cpp
    $(call construct_progress)
    $(call print_building_source,CXX)
    @$(CC) $(CFLAGS) -c $< -o $@

  %.c: %.o
    FILE_TYPE = 'C'
    $(call construct_progress)
    $(call print_building_source,C)
    @$(CC) $(CFLAGS) -c $< -o $@

  clean:
    @printf '$(MAKE_COLOR_YELLOW)Cleaning...$(MAKE_COLOR_NORMAL)\n'
    @rm -f $(OBJS) $(TARGET)

  # 计算构建进度函数
  define construct_progress
    @$(eval CURRENT_STEP := $(shell expr $(CURRENT_STEP) + 1))
  endef
  # 输出对齐的数字函数
  define print_aligned_number
    @printf "[%*d%%]" $(NUMBER_WIDTH) $1
  endef
  # 输出带颜色的信息函数
  ## 链接目标文件函数
  define print_linked_target
    $(call print_aligned_number, $(shell expr $(CURRENT_STEP) \* 100 / $(TOTAL_STEPS)))
    @printf '$(MAKE_COLOR_GREEN)Linking '
    @printf '$1'
    @printf ' executable $@...$(MAKE_COLOR_NORMAL)\n'
  endef
  ## 编译源文件函数
  define print_building_source
    $(call print_aligned_number, $(shell expr $(CURRENT_STEP) \* 100 / $(TOTAL_STEPS)))
    @printf '$(MAKE_COLOR_DRAGREEN)Building '
    @printf '$1'
    @printf ' object $(notdir $<.o)...$(MAKE_COLOR_NORMAL)\n'
  endef
  ## 构建成功函数
  define print_built_target
    $(call print_aligned_number, $(shell expr $(CURRENT_STEP) \* 100 / $(TOTAL_STEPS)))
    @printf ' Built target $(TARGET)\n'
  endef
  ```
* 在这个`Makefile`中，我们定义了一些颜色化输出的`ANSI`转义码，然后在输出信息的时候，我们就可以通过`printf`函数来输出带颜色的信息了，这样就可以更好的区分不同的信息了。
* 这个`Makefile`的输出效果如下所示：<br>
  $ make<br>
  [$~$ 33%] <font color=#00B400> Building CXX object hello.cpp.o...</font><br>
  [$~$ 66%]<font color=#00B400> Building CXX object main.cpp.o...</font><br>
  [100%]<font color=#00DC00> Linking CXX executable world...</font><br>
  [100%] Built target world

## 总结
* 通过这一节的学习，我们学习了如何通过`Makefile`来美化构建的输出，这样我们就可以更好的了解当前构建的进度了，同时也可以通过颜色化输出来区分不同的信息，这样就可以更好的构建我们的项目了，希术这一节的学习对你有所帮助。
