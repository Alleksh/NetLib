#include "Server.h"
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


void ProcessThread(
	WSADATA &wsaData,
	struct addrinfo *&addr,
	struct addrinfo &hints,
	int &listen_socket,
	std::vector<HeaderAndResponse> &AutoResponses,
	std::vector<std::pair<std::string, void(*)(std::stringstream&, char*&)>> &AutoGetResponses,
	bool &closeThread)
{
	while (1)
	{
		int result;
		char *buf = new char[PACKAGE_SIZE];
		int client_socket = INVALID_SOCKET;
		for (;;) 
		{
			client_socket = accept(listen_socket, NULL, NULL);

			if (client_socket == INVALID_SOCKET) 
			{
				std::cerr << "accept failed: " << WSAGetLastError() << "\n";
				closesocket(listen_socket);
				WSACleanup();
				throw "SOCKET_ERROR";
			}
			result = recv(client_socket, buf, PACKAGE_SIZE, 0);

			if (result == SOCKET_ERROR)
			{
				std::cerr << "recv failed: " << result << "\n";
				closesocket(client_socket);
			}
			else if (result == 0) 
			{
				std::cerr << "connection closed...\n";
			}
			else if (result > 0) 
			{
				buf[result] = '\0'; 
				std::cout << buf;
				std::stringstream response;
				Request _Value(buf);
				bool finded = 0;
				for (size_t i = 0; i < AutoResponses.size(); i++)
				{
					if (AutoResponses[i].header == _Value.header)
					{
						response << AutoResponses[i].response;
						finded = 1;
						break;
					}
				}
				if (!finded)
				for (size_t i = 0; i < AutoGetResponses.size(); i++)
				{
					if (AutoGetResponses[i].first == _Value.header)
					{
						AutoGetResponses[i].second(response,buf);
						finded = 1;
						break;
					}
				}
				if (!finded) response << "UNKNOWN_HEADER_ERROR";
				result = send(client_socket, response.str().c_str(),
					response.str().size(), 0);
				if (result == SOCKET_ERROR) 
				{
					std::cerr << "send failed: " << WSAGetLastError() << "\n";
				}
				closesocket(client_socket);
			}
		}

		closesocket(listen_socket);
		freeaddrinfo(addr);
		WSACleanup();
	}
}
void NET::addAutoResponse(std::string header, std::string response)
{
	AutoResponses.push_back(HeaderAndResponse(header, response));
}
void NET::addAutoGetResponse(std::string header, void(*getFunc)(std::stringstream&, char*&))
{
	AutoGetResponses.push_back(std::pair<std::string, void(*)(std::stringstream&, char*&)>(header, getFunc));
}
void NET::Initialize()
{
	thread = new 
		std::thread(ProcessThread, 
			std::ref(wsaData),
			std::ref(addr),
			std::ref(hints),
			std::ref(listen_socket),
			std::ref(AutoResponses), 
			std::ref(AutoGetResponses), 
			std::ref(closeThread));
}
NET::~NET()
{
	closeThread = 1;
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
}