SRC_DIR=src
OBJ_DIR=build
MOLY_DIR=src
CXX=g++

DEFAULT_EXE = $(OBJ_DIR)/Molybdenum

all: $(DEFAULT_EXE)

CXXFLAGS = -static-libstdc++ -static -static-libgcc -Ofast
CXXFLAGS += -Wall -Wextra -pedantic
CXXFLAGS += -march=native
CXXFLAGS += -Dmakefile

LDFLAGS += -static-libstdc++ -static -static-libgcc -Ofast -march=native

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -I$(SRC_DIR) $(CXXFLAGS) -c $< -o $@

$(OBJECTS): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

EXE ?= $(DEFAULT_EXE)

$(DEFAULT_EXE): $(OBJECTS)
	/usr/bin/c++  -static-libstdc++ -static -static-libgcc -Ofast -Wall -Wextra -pedantic -march=native build/main.o build/Position.o build/UCI.o build/search.o build/timemanagment.o build/Transpositiontable.o build/UCIOptions.o build/nnue.o -o $(DEFAULT_EXE)
	@if [ $(EXE) != $(DEFAULT_EXE) ]; then cp $(DEFAULT_EXE) $(EXE); fi

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(DEFAULT_EXE)
