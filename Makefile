DEFAULT_EXE=localengine

TARGET ?= $(DEFAULT_EXE)

all:
	cd ~/localengine && make EXE=$(TARGET)
