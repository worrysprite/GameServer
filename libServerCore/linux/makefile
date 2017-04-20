SolutionDir = ../..
ProjectDir = ..
DebugDir = ./Debug
SrcDir = $(ProjectDir)/src

DEBUG = -g3 -O0 -ggdb -rdynamic
CC = ccache g++
CC_FLAGS = -std=c++11 -fstack-protector-all

INC = -I$(SrcDir) -I/usr/local/mysql/include
LIB = 

SubDir = utils
SRC_PATH = $(SrcDir)
SRC_PATH += $(addprefix $(SrcDir)/, $(SubDir))
SOURCES = $(wildcard $(addsuffix /*.cpp, $(SRC_PATH)))

ObjDir = $(DebugDir)
ObjDir += $(addprefix $(DebugDir)/, $(SubDir))

TARGET = libServerCore.a
OBJECTS = $(patsubst $(SrcDir)/%.cpp,%.o,$(SOURCES))

$(TARGET) : $(ObjDir) $(OBJECTS)
	ar rv $(DebugDir)/$(TARGET) $(addprefix $(DebugDir)/,$(OBJECTS))

$(OBJECTS) : %.o : $(SrcDir)/%.cpp
	$(CC) $(DEBUG) $(INC) -c $(CC_FLAGS) $< -o $(DebugDir)/$@

$(ObjDir) :
	-mkdir $@

.PRONY : clean
clean:
	-rm -rf $(DebugDir)

