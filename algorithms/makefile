include ../common.mk

CXXFLAGS += -I..

OBJECTS := bfs.o dfs.o
ARCHIVE := libalgorithms.a

LDFLAGS :=
COVERAGE_FLAGS := --coverage -O0
ifeq ($(COVERAGE),1)
    CXXFLAGS += $(COVERAGE_FLAGS)
    LDFLAGS += --coverage
endif

all: $(ARCHIVE) make_test 

make_test: $(ARCHIVE)
	+$(MAKE) -C ./tests

test: $(ARCHIVE)
	+$(MAKE) -C ./tests test

$(ARCHIVE): $(OBJECTS)
	ar rcs $(ARCHIVE) $(OBJECTS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

.PHONY: all clean

clean:
	@rm -f $(ARCHIVE) $(OBJECTS) *.gcno *.gcda
	$(MAKE) -C ./tests clean
