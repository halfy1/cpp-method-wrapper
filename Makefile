CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

GTEST_FLAGS = -lgtest -lgtest_main -pthread

TARGET = wrapper_demo
TEST_TARGET = test_wrapper

SOURCES = main.cpp
TEST_SOURCES = test_wrapper_gtest.cpp
HEADERS = wrapper.h

all: $(TARGET) $(TEST_TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

$(TEST_TARGET): $(TEST_SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(TEST_SOURCES) -o $(TEST_TARGET) $(GTEST_FLAGS)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

test-verbose: $(TEST_TARGET)
	./$(TEST_TARGET) --gtest_print_time=1 --gtest_color=yes

test-filter: $(TEST_TARGET)
	./$(TEST_TARGET) --gtest_filter=$(FILTER)

check: test run

.PHONY: all clean run test test-verbose test-filter check
