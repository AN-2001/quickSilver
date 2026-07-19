
include ./common.mk

SUBSYSTEMS := jobBuilder jobParser algorithms
SUBSYSTEMS_TESTS := $(foreach s,$(SUBSYSTEMS),$(s)_test)
LIBS := $(foreach s,$(SUBSYSTEMS),$(s)/lib$(s).a)
OBJECTS := main.o
PROJ := quicksilver

LDFLAGS :=
COVERAGE_FLAGS := --coverage -O0
ifeq ($(COVERAGE),1)
    CXXFLAGS += $(COVERAGE_FLAGS)
    LDFLAGS += --coverage
endif

all: $(PROJ)
	+$(MAKE) -C ./tests

test: $(SUBSYSTEMS_TESTS)
	+$(MAKE) -C ./tests test

coverage:
	mkdir -p coverage
	gcovr \
		--root . \
		--exclude 'deps/.*' \
		--gcov-ignore-parse-errors=negative_hits.warn \
		--html \
		--html-details \
		-o coverage/index.html

$(PROJ): $(OBJECTS) $(SUBSYSTEMS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LIBS) $(LDFLAGS) -o $@

$(SUBSYSTEMS):
	+$(MAKE) -C $@

$(SUBSYSTEMS_TESTS):
	+$(MAKE) -C $(@:_test=) test

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $<

.PHONY: all clean $(SUBSYSTEMS)

clean:
	@rm -f $(OBJECTS) $(PROJ) *.gcno *.gcda
	@rm -rf ./coverage
	$(foreach s,$(SUBSYSTEMS),$(MAKE) -C $(s) clean;)
	$(MAKE) -C ./tests clean
