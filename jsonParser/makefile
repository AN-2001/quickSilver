CXX := g++
CXXFLAGS := -Wall -Werror -std=c++17 -I.. -g
OBJECTS := lexer.o parser.o
ARCHIVE := jsonParser.a

all: $(ARCHIVE)

$(ARCHIVE): $(OBJECTS)
	ar rcs $(ARCHIVE) $(OBJECTS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

.PHONY: all clean

clean:
	@rm -f $(ARCHIVE) $(OBJECTS) 
