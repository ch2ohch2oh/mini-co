CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Werror
CXXFLAGS += -I.
AR ?= ar
CLANG_FORMAT ?= clang-format

BUILD_DIR = build
LIB = $(BUILD_DIR)/libminico.a
DEMOS = example_mini example_deps example_await example_event
BINS = $(DEMOS:%=$(BUILD_DIR)/%)
EXAMPLES_DIR = examples
HEADERS = mini_co.h
SOURCES = mini_co.cpp $(DEMOS:%=$(EXAMPLES_DIR)/%.cpp)

all: $(LIB) $(BINS)

$(LIB): $(BUILD_DIR)/mini_co.o | $(BUILD_DIR)
	$(AR) rcs $@ $^

$(BINS): $(BUILD_DIR)/%: $(BUILD_DIR)/%.o $(LIB) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(BUILD_DIR)/$*.o $(LIB)

$(BUILD_DIR)/%.o: %.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(EXAMPLES_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

run: $(BINS)
	for d in $(DEMOS); do echo "--- $$d"; ./$(BUILD_DIR)/$$d; done

clean:
	rm -rf $(BUILD_DIR)

format:
	$(CLANG_FORMAT) -i $(SOURCES) $(HEADERS)

format-check:
	$(CLANG_FORMAT) --dry-run --Werror $(SOURCES) $(HEADERS)

.PHONY: all run clean format format-check
