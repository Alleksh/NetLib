#pragma once

//#define SERVER
#ifndef SERVER
#define CLIENT
#endif
//TODO
#include <map>
#include <string>
#include <vector>
#include <thread>
#include <iostream>
#include <mutex>
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
#ifdef SERVER
struct HeaderAndResponse
{
	std::string Header;
	Response response;
	HeaderAndResponse() {}
	HeaderAndResponse(std::string Header, Response response)
	{
		this->Header = Header;
		this->response = response;
	}
};
void ProcessThread(
	std::vector<HeaderAndResponse> &AutoResponses,
	std::vector<std::pair<HeaderAndResponse, void(*)(char*&, char*)>> &AutoGetResponses,
	bool &closeThread);
#else
void AutoSendFunc(
	int& CloseThreads,
	char* Request,
	int delay,
	void(*ProcessFunc)(Response),
	void(*sendRequest)(char*&, char*&));
void GetSendAutoFunc(
	int& CloseThreads,
	int delay,
	void(*getFunc)(char*&),
	void(*ProcessFunc)(Response),
	void(*sendRequest)(char*&, char*&));
struct ThreadInteraction
{
public:
	int ptr=1, myItr;
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
		std::invoke(delFunc, myItr);
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
};
void sendRequest(char*& recvdest, char*& send);
struct ID
{
	size_t id;
	enum Type {GSRequest, Request} type = Request;
	ID(size_t id, Type t=Request)
	{
		this->id = id;
		type = t;
	}
};
#endif
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

class NET
{
private:
	WSADATA wsaData;
	struct addrinfo* addr = NULL;
	struct addrinfo hints;
	int listen_socket;
#ifdef CLIENT
public:
#endif
	NET(std::string IP, std::string PORT);
public:
#ifdef SERVER
	NET(std::string PORT) : NET("0.0.0.0", PORT)
	{

	}
	void addAutoResponse(std::string header, Response response);
	void addAutoGetResponse(std::string header, Response response, void(*getFunc)(char*&, char*));
	void Initialize();
	void Join()
	{
		thread->join();
	}
	~NET();
private:
	std::thread *thread;
	std::vector<HeaderAndResponse> AutoResponses;
	std::vector<std::pair<HeaderAndResponse, void(*)(char*&, char*)>> AutoGetResponses;
	bool closeThread = 0;
#else
	~NET()
	{
	}
	ID addSendAutoFunc(std::string Request, void(*func)(Response), int delay);
	ID addGetSendAutoFunc(void(*get)(char*&), void(*send)(Response), int delay);
	void Initialize();
	ThreadInteraction* GetThreadInteractionByID(ID id);
	void __fastcall deleteRequestsElement(size_t _Val);
	void __fastcall deleteGSRequestsElement(size_t _Val);
private:
	std::vector<RequestStruct> Requests;
	std::mutex RequestsMutex;
	std::vector <GSRequestStruct> GSRequests;
	std::mutex GSRequestsMutex;
#endif
};