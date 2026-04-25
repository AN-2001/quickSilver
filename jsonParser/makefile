CXX := g++
CXXFLAGS := -Wall -Werror -std=c++17 -I../utils
OBJECTS := lexer.o
ARCHIVE := jsonParser.a

all: $(ARCHIVE)

$(ARCHIVE): $(OBJECTS)
	ar rcs $(ARCHIVE) $(OBJECTS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

.PHONY: all clean

clean:
	@rm -f $(ARCHIVE) $(OBJECTS) 
