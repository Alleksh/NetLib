#pragma once
//#define SERVER
#ifndef SERVER
#define CLIENT
#endif
#include <map>
#include <string>
#include <vector>
#include <thread>

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
	static void SourceToMap(std::string& source, std::map<std::string, std::string>& Map)
	{
		int i = 0;
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
};
#ifdef SERVER
#else
void AutoSendFunc(
	bool& CloseThreads,
	char* Request,
	int delay,
	void(*ProcessFunc)(request),
	void(*sendRequest)(char*&, char*))
{
	while (!CloseThreads)
	{
		char* ptr;
		sendRequest(ptr, Request);
		ProcessFunc(request(std::string(ptr)));
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		delete ptr;
	}
}
void GetSendAutoFunc(
	bool& CloseThreads,
	int delay,
	void(*getFunc)(char*&),
	void(*ProcessFunc)(request),
	void(*sendRequest)(char*&, char*))
{
	if (ProcessFunc != NULL)
	{
		while (!CloseThreads)
		{
			char *ptr, *ptr2;
			getFunc(ptr2);
			sendRequest(ptr, ptr2);
			ProcessFunc(request(std::string(ptr)));
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			delete ptr;
			delete ptr2;
		}
	}
	else
	{
		while (!CloseThreads)
		{
			char *ptr, *ptr2;
			getFunc(ptr2);
			sendRequest(ptr, ptr2);
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			delete ptr;
			delete ptr2;
		}
	}
}
#endif
void sendRequest(char*& recvdest, char*send)
{

}
class NET
{
private:
public:
#ifdef SERVER

#else
private:
	std::vector<std::pair<
		std::pair
			<std::string,
			int>,
		void(*)(request)>> Requests;
		
	
	std::vector <std::pair<
		int,
		std::pair
			<void(*)(char*&), 
			void(*)(request)>>> 
		GSRequests;
	bool CloseThreads = 0;;
public:
	void addSendAutoFunc(std::string Request, void(*func)(request), int delay)
	{
		Requests.push_back(
			std::pair<
			std::pair <std::string,int>,						void(*)(request)>
			(std::pair<std::string,int>(Request, delay),		func));
	}
	void addGetSendAutoFunc(void(*get)(char*&), void(*send)(request), int delay)
	{
		GSRequests.push_back(
			std::pair<
			int, std::pair<void(*)(char*&), void(*)(request)>>
			(delay, std::pair<void(*)(char*&), void(*)(request)>(get, send)));
	}
	void Initialize()
	{
		for (int i = 0; i < Requests.size(); i++)
		{
#define Request Requests[i].first.first
			char* request = new char[Request.size()+1];
			memcpy(request, Request.c_str(), Request.size());
			request[Request.size()] = 0;
#undef Request
		}
	}
#endif
};