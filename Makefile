CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Werror
AR ?= ar
CLANG_FORMAT ?= clang-format

LIB = libminico.a
LIB_OBJS = mini_co.o
DEMO = example_mini
DEMO_OBJS = example_mini.o
DEPS_DEMO = example_deps
DEPS_DEMO_OBJS = example_deps.o
AWAIT_DEMO = example_await
AWAIT_DEMO_OBJS = example_await.o
EVENT_DEMO = example_event
EVENT_DEMO_OBJS = example_event.o
HEADERS = mini_co.h
SOURCES = mini_co.cpp example_mini.cpp example_deps.cpp example_await.cpp example_event.cpp

all: $(LIB) $(DEMO) $(DEPS_DEMO) $(AWAIT_DEMO) $(EVENT_DEMO)

$(LIB): $(LIB_OBJS)
	$(AR) rcs $@ $^

$(DEMO): $(DEMO_OBJS) $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $(DEMO_OBJS) $(LIB)

$(DEPS_DEMO): $(DEPS_DEMO_OBJS) $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $(DEPS_DEMO_OBJS) $(LIB)

$(AWAIT_DEMO): $(AWAIT_DEMO_OBJS) $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $(AWAIT_DEMO_OBJS) $(LIB)

$(EVENT_DEMO): $(EVENT_DEMO_OBJS) $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $(EVENT_DEMO_OBJS) $(LIB)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(DEMO) $(DEPS_DEMO) $(AWAIT_DEMO) $(EVENT_DEMO)
	./$(DEMO)
	./$(DEPS_DEMO)
	./$(AWAIT_DEMO)
	./$(EVENT_DEMO)

clean:
	rm -f $(LIB_OBJS) $(DEMO_OBJS) $(DEPS_DEMO_OBJS) $(AWAIT_DEMO_OBJS) $(EVENT_DEMO_OBJS) $(LIB) $(DEMO) $(DEPS_DEMO) $(AWAIT_DEMO) $(EVENT_DEMO)

format:
	$(CLANG_FORMAT) -i $(SOURCES) $(HEADERS)

format-check:
	$(CLANG_FORMAT) --dry-run --Werror $(SOURCES) $(HEADERS)

.PHONY: all run clean format format-check
