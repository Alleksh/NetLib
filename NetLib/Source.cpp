#include "NetLib.h"
void getFunc(char*& dest)
{
	dest = new char[1024];
	memcpy(dest, std::string("getFunc").c_str(), 8);
	dest[8] = 0;
}
void sendFunc(request req)
{
	std::cout << req.Map.begin()->second << std::endl;
}
int main()
{
	NET net;
	net.addGetSendAutoFunc(getFunc, sendFunc, 1000);
	net.Initialize();
	std::this_thread::sleep_for(std::chrono::hours(1));
}