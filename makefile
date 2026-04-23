CC := g++
CFLAGS := -Wall -Werror -std=c++17
OBJECTS := main.o

graphToys: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

%.o: %.cpp
	$(CC) -c $(CFLAGS) $<

clean:
	rm *.o graphToys
