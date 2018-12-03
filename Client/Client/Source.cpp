#include "Client.h"
std::string _Value = "Hello, fucking world!";
void getFunc(char*& dest)
{
	dest = new char[1024];
	memcpy(dest,_Value.c_str(), _Value.size());
	dest[_Value.size()] = 0;
}
void sendFunc(Response req)
{
	std::cout << "Answer: " << std::string(req.source) << std::endl;
}
void function()
{
	std::this_thread::sleep_for(std::chrono::seconds(5));
	NET *net;
	try
	{
		net = new NET("localhost", "27015", 0, 1);
	}
	catch (std::string)
	{
		std::cout << "Server not finded.";
		return;
	}
	for (int i = 0; i<1; i++)
		net->addGetSendAutoFunc(getFunc, sendFunc, 10);
	net->Initialize();
	std::this_thread::sleep_for(std::chrono::seconds(5));
	delete net;
}
int main()
{
	function();
	std::this_thread::sleep_for(std::chrono::hours(1));
}