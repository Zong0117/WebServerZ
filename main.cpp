#include "server/server.h"
#include <iostream>
int main()
{
	Server server(12500,3, 60000, 8);
	std::cout << "server begin" << std::endl;
	server.ServerStart();
}