CXX := g++
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -pedantic -s
TARGET := wcpp
SRC := main.cpp
BUILD_DIR := build

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

install: $(BUILD_DIR)/$(TARGET)
	sudo cp $< /usr/bin/$(TARGET)
	sudo strip --strip-unneeded /usr/bin/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
