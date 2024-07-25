SRC_DIR=src
OBJ_DIR=build
MOLY_DIR=src
CXX=clang++

DEFAULT_NET_NAME=moly_20240715.nnue
DEFAULT_POLICY_NAME=experiment9q.bin

DEFAULT_EXE = $(OBJ_DIR)/Molybdenum
DATAGEN_EXE = $(OBJ_DIR)/Datagen
DEFAULT_NET = $(MOLY_DIR)/Nets/$(DEFAULT_NET_NAME)
DEFAULT_POLICY = $(MOLY_DIR)/Nets/$(DEFAULT_POLICY_NAME)

EVALFILE ?= $(DEFAULT_NET)
POLICYFILE ?= $(DEFAULT_POLICY)

all: $(DEFAULT_EXE)

datagen: $(DATAGEN_EXE)

CXXFLAGS = -static -Ofast -std=c++17
CXXFLAGS += -Wall -Wextra -pedantic
CXXFLAGS += -march=native
CXXFLAGS += -DEVALFILE=\"$(EVALFILE)\" -DPOLICYFILE=\"$(POLICYFILE)\"

LDFLAGS += -static -Ofast -march=native

ifeq ($(OS),Windows_NT)
	LDFLAGS += -Wl,/STACK:4294967296
endif

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))
DG_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%_dg.o, $(SOURCES)) $(OBJ_DIR)/Datagen.o

$(OBJ_DIR)/%_dg.o: $(SRC_DIR)/%.cpp
	$(CXX) -I$(SRC_DIR) $(CXXFLAGS) -DDATAGEN -c $< -o $@

$(OBJ_DIR)/Datagen.o: $(SRC_DIR)/Datagen/Datagen.cpp
	$(CXX) -I$(SRC_DIR) $(CXXFLAGS) -DDATAGEN -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -I$(SRC_DIR) $(CXXFLAGS) -c $< -o $@

$(OBJECTS): | $(OBJ_DIR)

$(DG_OBJECTS): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

EXE ?= $(DEFAULT_EXE)

$(DEFAULT_EXE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $(DEFAULT_EXE)
	@if [ $(EXE) != $(DEFAULT_EXE) ]; then cp $(DEFAULT_EXE) $(EXE); fi

$(DATAGEN_EXE): $(DG_OBJECTS)
	$(CXX) $(LDFLAGS) $(DG_OBJECTS) -o $(DATAGEN_EXE)

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(DG_OBJECTS) $(DEFAULT_EXE)
