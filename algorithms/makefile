MAKE := make
CXX := g++
CXXFLAGS := -g -std=c++23 -I..\
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
OBJECTS := bfs.o 
ARCHIVE := algorithms.a

all: $(ARCHIVE)

$(ARCHIVE): $(OBJECTS)
	ar rcs $(ARCHIVE) $(OBJECTS)
	+$(MAKE) -C ./tests

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

.PHONY: all clean

clean:
	@rm -f $(ARCHIVE) $(OBJECTS) 
	$(MAKE) -C ./tests clean
