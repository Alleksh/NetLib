#include "Client.h"
std::string _Value = "";
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
int main()
{
	std::string IP;
	std::cout << "Enter IP: ";
	std::cin >> IP;
	std::cout << "Enter your name: ";
	std::cin >> _Value;
	_Value += '\n';
	NET net(IP, "27015");
	net.addGetSendAutoFunc(getFunc, sendFunc, 1000);
	net.Initialize();
	std::this_thread::sleep_for(std::chrono::hours(1));
}