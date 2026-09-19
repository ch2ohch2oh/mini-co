CXX ?= c++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Werror
CXXFLAGS += -I.
AR ?= ar
CLANG_FORMAT ?= clang-format

BUILD_DIR = build
LIB = $(BUILD_DIR)/libminico.a
DEMOS = example_mini example_deps example_await example_event example_stackless example_stackless_io example_stackless_await example_stackless_scheduler
BINS = $(DEMOS:%=$(BUILD_DIR)/%)
TESTS = test_stackful test_stackless
TEST_BINS = $(TESTS:%=$(BUILD_DIR)/%)
EXAMPLES_DIR = examples
TESTS_DIR = tests
HEADERS = mini_co.h mini_co_stackless.h
LIB_SOURCES = mini_co.cpp mini_co_stackless.cpp
LIB_OBJECTS = $(LIB_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
SOURCES = $(LIB_SOURCES) $(DEMOS:%=$(EXAMPLES_DIR)/%.cpp) \
	$(TESTS:%=$(TESTS_DIR)/%.cpp)

all: $(LIB) $(BINS)

$(LIB): $(LIB_OBJECTS) | $(BUILD_DIR)
	$(AR) rcs $@ $^

$(BINS) $(TEST_BINS): $(BUILD_DIR)/%: $(BUILD_DIR)/%.o $(LIB) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(BUILD_DIR)/$*.o $(LIB)

$(BUILD_DIR)/%.o: %.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(EXAMPLES_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(TESTS_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

run: $(BINS)
	for d in $(DEMOS); do echo "--- $$d"; ./$(BUILD_DIR)/$$d; done

test: $(TEST_BINS)
	for t in $(TESTS); do echo "--- $$t"; ./$(BUILD_DIR)/$$t; done

clean:
	rm -rf $(BUILD_DIR)

format:
	$(CLANG_FORMAT) -i $(SOURCES) $(HEADERS)

format-check:
	$(CLANG_FORMAT) --dry-run --Werror $(SOURCES) $(HEADERS)

.PHONY: all run test clean format format-check
