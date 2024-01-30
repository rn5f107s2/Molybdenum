SRC_DIR=src
OBJ_DIR=build
MOLY_DIR=src
CXX=clang++

DEFAULT_EXE = $(OBJ_DIR)/Molybdenum

all: $(DEFAULT_EXE)

CXXFLAGS = -static -Ofast -std=c++17
CXXFLAGS += -Wall -Wextra -pedantic
CXXFLAGS += -march=native
CXXFLAGS += -DMAKEFILE

LDFLAGS += -static -Ofast -march=native -flto

ifeq ($(OS),Windows_NT)
	LDFLAGS += -Wl,/STACK:4294967296
endif

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -I$(SRC_DIR) $(CXXFLAGS) -c $< -o $@

$(OBJECTS): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

EXE ?= $(DEFAULT_EXE)

$(DEFAULT_EXE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $(DEFAULT_EXE)
	@if [ $(EXE) != $(DEFAULT_EXE) ]; then cp $(DEFAULT_EXE) $(EXE); fi

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(DEFAULT_EXE)
