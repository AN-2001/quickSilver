CXX := g++
SUBSYSTEMS := jsonParser
LIBS := $(foreach s,$(SUBSYSTEMS),$(s)/$(s).a)
OBJECTS := main.o
CXXFLAGS := -Wall -Werror -std=c++17 $(foreach s,$(SUBSYSTEMS),-I$(s))
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
