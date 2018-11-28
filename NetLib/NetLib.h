#pragma once
//#define SERVER
#ifndef SERVER
#define CLIENT
#endif
#include <map>
#include <string>
#include <vector>
#include <thread>
#include <iostream>
#include <mutex>
//структура дл€ запросов
struct request
{
	std::string source;
	std::map<std::string, std::string> Map;
	request(std::string source)
	{
		this->source = source;
		SourceToMap(source, Map);
	}
	request() {}
	static void SourceToMap(std::string& source, std::map<std::string, std::string>& Map);
};
#ifdef SERVER
#else
void AutoSendFunc(
	int* CloseThreads,
	char* Request,
	int delay,
	void(*ProcessFunc)(request),
	void(*sendRequest)(char*&, char*));
void GetSendAutoFunc(
	int* CloseThreads,
	int delay,
	void(*getFunc)(char*&),
	void(*ProcessFunc)(request),
	void(*sendRequest)(char*&, char*));
// —труктура дл€ взаимодействи€ с потоками
struct ThreadInteraction
{
public:
	int ptr, myItr;
	//(void)(NET::*deleteFunc)(unsigned int);
	ThreadInteraction(){}
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
		std::this_thread::sleep_for(std::chrono::seconds(1));
		//(*net).(*deleteFunc)(myItr);
	}
	int state()
	{
		return ptr;
	}
};
struct ThreadInteraction2
{
private:
	ThreadInteraction *ti;
public:
	void pause()
	{
		ti->ptr = 2;
	}
	void unPause()
	{
		ti->ptr = 1;
	}
	void close()
	{
		ti->close();
	}
	int state()
	{
		return ti->ptr;
	}
	ThreadInteraction2(ThreadInteraction& ti)
	{
		this->ti = &ti;
	}
};
struct RequestStruct
{
	ThreadInteraction TI;
	std::string Request;
	void(*func)(request);
	int delay;
	RequestStruct() {}
};
struct GSRequestStruct
{
	ThreadInteraction TI;
	void(*getFunc)(char*&);
	void(*sendFunc)(request);
	int delay;
	GSRequestStruct() {}
};
#endif
void sendRequest(char*& recvdest, char*send);
class NET
{
private:
public:
#ifdef SERVER

#else
private:
	std::vector<RequestStruct> Requests;
	std::mutex RequestsMutex;
	void deleteRequestsElement(unsigned int _Val)
	{
		if (_Val >= Requests.size()) return;
		Requests.erase(Requests.begin() + _Val);
		for (unsigned int i = _Val; i < Requests.size(); i++)
			Requests[i].TI.myItr = i;// or Requests[i].TI.myItr--;
	}
	std::vector <GSRequestStruct> GSRequests;
	std::mutex GSRequestsMutex;
	void deleteGSRequestsElement(unsigned int _Val)
	{
		if (_Val >= GSRequests.size()) return;
		GSRequests.erase(GSRequests.begin() + _Val);
		for (unsigned int i = _Val; i < Requests.size(); i++)
			GSRequests[i].TI.myItr = i;// or Requests[i].TI.myItr--;
	}
public:
	~NET()
	{
	}
	ThreadInteraction2* addSendAutoFunc(std::string Request, void(*func)(request), int delay);
	ThreadInteraction2* addGetSendAutoFunc(void(*get)(char*&), void(*send)(request), int delay);
	//»нициализировать потоки
	void Initialize();
#endif
};