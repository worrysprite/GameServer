SolutionDir = ../..
ProjectDir = ..
SrcDir = $(ProjectDir)/src
DebugDir = ./Debug
libServerCoreDir = $(SolutionDir)/libServerCore

DEBUG = -g3 -O0
CC = g++
CC_FLAGS = -DLINUX=1 -std=c++11

INC = -I$(SrcDir) -I$(libServerCoreDir)/src -I/usr/include/mysql
LIB = -L$(libServerCoreDir)/linux/Debug -L/usr/lib64/mysql -lServerCore -lpthread -lmysqlclient -lrt -ldl

TARGET = GameServer
SOURCES = $(wildcard $(SrcDir)/*.cpp)
SOURCES += $(wildcard $(SrcDir)/database/*.cpp)
SOURCES += $(wildcard $(SrcDir)/gameLogic/*cpp)
OBJECTS = $(patsubst $(SrcDir)/%.cpp,%.o,$(SOURCES))

$(TARGET) : $(OBJECTS) ServerCore
	$(CC) $(DEBUG) -o $(DebugDir)/$(TARGET) $(addprefix $(DebugDir)/,$(OBJECTS)) $(LIB)

$(OBJECTS) : %.o : $(SrcDir)/%.cpp
	$(CC) $(DEBUG) $(INC) -c $(CC_FLAGS) $< -o $(DebugDir)/$@

ServerCore :
	make -C $(libServerCoreDir)/linux

.PRONY : clean
clean:
	-rm -rf $(DebugDir)/*.*
	-rm -rf $(DebugDir)/database/*.*
	-rm -rf $(DebugDir)/gameLogic/*.*
	make -C $(libServerCoreDir)/linux clean
