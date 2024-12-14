# 生成可执行文件
#world: hello.o main.o
#	g++ hello.o main.o -o world

# 生成hello.o文件
#hello.o: hello.cpp
#	g++ -c hello.cpp -o hello.o

# 生成main.o文件
#main.o: main.cpp hello.h
#	g++ -c main.cpp -o main.o

# 清理文件
#clean:
#	rm -f hello.o main.o world

# 列出所有的.cpp文件：
#SRCS = $(wildcard *.cpp)

# 根据SRCS生成目标文件：
#OBJS = $(SRCS:.cpp=.o)

# 根据SRCS生成依赖文件：
#DEPS = $(SRCS:.cpp=.d)

#TARGET = world

# 指定编译器：
#CC = g++

# 默认目标：
#$(TARGET): $(OBJS)
#	$(CC) -o $@ $^

# 生成依赖规则：xyz.d 的规则由xyz.cpp生成
#%.d: %.cpp
#	rm -f $@; \
	$(CC) -MM $< > $@.tmp; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.tmp > $@; \
	rm -f $@.tmp

# 模式规则：
#%.o: %.cpp
#	@echo 'compiling $<...'
#	$(CC) -c -o $@ $<

#clean:
#	rm -f *.o *.d $(TARGET)

# 引入依赖文件：
#include $(DEPS)
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
TOTAL_STEPS := $(words $(SRCS) $(OBJS) $(TARGET))

all: $(TARGET)
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
	@printf ' executable $(notdir $@)...$(MAKE_COLOR_NORMAL)\n'
endef
## 编译源文件函数
define print_building_source
	$(call print_aligned_number, $(shell expr $(CURRENT_STEP) \* 100 / $(TOTAL_STEPS)))
	@printf '$(MAKE_COLOR_DRAGREEN)Building '
	@printf '$1'
	@printf ' object $(notdir $(<:.cpp=.o))...$(MAKE_COLOR_NORMAL)\n'
endef
## 构建成功函数
define print_built_target
	$(call print_aligned_number, 100)
	@printf ' Built target $(TARGET)\n'
endef
