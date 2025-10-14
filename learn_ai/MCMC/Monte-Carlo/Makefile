.PHONY:ALL clean

MAKE_COLOR_NORMAL   := \033[0m       # 设置结束颜色
MAKE_COLOR_RED      := \033[38;5;9m   # 设置红色，表示错误
MAKE_COLOR_YELLOW   := \033[38;5;11m   # 设置黄色，表示警告
MAKE_COLOR_GREEN    := \033[38;5;40m # 设置绿色，表示成功构建
MAKE_COLOR_DRAGREEN := \033[38;5;34m # 设置深绿色，表示正在构建

DIR:=$(shell pwd)
SRC:=$(wildcard $(DIR)/*.cpp)
BIN:=$(addprefix $(DIR)/, ../bin/Monte-Carlo)

FMT:=$(shell pkg-config --cflags --libs fmt)
TBB:=$(shell pkg-config --cflags --libs tbb)

CXX:=clang++
CXX_FLAGS:= -std=c++23 -O2 -Wall -Wextra -ffunction-sections -fdata-sections -fno-rtti -Wl,-dead_strip -flto -fno-exceptions -fvisibility=hidden

TARGET_LIST:=$(addprefix $(BIN)/, $(basename $(notdir $(SRC))))

NUMBER_WIDTH := 3

CURRENT_STEP = 0
TOTAL_STEPS := $(words $(SRC) $(TARGET_LIST))


ALL:$(TARGET_LIST)
	$(call print_aligned_number, 100)
	@printf '$(MAKE_COLOR_GREEN)Build is complete$(MAKE_COLOR_NORMAL)\n'

$(BIN)/Monte-Carlo:$(DIR)/Monte-Carlo.cpp
	$(call target_fn, $<, $@, $(FMT))
$(BIN)/Box-Muller:$(DIR)/Box-Muller.cpp
	$(call target_fn, $<, $@, $(FMT))
$(BIN)/Europen-Call-Option:$(DIR)/Europen-Call-Option.cpp
	$(call target_fn, $<, $@, $(FMT) $(TBB))

clean:
	@printf '$(MAKE_COLOR_YELLOW)Cleaning...$(MAKE_COLOR_NORMAL)\n'
	@rm -rf $(BIN)/* $(DIR)/compile_commands.json


define target_fn
	$(call check_and_create_folder,$(BIN))
	$(call construct_progress)
	$(call print_aligned_number, $(shell expr $(CURRENT_STEP) \* 100 / $(TOTAL_STEPS)))
	@printf '$(MAKE_COLOR_GREEN)Built target $<$(MAKE_COLOR_NORMAL)\n'
	@$(CXX) $(1) -o $(2) $(CXX_FLAGS) $(3)
endef
# 定义检查和创建文件夹的函数接口
define check_and_create_folder
@if [ ! -d $(1) ]; then \
    printf '$(MAKE_COLOR_YELLOW)Creating $(1) folder$(MAKE_COLOR_NORMAL)\n'; \
    mkdir -p $(1); \
fi
endef

define construct_progress
	@$(eval CURRENT_STEP := $(shell expr $(CURRENT_STEP) + 1))
endef

define print_aligned_number
	@printf "[%*d%%]" $(NUMBER_WIDTH) $1
endef
