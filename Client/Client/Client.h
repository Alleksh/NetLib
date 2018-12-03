#pragma once
#include <map>
#include <string>
#include <vector>
#include <thread>
#include <iostream>
#include <mutex>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#define PACKAGE_SIZE 1024
#define SEND_REQUEST_MACRO bool (*sendRequest)(std::string IP,std::string PORT,struct addrinfo& hints,struct addrinfo *&result, char*&, char*&)
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
void AutoSendFunc(
	std::string IP, 
	std::string PORT,
	struct addrinfo& hints,
	struct addrinfo *&result,
	int& CloseThreads,
	char* Request,
	int delay,
	void(*ProcessFunc)(Response),
	SEND_REQUEST_MACRO);
void GetSendAutoFunc(
	std::string IP,
	std::string PORT,
	struct addrinfo& hints,
	struct addrinfo *&result,
	int& CloseThreads,
	int delay,
	void(*getFunc)(char*&),
	void(*ProcessFunc)(Response),
	SEND_REQUEST_MACRO);
struct ThreadInteraction
{
public:
	int ptr = 1;
	size_t myItr;
	std::function<void(size_t)> delFunc;
	ThreadInteraction() {}
	void pause()
	{
		ptr = 2;
	}
	void unPause()
	{
		ptr = 1;
	}
	void close()
	{
		ptr = 0;
		delFunc(myItr);
	}
	int state()
	{
		return ptr;
	}
};
struct RequestStruct
{
	ThreadInteraction TI;
	std::string Request;
	void(*func)(Response);
	int delay;
	RequestStruct() {}
};
struct GSRequestStruct
{
	ThreadInteraction TI;
	void(*getFunc)(char*&);
	void(*sendFunc)(Response);
	int delay;
	GSRequestStruct() {}
}; bool sendRequest(
	std::string IP,
	std::string PORT,
	struct addrinfo& hints,
	struct addrinfo *&result, char*& recvbuf, char*& sendbuf);
class NET
{
public:
	NET(std::string, std::string, size_t, size_t);
	~NET()
	{
		for (int i = 0; i < Requests.size(); i++)
		{
			Requests[i].TI.close();
		}
		for (int i = 0; i < GSRequests.size(); i++)
		{
			GSRequests[i].TI.close();
		}
		WSACleanup();
	}
	ThreadInteraction* addSendAutoFunc(std::string Request, void(*func)(Response), int delay);
	ThreadInteraction* addGetSendAutoFunc(void(*get)(char*&), void(*send)(Response), int delay);
	void Initialize();
	void __fastcall deleteRequestsElement(size_t _Val);
	void __fastcall deleteGSRequestsElement(size_t _Val);
private:
	WSADATA wsaData;
	struct addrinfo hints;
	struct addrinfo *result = NULL;
	std::string IP;
	std::string PORT;
	std::vector<RequestStruct> Requests;
	std::mutex RequestsMutex;
	std::vector <GSRequestStruct> GSRequests;
	std::mutex GSRequestsMutex;
};