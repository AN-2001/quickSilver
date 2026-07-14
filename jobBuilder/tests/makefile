ARCHIVE := ../libjobBuilder.a

GTEST_LIBS := -L../../deps/googletest/build/lib
GTEST_INCLUDE := -isystem ../../deps/googletest/googletest/include

include ../../common.mk
CXXFLAGS += -I../.. $(GTEST_INCLUDE)

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
