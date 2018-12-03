#include "Client.h"
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
NET::NET(std::string IP, std::string PORT, size_t FixedRequestsSize, size_t FixedGSRequestsSize)
{
	Requests.reserve(FixedRequestsSize);
	GSRequests.reserve(FixedGSRequestsSize);
	this->IP = IP;
	this->PORT = PORT;
	int iResult;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		std::cerr << "WSAStartup error: " << iResult;
		throw "WSAStartup error";
		return;
	}


	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		std::cerr << "WSAStartup error: " << iResult;
		throw "WSAStartup error";
		return;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(IP.c_str(), PORT.c_str(), &hints, &result);
	if (iResult != 0)
	{
		std::cerr << "getaddrinfo error: " << iResult;
		WSACleanup();
		throw "SERVER_NOT_FINDED";
		return;
	}
}
bool sendRequest(
	std::string IP,
	std::string PORT,
	struct addrinfo& hints,
	struct addrinfo *&result, char*& recvbuf, char*& sendbuf)
{
	int iResult;
	recvbuf = new char[1024];
	// Resolve the server address and port
	iResult = getaddrinfo(IP.c_str(), PORT.c_str(), &hints, &result);
	if (iResult != 0)
	{
		std::cerr << "getaddrinfo error: " << iResult;
		WSACleanup();
		return 0;
	}
	SOCKET ConnectSocket;
	// Attempt to connect to an address until one succeeds
	for (struct addrinfo *ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET)
		{
			std::cerr << "socket error: ";
			WSACleanup();
			return 0;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	if (ConnectSocket == INVALID_SOCKET)
	{
		std::cerr << "Unable to connect to server!\n";
		WSACleanup();
		return 0;
	}

	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 0;
	}

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 0;
	}

	// Receive until the peer closes the connection
	do {
		iResult = recv(ConnectSocket, recvbuf, PACKAGE_SIZE, 0);
	} while (iResult > 0);
	closesocket(ConnectSocket);
	return 1;
}
void AutoSendFunc(
	std::string IP,
	std::string PORT,
	struct addrinfo& hints,
	struct addrinfo *&result,
	int& CloseThreads,
	char* Request,
	int delay,
	void(*ProcessFunc)(Response),
	SEND_REQUEST_MACRO)
{
	while (CloseThreads != 0)
	{
		while (CloseThreads == 2)
			std::this_thread::sleep_for(std::chrono::milliseconds(150));

		char* ptr;
		sendRequest(IP, PORT, hints,result, ptr, Request);
		ProcessFunc(Response(std::string(ptr)));
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		delete ptr;
	}
	delete Request;
	CloseThreads = 3;
}
void GetSendAutoFunc(
	std::string IP,
	std::string PORT,
	struct addrinfo& hints,
	struct addrinfo *&result,
	int& CloseThreads,
	int delay,
	void(*getFunc)(char*&),
	void(*ProcessFunc)(Response),
	SEND_REQUEST_MACRO)
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
			if(!sendRequest(IP, PORT, hints, result, ptr, ptr2))
			{
				delete ptr;
				delete ptr2;
				return;
			}
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
			sendRequest(IP, PORT, hints,result, ptr, ptr2);
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
			delete ptr;
			delete ptr2;
		}
	}
	CloseThreads = 3;
}

ThreadInteraction* NET::addSendAutoFunc(std::string Request, void(*func)(Response), int delay)
{
	RequestStruct str;
	str.TI.myItr = Requests.size();
	str.TI.delFunc = std::bind(&NET::deleteRequestsElement, this, std::placeholders::_1);
	str.Request = Request;
	str.func = func;
	str.delay = delay;
	Requests.push_back(str);
	return &Requests[Requests.size() - 1].TI;
}
ThreadInteraction* NET::addGetSendAutoFunc(void(*get)(char*&), void(*send)(Response), int delay)
{
	GSRequestStruct str;
	str.TI.myItr = GSRequests.size();
	str.TI.delFunc = std::bind(&NET::deleteGSRequestsElement, this, std::placeholders::_1);
	str.getFunc = get;
	str.sendFunc = send;
	str.delay = delay;
	if (1)
	{
		GSRequests.push_back(str);
	}
	return &GSRequests[GSRequests.size() - 1].TI;
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
			IP,
			PORT,
			std::ref(hints),
			std::ref(result),
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
			IP,
			PORT,
			std::ref(hints),
			std::ref(result),
			std::ref(GSRequests[i].TI.ptr),
			GSRequests[i].delay,
			GSRequests[i].getFunc,
			GSRequests[i].sendFunc,
			&sendRequest
		);
		thread.detach();
	}
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
	for (size_t i = 0; i < GSRequests.size(); i++)
		GSRequests[i].TI.myItr=i;// or Requests[i].TI.myItr--;
	GSRequestsMutex.unlock();
}