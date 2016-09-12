# GameServer
简易高效的跨平台网络库

windows下使用iocp<br/>
linux下使用epoll ET模式<br/>
能满足简单游戏服务器的需求<br/>

##### 目前有以下特征：
数据流方式读写socket<br/>
使用ByteArray的输入(<<)输出(>>)操作符方便快捷地序列化或解析数据<br/>
mysql数据库查询队列，支持statement查询方式，缓存已prepare过的语句提高执行效率<br/>
高效易用的Timer计时器，使用C++11的std::bind和function新特性作为计时回调<br/>

##### 即将进行的更新：
处理ctrl+c等信号（或许会考虑增加输入控制）
新增更多的工具方法