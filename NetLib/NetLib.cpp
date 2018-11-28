#include "NetLib.h"
void request::SourceToMap(std::string& source, std::map<std::string, std::string>& Map)
{
	unsigned int i = 0;
	while (i < source.size())
	{
		std::pair<std::string, std::string> pair;
		while (i < source.size() && source[i] != ':')
		{
			pair.first += source[i];
			i++;
		}
		i++;
		while (i < source.size() && source[i] != '\n')
		{
			pair.second += source[i];
			i++;
		}i++;
		Map.emplace(pair.first, pair.second);
	}
}
#ifdef SERVER
#else
void AutoSendFunc(
	int* CloseThreads,
	char* Request,
	int delay,
	void(*ProcessFunc)(request),
	void(*sendRequest)(char*&, char*))
{
	while ((*CloseThreads) != 0)
	{
		while ((*CloseThreads) == 2)
			std::this_thread::sleep_for(std::chrono::milliseconds(150));

		char* ptr;
		sendRequest(ptr, Request);
		ProcessFunc(request(std::string(ptr)));
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		delete ptr;
	}
}
void GetSendAutoFunc(
	int* CloseThreads,
	int delay,
	void(*getFunc)(char*&),
	void(*ProcessFunc)(request),
	void(*sendRequest)(char*&, char*))
{
	if (ProcessFunc != NULL)
	{
		while ((*CloseThreads) != 0)
		{
			while ((*CloseThreads) == 2)
				std::this_thread::sleep_for(std::chrono::milliseconds(150));

			char *ptr, *ptr2;
			getFunc(ptr2);
			sendRequest(ptr, ptr2);
			ProcessFunc(request(std::string(ptr)));
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
			delete ptr;
			delete ptr2;
		}
	}
	else
	{
		while ((*CloseThreads) != 0)
		{
			while ((*CloseThreads) == 2)
				std::this_thread::sleep_for(std::chrono::milliseconds(150));

			char *ptr, *ptr2;
			getFunc(ptr2);
			sendRequest(ptr, ptr2);
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
			delete ptr;
			delete ptr2;
		}
	}
}

ThreadInteraction2* NET::addSendAutoFunc(std::string Request, void(*func)(request), int delay)
{
	RequestStruct str;
	str.TI.myItr = Requests.size();
	//str.TI.deleteFunc = deleteRequestsElement;
	str.Request = Request;
	str.func = func;
	str.delay = delay;
	Requests.push_back(str);
	ThreadInteraction2* returnVal = new ThreadInteraction2(str.TI);
	return returnVal;
}

ThreadInteraction2* NET::addGetSendAutoFunc(void(*get)(char*&), void(*send)(request), int delay)
{
	GSRequestStruct str;
	str.TI.myItr = GSRequests.size();
	//str.TI.deleteFunc = deleteGSRequestsElement;
	str.getFunc = get;
	str.sendFunc = send;
	str.delay = delay;
	GSRequests.push_back(str);
	ThreadInteraction2* returnVal = new ThreadInteraction2(str.TI);
	return returnVal;
}

void NET::Initialize()
{
	for (unsigned int i = 0; i < Requests.size(); i++)
	{
		char* request = new char[Requests[i].Request.size() + 1];
		memcpy(request, Requests[i].Request.c_str(), Requests[i].Request.size());
		request[Requests[i].Request.size()] = 0;

		std::thread thread
		(
			AutoSendFunc,
			&Requests[i].TI.ptr,
			request,
			Requests[i].delay,
			Requests[i].func,
			&sendRequest
		);
		thread.detach();
	}
	for (unsigned int i = 0; i < GSRequests.size(); i++)
	{
		std::thread thread
		(
			GetSendAutoFunc,
			&GSRequests[i].TI.ptr,
			GSRequests[i].delay,
			GSRequests[i].getFunc,
			GSRequests[i].sendFunc,
			&sendRequest
		);
		thread.detach();
	}
}
#endif
void sendRequest(char*& recvdest, char*send)
{
	recvdest = new char[1024];
	memcpy(recvdest, std::string("123:123").c_str(), 8);
	recvdest[8] = 0;
	std::cout << send << " sended." << std::endl;
}