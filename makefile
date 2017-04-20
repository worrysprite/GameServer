SolutionDir = .
GameServerDir = ./GameServer
ConsoleDir = ./Console
CrossBattleDir = ./CrossBattleServer
DebugDir = ./Debug
libServerCoreDir = ./libServerCore

Target : 
	make -C $(GameServerDir)/linux
	\cp $(GameServerDir)/linux/Debug/GameServer $(DebugDir)/GameServer

console :
	make -C $(ConsoleDir)/linux
	\cp $(ConsoleDir)/linux/Debug/Console $(DebugDir)/Console

crossbattleserver :
	make -C $(CrossBattleDir)/linux
	\cp $(CrossBattleDir)/linux/Debug/CrossBattleServer $(DebugDir)/CrossBattleServer

.PRONY : clean
clean :
	make -C $(GameServerDir)/linux clean
	make -C $(ConsoleDir)/linux clean
	make -C $(CrossBattleDir)/linux clean

.PRONY : run
run : Target
	cd $(DebugDir)
	nohup ./GameServer > output.log 2>&1 &
