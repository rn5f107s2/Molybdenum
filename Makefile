DEFAULT_EXE=localengine

CWD := $(shell pwd)
EXE ?= $(DEFAULT_EXE)

all:
	cd ~/localengine && make EXE=$(CWD)/$(EXE)
