# 本目录下所有源代码的makefile，使用方法是nmake TARGET_NAME=源代码文件名字（不加后缀）
# 例如 nmake TARGET_NAME=CreateXmlFile
# Author: Wang Xuebin
#指定要使用的库的路径,需要用户修改的变量一般放在makefile文件的最上面


CXX=g++
CXX_FLAGS=-Wall -O2 -D_DEBUG

#指定链接的库

#指定编译路径选项,链接路径选项
LIBXML2 := $(shell pkg-config --cflags --libs libxml-2.0)
LIBFMT := $(shell pkg-config --cflags --libs fmt)

.PHONY : clean
#################################################

#

# Targets 目标

#

$(TARGET_NAME) : $(TARGET_NAME).cpp

clean : $(TARGET_NAME)

$(TARGET_NAME):
	$(CXX) $(CXX_FLAGS) -o $(TARGET_NAME) $(TARGET_NAME).cpp $(LIBXML2) $(LIBFMT)

clean : $(TARGET_NAME)
	@rm -f $(TARGET_NAME)
