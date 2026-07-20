include ../common.mk

CXXFLAGS += -I..

OBJECTS := lexer.o parser.o
ARCHIVE := libjobParser.a

COVERAGE_FLAGS := --coverage -O0

ifeq ($(COVERAGE),1)
    CXXFLAGS := $(CXXFLAGS) $(COVERAGE_FLAGS)
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
