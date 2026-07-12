CXX := g++
SUBSYSTEMS := jobBuilder jobParser algorithms connections
LIBS := $(foreach s,$(SUBSYSTEMS),$(s)/$(s).a)
LDFLAGS :=
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
COVERAGE_FLAGS := --coverage -O0
PROJ := quicksilver

ifeq ($(COVERAGE),1)
    CXXFLAGS := $(COMMON_FLAGS) $(COVERAGE_FLAGS)
    LDFLAGS += --coverage
else
    CXXFLAGS := $(COMMON_FLAGS) -O2
endif

all: $(PROJ)
	+$(MAKE) -C ./tests

$(PROJ): $(OBJECTS) $(SUBSYSTEMS)
	$(CXX) $(OBJECTS) $(LIBS) $(LDFLAGS) -o $@

$(SUBSYSTEMS):
	+$(MAKE) -C $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $<

.PHONY: all clean $(SUBSYSTEMS)

clean:
	@rm -f $(OBJECTS) $(PROJ)
	$(foreach s,$(SUBSYSTEMS),$(MAKE) -C $(s) clean;)
	$(MAKE) -C ./tests clean
