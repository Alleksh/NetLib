#include "Server.h"
void getFunc(std::stringstream& val, char*& _value)
{


}
int main()
{
	NET net("27015");
	net.addAutoGetResponse("Header", getFunc);
	net.Initialize();
	net.Join();
	std::this_thread::sleep_for(std::chrono::hours(1));
}