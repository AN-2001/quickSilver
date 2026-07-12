CXX := g++

ARCHIVE := ../libalgorithms.a

GTEST_LIBS := -L../../deps/googletest/build/lib
GTEST_INCLUDE := -isystem ../../deps/googletest/googletest/include

CXXFLAGS := -g -std=c++23 -I../.. $(GTEST_INCLUDE) \
            -Wall -Wextra -Wpedantic \
            -Wshadow -Wconversion -Wsign-conversion \
            -Wold-style-cast -Wuseless-cast \
            -Wnull-dereference -Wdouble-promotion \
            -Wformat=2 -Wformat-security \
            -Wlogical-op -Wduplicated-cond -Wduplicated-branches \
            -Wrestrict -Winit-self -Wswitch-default -Wswitch-enum \
            -Wunused -Wunused-variable -Wunused-parameter -Wunused-function \
            -Wmissing-declarations -Wmissing-include-dirs \
            -Wzero-as-null-pointer-constant \
            -Wsuggest-override -Wsuggest-final-types -Wsuggest-final-methods \
            -Wctor-dtor-privacy -Wnon-virtual-dtor \
            -Woverloaded-virtual -Wredundant-decls

LDFLAGS := $(GTEST_LIBS)
LDLIBS := -lgtest_main -lgtest -pthread

COVERAGE_FLAGS := --coverage -O0
ifeq ($(COVERAGE),1)
    CXXFLAGS := $(CXXFLAGS) $(COVERAGE_FLAGS)
    LDFLAGS += --coverage
endif

SRCS := $(wildcard *.cpp)
TARGET := runTests

all: $(TARGET)

test: all
	./$(TARGET)

$(TARGET): $(SRCS) $(ARCHIVE)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@


.PHONY: clean

clean:
	@rm -f $(TARGET) *.gcno *.gcda
