.PHONY: ALL

BENCHMARK := $(shell pkg-config --cflags --libs --static benchmark benchmark_main)
GTEST := $(shell pkg-config --cflags --libs gtest)
SIMDAVX := -mavx2 -mfma
CXXFLAGS := -std=c++20 -O2
CXX := g++

ifdef FILE
TARGET = $(FILE)
else
TARGET = main
endif

ALL: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CXX) -o $@ $< $(CXXFLAGS) $(SIMDAVX) $(GTEST) $(BENCHMARK) && ./$@
