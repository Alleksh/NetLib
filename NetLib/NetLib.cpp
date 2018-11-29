#include "NetLib.h"
void Response::SourceToMap(std::string& source, std::map<std::string, std::string>& Map)
{
	size_t i = 0;
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
NET::NET(std::string IP, std::string PORT)
{
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		std::cerr << "WSAStartup failed: " << result << "\n";
		throw "WSAStartup failed";
		return;
	}
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;


	result = getaddrinfo(IP.c_str(), PORT.c_str(), &hints, &addr);
	if (result != 0)
	{
		std::cerr << "getaddrinfo failed: " << result << "\n";
		WSACleanup();
		throw "getaddrinfo failed";
		return;
	}


	listen_socket = socket(addr->ai_family, addr->ai_socktype,
		addr->ai_protocol);
	if (listen_socket == INVALID_SOCKET)
	{
		std::cerr << "Error at socket: " << WSAGetLastError() << "\n";
		freeaddrinfo(addr);
		WSACleanup();
		throw "error at socket.";
		return;
	}

	result = bind(listen_socket, addr->ai_addr, (int)addr->ai_addrlen);

	if (result == SOCKET_ERROR)
	{
		std::cerr << "bind failed with error: " << WSAGetLastError() << "\n";
		freeaddrinfo(addr);
		closesocket(listen_socket);
		WSACleanup();
		throw "socket bind failed";
		return;
	}

	if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << "listen failed with error: " << WSAGetLastError() << "\n";
		closesocket(listen_socket);
		WSACleanup();
		return;
	}
	}

#ifdef SERVER
void ProcessThread(
	std::vector<HeaderAndResponse> &AutoResponses,
	std::vector<std::pair<HeaderAndResponse, void(*)(char*&, char*)>> &AutoGetResponses,
	bool &closeThread)
{
	while (1)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "Hello, world\n";
	}
}
void NET::addAutoResponse(std::string header, Response response)
{
	AutoResponses.push_back(HeaderAndResponse(header, response));
}
void NET::addAutoGetResponse(std::string header, Response response, void(*getFunc)(char*&, char*))
{
	AutoGetResponses.push_back(std::pair<HeaderAndResponse, void(*)(char*&, char*)>(HeaderAndResponse(header, response), getFunc));
}
void NET::Initialize()
{
	thread = new std::thread(ProcessThread, std::ref(AutoResponses), std::ref(AutoGetResponses), std::ref(closeThread));
}
NET::~NET()
{
	closeThread = 1;
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
}
#else

void sendRequest(char*& recvdest, char*& send)
{
	recvdest = new char[1024];
	memcpy(recvdest, std::string("123:123").c_str(), 8);
	recvdest[8] = 0;
}
void AutoSendFunc(
	int& CloseThreads,
	char* Request,
	int delay,
	void(*ProcessFunc)(Response),
	void(*sendRequest)(char*&, char*&))
{
	while (CloseThreads != 0)
	{
		while (CloseThreads == 2)
			std::this_thread::sleep_for(std::chrono::milliseconds(150));

		char* ptr;
		sendRequest(ptr, Request);
		ProcessFunc(Response(std::string(ptr)));
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		delete ptr;
	}
	delete Request;
	CloseThreads = 3;
}
void GetSendAutoFunc(
	int& CloseThreads,
	int delay,
	void(*getFunc)(char*&),
	void(*ProcessFunc)(Response),
	void(*sendRequest)(char*&, char*&))
{
	if (ProcessFunc != NULL)
	{
		while (CloseThreads != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
			while (CloseThreads == 2)
				std::this_thread::sleep_for(std::chrono::milliseconds(50));

			char *ptr, *ptr2;
			getFunc(ptr2);
			sendRequest(ptr, ptr2);
			ProcessFunc(Response(std::string(ptr)));
			delete ptr;
			delete ptr2;
		}
	}
	else
	{
		while (CloseThreads != 0)
		{
			while (CloseThreads == 2)
				std::this_thread::sleep_for(std::chrono::milliseconds(150));

			char *ptr, *ptr2;
			getFunc(ptr2);
			sendRequest(ptr, ptr2);
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
			delete ptr;
			delete ptr2;
		}
	}
	CloseThreads = 3;
}

ID NET::addSendAutoFunc(std::string Request, void(*func)(Response), int delay)
{
	RequestStruct str;
	str.TI.myItr = Requests.size();
	str.TI.delFunc = &(this->deleteRequestsElement);
	str.Request = Request;
	str.func = func;
	str.delay = delay;
	Requests.push_back(str);
	return ID(Requests.size()-1);
}
ID NET::addGetSendAutoFunc(void(*get)(char*&), void(*send)(Response), int delay)
{
	GSRequestStruct str;
	str.TI.myItr = GSRequests.size();
	str.TI.delFunc = &deleteGSRequestsElement;
	str.getFunc = get;
	str.sendFunc = send;
	str.delay = delay;
	if(1)
	{
		GSRequests.push_back(str);
	}
	return ID(Requests.size() - 1, ID::GSRequest);
}

void NET::Initialize()
{
	for (size_t i = 0; i < Requests.size(); i++)
	{
		char* Response = new char[Requests[i].Request.size() + 1];
		memcpy(Response, Requests[i].Request.c_str(), Requests[i].Request.size());
		Response[Requests[i].Request.size()] = 0;

		std::thread thread
		(
			AutoSendFunc,
			std::ref(Requests[i].TI.ptr),
			Response,
			Requests[i].delay,
			Requests[i].func,
			&sendRequest
		);
		thread.detach();
	}
	for (size_t i = 0; i < GSRequests.size(); i++)
	{
		std::thread thread
		(
			GetSendAutoFunc,
			std::ref(GSRequests[i].TI.ptr),
			GSRequests[i].delay,
			GSRequests[i].getFunc,
			GSRequests[i].sendFunc,
			&sendRequest
		);
		thread.detach();
	}
}
ThreadInteraction* NET::GetThreadInteractionByID(ID id)
{
	if (id.type == ID::Request)
		return &Requests[id.id].TI;
	else
		return &GSRequests[id.id].TI;
}
void NET::deleteRequestsElement(size_t _Val)
{
	if (_Val >= Requests.size()) return;
	RequestsMutex.lock();
	Requests.erase(Requests.begin() + _Val);
	for (size_t i = _Val; i < Requests.size(); i++)
		Requests[i].TI.myItr = i;// or Requests[i].TI.myItr--;
	RequestsMutex.unlock();
}
void NET::deleteGSRequestsElement(size_t _Val)
{
	if (_Val >= GSRequests.size()) return;
	GSRequestsMutex.lock();
	GSRequests.erase(GSRequests.begin() + _Val);
	for (size_t i = _Val; i < Requests.size(); i++)
		GSRequests[i].TI.myItr = i;// or Requests[i].TI.myItr--;
	GSRequestsMutex.unlock();
}
#endif