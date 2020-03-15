PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

OBJS = N1003_Semafor1.o

ifeq ($(BUILD_MODE),debug)
	CFLAGS += -g
else ifeq ($(BUILD_MODE),run)
	CFLAGS += -O2
else
#	$(error Build mode $(BUILD_MODE) not supported by this Makefile)
endif

all: N1003_Semafor1

N1003_Semafor1: 	$(OBJS)
	$(CXX) -o $@ $^

%.o:	$(PROJECT_ROOT)%.cpp
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o:	$(PROJECT_ROOT)%.c
	$(CC) -DSEMMSL=32   -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	rm -fr N1003_Semafor1 $(OBJS)
