SolutionDir = ../..
ProjectDir = ..
DebugDir = ./Debug
SrcDir = $(ProjectDir)/src

DEBUG = -g3 -O0
CC = g++
CC_FLAGS = -DLINUX=1 -std=c++11

INC = -I$(SrcDir) -I/usr/include/mysql
LIB = 

TARGET = libServerCore.a
SOURCES = $(wildcard $(SrcDir)/*.cpp)
OBJECTS = $(patsubst $(SrcDir)/%.cpp,%.o,$(SOURCES))

$(TARGET) : $(OBJECTS)
	ar rv $(DebugDir)/$(TARGET) $(addprefix $(DebugDir)/,$?)

$(OBJECTS) : %.o : $(SrcDir)/%.cpp
	$(CC) $(DEBUG) $(INC) -c $(CC_FLAGS) $< -o $(DebugDir)/$@

.PRONY : clean
clean:
	-rm -rf $(DebugDir)/*

