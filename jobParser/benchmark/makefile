ARCHIVE := ../libjobParser.a

BENCHMARK_LIBS := -L../../deps/benchmark/build/src
BENCHMARK_INCLUDE := -isystem ../../deps/benchmark/include

include ../../common.mk

CXXFLAGS += -I../.. $(BENCHMARK_INCLUDE)
CXXFLAGS += -O3 -g
CXXFLAGS += -fno-omit-frame-pointer

LDFLAGS := $(BENCHMARK_LIBS)
LDLIBS := -lbenchmark_main -lbenchmark -pthread

SRCS := $(wildcard *.cpp)
TARGET := runBenchmarks

all: $(TARGET)

bench: all
	python3 ./gen.py > graph.json
	sudo perf record -F 99 --call-graph dwarf -- ./runBenchmarks --benchmark_min_time=30s
	sudo perf script > script.perf
	stackcollapse-perf.pl script.perf > stack.out
	flamegraph.pl stack.out > flame.svg

$(TARGET): $(SRCS) $(ARCHIVE)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

.PHONY: clean

clean:
	rm -f $(TARGET) *.gcno *.gcda *.perf *.svg *.out *.json
