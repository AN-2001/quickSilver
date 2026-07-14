include ../common.mk

CXXFLAGS += -I..

OBJECTS := jobPipeline.o
ARCHIVE := libjobBuilder.a

COVERAGE_FLAGS := --coverage -O0
ifeq ($(COVERAGE),1)
    CXXFLAGS := $(CXXFLAGS) $(COVERAGE_FLAGS)
endif

all: $(ARCHIVE)

test: 
	+$(MAKE) -C ./tests test

$(ARCHIVE): $(OBJECTS)
	ar rcs $(ARCHIVE) $(OBJECTS)
	+$(MAKE) -C ./tests

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

.PHONY: all clean

clean:
	@rm -f $(ARCHIVE) $(OBJECTS) *.gcno *.gcda
	$(MAKE) -C ./tests clean
