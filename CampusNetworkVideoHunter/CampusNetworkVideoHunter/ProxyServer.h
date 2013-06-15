#include <Windows.h>
#include <process.h>
#include <string>
#include <iostream>


#pragma comment(lib,"Ws2_32.lib")
using namespace std;

typedef struct _WORKPARAM{
	SOCKET		sckClient;
	sockaddr_in	client_addr;
}WORKPARAM;

typedef struct _client_request_summary
{
	string type;
	string url;
	string host;
	string range;
}client_request_summary;

long GetContentLength(string *m_ResponseHeader);

bool AnalyzeClientRequest(string *client_request,client_request_summary *crs);

void WorkThread(void *pvoid);

void ListenThread(void *pvoid);
