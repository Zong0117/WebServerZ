# WebServerZ
C++实现高并发web服务器

		·使用非阻塞I/O，I/O多路复用(epoll)，线程池实现的Reactor服务器模型

		·实现计时器管理超时连接

		·实现自增长的buffer

		·实现HTTP get请求



环境：

		ubantu

		c++14


测压：

		webbench -c 1000 -t 10 http://ip:port/
![35ca71105162eaa65c604996a65b98c](https://user-images.githubusercontent.com/64644244/132851447-c284d3f3-698e-411d-826d-6709d5952624.jpg)



编译：

	g++ -std=c++14 -O2 -Wall -g buffer/*.cpp http/*.cpp server/*.cpp timer/*.cpp main.cpp -lpthread -o myserver

