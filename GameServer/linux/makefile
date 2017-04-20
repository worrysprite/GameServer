SolutionDir = ../..
ProjectDir = ..
SrcDir = $(ProjectDir)/src
DebugDir = ./Debug
libServerCoreDir = $(SolutionDir)/libServerCore
libCommonDir = $(SolutionDir)/libCommon

DEBUG = -g3 -O0 -ggdb -rdynamic
CC = ccache g++
CC_FLAGS = -DNDEBUG -std=c++11 -fstack-protector-all

INC = -I$(SrcDir) -I$(libServerCoreDir)/src -I$(libCommonDir)/src -I/usr/local/mysql/include -I/data/rapidjson/include -I/usr/local/curl/include
LIB = -L$(libServerCoreDir)/linux/Debug -L$(libCommonDir)/linux/Debug -L/usr/local/mysql/lib -L/usr/local/ssl/lib -L/usr/local/curl/lib -lServerCore -lCommon -lpthread -lmysqlclient -lrt -ldl -lssl -lcrypto -lcurl -lcares

SubPath = database
SubPath += gameLogic
SubPath += console
SubPath += network
SubPath += sdk
SubPath += upgrade

SRC_PATH = $(SrcDir)
SRC_PATH += $(addprefix $(SrcDir)/, $(SubPath))
SOURCES = $(wildcard $(addsuffix /*.cpp, $(SRC_PATH)))

ObjDir = $(DebugDir)
ObjDir += $(addprefix $(DebugDir)/, $(SubPath))

TARGET = GameServer
OBJECTS = $(patsubst $(SrcDir)/%.cpp,%.o,$(SOURCES))

$(TARGET) : ServerCore Common $(ObjDir) $(OBJECTS)
	$(CC) $(DEBUG) -o $(DebugDir)/$(TARGET) $(addprefix $(DebugDir)/,$(OBJECTS)) $(LIB)

$(OBJECTS) : %.o : $(SrcDir)/%.cpp
	$(CC) $(DEBUG) $(INC) -c $(CC_FLAGS) $< -o $(DebugDir)/$@

$(ObjDir) :
	-mkdir $@

ServerCore :
	make -C $(libServerCoreDir)/linux

Common :
	make -C $(libCommonDir)/linux

.PRONY : clean
clean:
	-rm -rf $(DebugDir)
	make -C $(libServerCoreDir)/linux clean
	make -C $(libCommonDir)/linux clean
