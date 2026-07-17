include ../common.mk

CXXFLAGS += -I..
SUBSYSTEMS := jobBuilder jobParser algorithms connections
LIBS := $(foreach s,$(SUBSYSTEMS),../$(s)/lib$(s).a)
INTEGRATION := quicksilver_system_integration 
SERVER := quicksilver_as_server

LDFLAGS :=
COVERAGE_FLAGS := --coverage -O0
ifeq ($(COVERAGE),1)
    CXXFLAGS += $(COVERAGE_FLAGS)
    LDFLAGS += --coverage
endif

all: $(INTEGRATION) $(SERVER)

test: all
	./integration_tests.sh

$(INTEGRATION): quicksilver_system_integration.o $(SUBSYSTEMS)
	$(CXX) $(CXXFLAGS) $@.o $(LIBS) $(LDFLAGS) -o $@

$(SERVER): quicksilver_as_server.o  $(SUBSYSTEMS)
	$(CXX) $(CXXFLAGS) $@.o $(LIBS) $(LDFLAGS) -o $@

$(SUBSYSTEMS):
	+$(MAKE) -C ../$@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $<

.PHONY: all clean $(SUBSYSTEMS)

clean:
	@rm -f *.o $(INTEGRATION) $(SERVER) *.gcno *.gcda
