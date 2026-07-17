include ../common.mk

CXXFLAGS += -I..
SUBSYSTEMS := jobBuilder jobParser algorithms connections
LIBS := $(foreach s,$(SUBSYSTEMS),../$(s)/lib$(s).a)
OBJECTS := integration.o
PROJ := integration

LDFLAGS :=
ifeq ($(COVERAGE),1)
    CXXFLAGS += $(COVERAGE_FLAGS)
    LDFLAGS += --coverage
endif

all: $(PROJ)

test: all
	./test.sh

$(PROJ): $(OBJECTS) $(SUBSYSTEMS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LIBS) $(LDFLAGS) -o $@

$(SUBSYSTEMS):
	+$(MAKE) -C ../$@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $<

.PHONY: all clean $(SUBSYSTEMS)

clean:
	@rm -f $(OBJECTS) $(PROJ) *.gcno *.gcda
