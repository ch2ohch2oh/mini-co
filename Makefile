CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Werror
AR ?= ar
CLANG_FORMAT ?= clang-format

LIB = libminico.a
DEMOS = example_mini example_deps example_await example_event
HEADERS = mini_co.h
SOURCES = mini_co.cpp $(DEMOS:=.cpp)

all: $(LIB) $(DEMOS)

$(LIB): mini_co.o
	$(AR) rcs $@ $^

$(DEMOS): %: %.o $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $*.o $(LIB)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(DEMOS)
	for d in $(DEMOS); do echo "--- $$d"; ./$$d; done

clean:
	rm -f *.o $(LIB) $(DEMOS)

format:
	$(CLANG_FORMAT) -i $(SOURCES) $(HEADERS)

format-check:
	$(CLANG_FORMAT) --dry-run --Werror $(SOURCES) $(HEADERS)

.PHONY: all run clean format format-check
