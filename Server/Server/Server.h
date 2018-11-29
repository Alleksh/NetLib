#pragma once
#include <map>
#include <string>
#include <vector>
#include <thread>
#include <iostream>
#include <mutex>
#include <sstream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#define PACKAGE_SIZE 1024
struct Response
{
	std::string source;
	std::map<std::string, std::string> Map;
	Response(std::string source)
	{
		this->source = source;
		SourceToMap(source, Map);
	}
	Response() {}
	static void SourceToMap(std::string& source, std::map<std::string, std::string>& Map);
};
struct Request
{
	char
		*source,
		*header,
		*body;
	Request(char *source)
	{
		this->source = source;
		int i = 0;
		std::string _Value;
		for (; i < PACKAGE_SIZE; i++)
			if (source[i] == '\n') break;
			else _Value+= source[i];
		i++;
		header = new char[_Value.size()+1];
		header[_Value.size()] = 0;
		memcpy(header, _Value.c_str(), _Value.size());
		if((PACKAGE_SIZE - _Value.size())>0)
		{
			body = new char[PACKAGE_SIZE - _Value.size()];

			memcpy(body, source+_Value.size(), PACKAGE_SIZE - _Value.size());
		}
		else body = new char[1];
	}
	~Request()
	{
		delete header;
		delete body;
	}
};
struct HeaderAndResponse
{
	std::string header;
	std::string response;
	HeaderAndResponse() {}
	HeaderAndResponse(std::string header, std::string response)
	{
		this->header = header;
		this->response = response;
	}
};
void ProcessThread(
	WSADATA &wsaData,
	struct addrinfo *&addr,
	struct addrinfo &hints,
	int &listen_socket,
	std::vector<HeaderAndResponse> &AutoResponses,
	std::vector<std::pair<std::string, void(*)(std::stringstream&, char*&)>> &AutoGetResponses,
	bool &closeThread);

class NET
{
private:
	WSADATA wsaData;
	struct addrinfo* addr = NULL;
	struct addrinfo hints;
	int listen_socket;
	NET(std::string IP, std::string PORT);
public:
	NET(std::string PORT) : NET("0.0.0.0", PORT)
	{

	}
	void addAutoResponse(std::string header, std::string response);
	void addAutoGetResponse(std::string header, void(*getFunc)(std::stringstream&, char*&));
	void Initialize();
	void Join()
	{
		thread->join();
	}
	~NET();
private:
	std::thread *thread;
	std::vector<HeaderAndResponse> AutoResponses;
	std::vector<std::pair<std::string, void(*)(std::stringstream&, char*&)>> AutoGetResponses;
	bool closeThread = 0;
};