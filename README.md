# GameServer
简单易用的跨平台网络库

windows下使用iocp  
linux下使用epoll ET模式  
能满足简单游戏服务器的需求  
需要以下库支持：  
**[mysql-connector](https://www.mysql.com/products/connector/)**  
**[openssl](https://www.openssl.org/)**  
**[libcurl](https://curl.haxx.se/libcurl/)**

--------
#### 2017/04/20更新：
* 新增了客户端Socket的实现（`ClientSocket`类），现在可以通过它与`ServerSocket`结合做多进程服务器了
* 新增了http客户端的实现（需要[libcurl](https://curl.haxx.se/libcurl/)库），以队列的形式处理简单的http请求
* 优化了DBStatement的`<<`操作符，使得更容易绑定字符串（使用`std::string`）和Blob字段了（使用`ByteArray`）
* 优化了DBStatement的`>>`操作符，使得更容易读取Blob字段了（使用`ByteArray`）
* 优化了Recordset的`>>`操作符，使得更容易读取Blob字段了（使用`ByteArray`）
* 优化了事件的回调函数，使用C++11标准的`std::function`定义`EventCallback`
* 原`ClientSocket`类更名为`Client`，给`Client`新增了`onDisconnected`和`update`函数供重载，其中`onDisconnected`会在客户端断开连接时调用，`update`则会在每次`ServerSocket`的`update`时（通常是主循环）调用
* 将判定不活跃的客户端时使用的时间改为了`GetTickCount64`，不会因为修改服务器时间导致客户端意外掉线（游戏测试常改时间）
* 优化了`ServerSocket`的退出和析构，使之释放更全面
* 修复了`ServerSocket`在windows下给`Client`的ip地址不正确
* `ByteArray`新增了toHexString方便把字节转化成16进制字符串，优化了`ByteArray`的`resize`和`truncate`方法
* 新增工具类`TimeTool`，提供常用的方法如获取unix时间戳、获取今天（或其他时间）的凌晨0点整的时间戳、获取本月（或其他时间）的第一天凌晨0点整的时间戳、判断一个时间是否昨天或更早以前
* 修复一个`Timer`计时器会引起崩溃的BUG
* 丰富`String`类的工具函数，如`random`随机生成字符串
* 新增工具类`Math`，使用C++11标准库的梅森旋转算法生成随机数；新增平面向量`Vector2D`类
* 优化`Log`类使用的时间函数，避免使用`ctime`或`localtime`等不安全的C API
* 提供简单的示例工程GameServer以及相关第三方库，vs2013环境已编译测试通过

-------
#### 目前有以下特征：
数据流方式读写socket  
使用`ByteArray`的`<<`和`>>`操作符方便快捷地读写二进制数据  
mysql数据库查询队列，支持statement查询方式，缓存已prepare过的语句提高执行效率  
高效易用的计时器，使用C++11的新特性`std::bind`和`std::function`作为计时回调  


#### 即将进行的更新：
实现一个封闭式数据流替代`ServerSocket`中的`ByteArray`，因为`ByteArray`为了提供内部实际控制的内存指针给外部访问，频繁进行内存复制和移动
实现一个回环容器
优化`String`类的一些工具方法，减少对第三方库的依赖

-------
此版本作为GameServer-v1封存，发布版在v1.0.0 tag
GameServer-v2将以asio作为网络库，使用最新的C++20协程来实现