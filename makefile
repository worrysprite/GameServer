SolutionDir = .
ProjectDir = ./GameServer
DebugDir = ./Debug
libServerCoreDir = ./libServerCore

Target : Force
	make -C $(ProjectDir)/linux
	\cp $(ProjectDir)/linux/Debug/GameServer $(DebugDir)/GameServer

Force :

.PRONY : clean
clean :
	-rm -rf $(DebugDir)/*
	make -C $(ProjectDir)/linux clean

.PRONY : run
run : Target
	nohup $(DebugDir)/GameServer > $(DebugDir)/output.log 2>&1 &
