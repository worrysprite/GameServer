# GameServer
简易高效的跨平台服务器框架

windows下使用iocp<br/>
linux下使用epoll<br/>
能满足一般游戏服务器的需求<br/>

##### 目前有以下特征：
数据流方式读写socket<br/>
mysql数据库查询队列<br/>
Timer计时器<br/>

##### 即将进行的更新：
新增Actor模式，优化线程同步<br/>
新增环形队列处理socket io<br/>
mysql数据库查询改为PrepareStatement模式<br/>
处理ctrl+c等信号（或许会考虑增加输入控制）
