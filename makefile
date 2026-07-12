CXX := g++
SUBSYSTEMS := jobBuilder jobParser algorithms connections
SUBSYSTEMS_TESTS := $(foreach s,$(SUBSYSTEMS),$(s)_test)
LIBS := $(foreach s,$(SUBSYSTEMS),$(s)/$(s).a)
OBJECTS := main.o
CXXFLAGS := -g -std=c++23 -I. \
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
		--exclude '.*/deps/.*' \
		--html \
		--html-details \
		-o coverage/index.html

$(PROJ): $(OBJECTS) $(SUBSYSTEMS)
	$(CXX) $(OBJECTS) $(LIBS) $(LDFLAGS) -o $@

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
