CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Werror
AR ?= ar

LIB = libminico.a
LIB_OBJS = mini_co.o
DEMO = example_mini
DEMO_OBJS = example_mini.o
HEADERS = mini_co.h

all: $(LIB) $(DEMO)

$(LIB): $(LIB_OBJS)
	$(AR) rcs $@ $^

$(DEMO): $(DEMO_OBJS) $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $(DEMO_OBJS) $(LIB)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(DEMO)
	./$(DEMO)

clean:
	rm -f $(LIB_OBJS) $(DEMO_OBJS) $(LIB) $(DEMO)

.PHONY: all run clean
