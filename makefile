CXX := g++
SUBSYSTEMS := jsonParser
LIBS := $(foreach s,$(SUBSYSTEMS),$(s)/$(s).a)
OBJECTS := main.o
CXXFLAGS := -O3 -Wall -Werror -std=c++23 -I. -g
PROJ := graphToys

all: $(PROJ)

graphToys: $(OBJECTS) $(SUBSYSTEMS)
	$(CXX) $(OBJECTS) $(LIBS) -o $@

$(SUBSYSTEMS):
	+$(MAKE) -C $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $<

.PHONY: all clean $(SUBSYSTEMS)

clean:
	@rm -f $(OBJECTS) $(PROJ)
	$(foreach s,$(SUBSYSTEMS),+$(MAKE) -C $(s) clean;)
