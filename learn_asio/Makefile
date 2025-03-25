# 编译器
CXX := g++
TARGET_MAX_CHAR_NUM = 20

# 编译选项
CXXFLAGS := -Wall -g -std=c++17
LIBFLAGS := $(shell pkg-config --libs libssl libcrypto fmt tbb benchmark)
HEADFLAGS := $(shell pkg-config --cflags libssl libcrypto fmt tbb benchmark) -I ./include

# 目标程序名
TARGET := $(basename $(notdir $(SRC)))

## 默认规则
all: $(TARGET) $(LIB)

# 链接目标程序
$(TARGET): $(SRC)
	@$(CXX) $(CXXFLAGS) $< -o ./bin/$@ $(LIBFLAGS) $(HEADFLAGS)

## 清理编译生成的文件
clean:
	rm -f $(TARGET)

## 帮助文档
help:
	@echo "Usage: make SRC=<source_file> [TARGET=<target_name>]"
	@echo ""
	@echo "<target_name> will default to the base name of <source_file> without the extension."
	@echo ''
	@echo 'Targets:'
	@awk '/^[a-zA-Z\-\-_0-9]+:/ { \
    helpMessage = match(lastLine, /^## (.*)/); \
    if (helpMessage) { \
      helpCommand = substr($$1, 0, index($$1, ":")-1); \
      helpMessage = substr(lastLine, RSTART + 3, RLENGTH); \
      printf "%-$(TARGET_MAX_CHAR_NUM)s %s\n", helpCommand, helpMessage; \
    } \
  } \
  { lastLine = $$0 }' $(MAKEFILE_LIST)
