CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Werror
CXXFLAGS += -I.
AR ?= ar
CLANG_FORMAT ?= clang-format

LIB = libminico.a
DEMOS = example_mini example_deps example_await example_event
EXAMPLES_DIR = examples
HEADERS = mini_co.h
SOURCES = mini_co.cpp $(DEMOS:%=$(EXAMPLES_DIR)/%.cpp)

all: $(LIB) $(DEMOS)

$(LIB): mini_co.o
	$(AR) rcs $@ $^

$(DEMOS): %: $(EXAMPLES_DIR)/%.o $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $(EXAMPLES_DIR)/$*.o $(LIB)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(DEMOS)
	for d in $(DEMOS); do echo "--- $$d"; ./$$d; done

clean:
	rm -f *.o $(EXAMPLES_DIR)/*.o $(LIB) $(DEMOS)

format:
	$(CLANG_FORMAT) -i $(SOURCES) $(HEADERS)

format-check:
	$(CLANG_FORMAT) --dry-run --Werror $(SOURCES) $(HEADERS)

.PHONY: all run clean format format-check
