#include "NetLib.h"
void getFunc(char*& dest)
{
	dest = new char[1024];
	memcpy(dest, std::string("getFunc").c_str(), 8);
	dest[8] = 0;
}
void sendFunc(Response req)
{
}
int main()
{
	NET net("localhost","27015");
	for(int i=0;i<1000;i++)
	net.addGetSendAutoFunc(getFunc,sendFunc,50);
	net.Initialize();
	std::this_thread::sleep_for(std::chrono::seconds(5));
	for (int i = 0; i < 1000; i++) net.GetThreadInteractionByID(ID(i,ID::GSRequest))->close();
	std::this_thread::sleep_for(std::chrono::hours(1));
}