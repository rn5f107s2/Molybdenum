SRC_DIR=src
OBJ_DIR=build
MOLY_DIR=src
CXX=g++

DEFAULT_EXE = $(OBJ_DIR)/Molybdenum

all: $(DEFAULT_EXE)

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

EXE ?= $(DEFAULT_EXE)

$(DEFAULT_EXE): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(DEFAULT_EXE) $(OBJECTS)
	@if [ $(EXE) != $(DEFAULT_EXE) ]; then cp $(DEFAULT_EXE) $(EXE); fi

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(DEFAULT_EXE)
