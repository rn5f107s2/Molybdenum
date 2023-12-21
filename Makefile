all: Molybdenum

SRC=src
OBJ=build
CXX=g++

CXXFLAGS = --std=c++17 -march=native -mtune=native
CXXFLAGS += -Ofast -static-libstdc++ -static -static-libgcc
CXXFLAGS += -Wall -Wextra -pedantic

LDFLAGS += -static-libstdc++ -static -static-libgcc

SOURCES := $(wildcard $(SRC)/*.cpp)
OBJECTS := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCES))

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) -I$(SRC) $(CXXFLAGS) -c $< -o $@

Molybdenum: $(OBJECTS)
	$(CXX) $(LDFLAGS) -o Molybdenum $(OBJECTS)

.PHONY: clean
clean:
	rm -f $(OBJECTS) Molybdenum
