all: Molybdenum

SRC_DIR=src
OBJ_DIR=build
CXX=g++

CXXFLAGS = --std=c++17 -march=native -mtune=native
CXXFLAGS += -Ofast -static-libstdc++ -static -static-libgcc
CXXFLAGS += -Wall -Wextra -pedantic
CXXFLAGS += -Dmakefile

LDFLAGS += -static-libstdc++ -static -static-libgcc

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -I$(SRC_DIR) $(CXXFLAGS) -c $< -o $@

$(OBJECTS): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

Molybdenum: $(OBJECTS)
	$(CXX) $(LDFLAGS) -o Molybdenum $(OBJECTS)

.PHONY: clean
clean:
	rm -f $(OBJECTS) Molybdenum
