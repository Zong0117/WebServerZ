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
		
![caad5675225203c66b4c79fc290d40d](https://user-images.githubusercontent.com/64644244/132852694-2ea20854-5527-413c-8e5b-6a9625318dcc.png)





编译：

	g++ -std=c++14 -O2 -Wall -g buffer/*.cpp http/*.cpp server/*.cpp timer/*.cpp main.cpp -lpthread -o myserver

