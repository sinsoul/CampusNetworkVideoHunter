#include "Mystd.h"
#include "Transcoding.h"
#include <WindowsX.h>
#include<fstream>

#define TRANSPARENTCOLOR RGB(254,253,251)
#define CURRENT_VERSION 10
#define VERSION_STRING	"Ver 2.1.1214"
char *server_addr="210.41.192.177";
char CustomIP[20];
char szClassName[21] = "CNVH_SinSoul";
char *wndName="西华师范校园网视频猎手 - "VERSION_STRING"  - SinSoul";
string conf_path;
HWND hwndButton = 0;
HWND hwndEdit = 0; 
HWND hwndLable_0 = 0,hwndLable_1=0;
HWND hListView=0;
HWND hwnd=0; 
HINSTANCE hinst;
bool b_Search=false,b_Downloading=false,b_Proxy=false;
int totalresult=0;
int selected=-1;//当前选择的列表行号
string *TotalXML;
WNDPROC OldEditProc;
Gdiplus::GdiplusStartupInput gdiplusStartupInput;
ULONG gdiplusToken;

typedef struct _movinfo
{
	string mov_name;
	string mov_link;
}movinfo;

list<movinfo> MovList;
list<movinfo>::iterator i_ML;

int WINAPI WinMain (HINSTANCE hThisInstance,HINSTANCE hPrevInstance,LPSTR lpszArgument,int nFunsterStil)
{ 
	MSG messages;
	WNDCLASSEX wincl;
	hinst=hThisInstance;
	wincl.hInstance = hThisInstance;
	wincl.lpszClassName = szClassName;
	wincl.lpfnWndProc = WindowProcedure;
	wincl.style = CS_DBLCLKS;
	wincl.cbSize = sizeof (WNDCLASSEX);
	wincl.hIcon = LoadIcon (NULL,MAKEINTRESOURCE(IDI_ICON1));
	wincl.hIconSm = (HICON)LoadImage(hThisInstance,MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR);
	wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
	wincl.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wincl.cbClsExtra = 0;
	wincl.cbWndExtra = 0;

	if (EnableAero(NULL))
	{
		LOGBRUSH Lb;
		Lb.lbStyle = BS_SOLID;
		Lb.lbColor = TRANSPARENTCOLOR;
		wincl.hbrBackground =CreateBrushIndirect(&Lb);
	}
	else
	{
		wincl.hbrBackground =(HBRUSH)4;
	}

	InitCommonControls();

	if (!RegisterClassEx (&wincl))
	{
		return 0;
	}

	hwnd = CreateWindowEx (NULL,szClassName,wndName, 
		WS_OVERLAPPED|WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,
		720,520,HWND_DESKTOP,NULL,hThisInstance,NULL);

	hwndButton=CreateWindowEx(NULL,TEXT("BUTTON"),TEXT("捕获"),
		WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON|BS_PUSHBUTTON
		,370,28,80,25,hwnd,(struct HMENU__ *)1,hThisInstance,NULL);

	hwndEdit=CreateWindowEx(NULL,TEXT("EDIT"),TEXT(""),
		WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,
		60,30,300,20,hwnd,(struct HMENU__ *)0,hThisInstance,NULL);
	
	hwndLable_0=CreateWindowEx(NULL,TEXT("EDIT"),TEXT(""),
		WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_READONLY,
		15,250,690,200,hwnd,(struct HMENU__ *)0,hThisInstance,NULL);

	hwndLable_1=CreateWindowEx(NULL,TEXT("EDIT"),TEXT(""),
		WS_CHILD|WS_VISIBLE|ES_READONLY|ES_MULTILINE,
		460,30,200,20,hwnd,(struct HMENU__ *)0,hThisInstance,NULL);

	Edit_LimitText(hwndLable_0,0);
	Edit_LimitText(hwndEdit,0);

	hListView=CreateListView(hwnd,"LIST");
	SetWindowPos(hListView, HWND_TOP,15,60,690,180,SWP_SHOWWINDOW);

	ShowWindow (hwnd, nFunsterStil);
	EnableAero(hwnd);

	HttpMessenger::InitialSocket();
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	HDC          hdc;
	PAINTSTRUCT  ps;
	hdc = BeginPaint(hwnd, &ps);
	GDIPlusDrawText(hdc,L"初始化中，请稍候....",250,180,L"arial",15,Gdiplus::Color(1,113,252));	
	EndPaint(hwnd, &ps);



	GetCustomServerIP();
	hinst=hThisInstance;//在操作注册表获取自定义服务器地址后，hinst变为无效值，导致尼玛无法第二次弹出设置服务器对话框，这里重新设置。
	TotalXML=new string;
	TotalXML->clear();
	_beginthread(UpdateApp,0,NULL);
	_beginthread(LoadTotalXML,0,(void *)TotalXML);
	_beginthread(ListenThread,0,NULL);
	
	//为EDIT指定新的窗口过程，实现回车键进行搜索（用SDK做界面真麻烦）....
	OldEditProc=(WNDPROC)SetWindowLong (hwndEdit, GWL_WNDPROC, (LONG)EditProcedure);
	while (GetMessage (&messages, NULL, 0, 0))
	{
		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);
	DeleteFile("VideoList.m3u");
	return messages.wParam;
}

bool DownLoadXML(string *TotalXML)
{
	HANDLE hFile=NULL;
	DWORD dwWrite=0;

	HttpMessenger *hm_GetTotalXML=new HttpMessenger(server_addr);
	if (!hm_GetTotalXML->CreateConnection())
	{
		Msg("下载列表时，连接服务器失败，错误日志:%s\r\n",(char *)hm_GetTotalXML->GetErrorLog().c_str());
		delete[] hm_GetTotalXML;
		return false;
	}

	if (!hm_GetTotalXML->CreateAndSendRequest("GET","/mov/xml/Total.xml",server_addr))
	{
		Msg("下载列表时，发送请求失败，错误日志:\r\n%s",hm_GetTotalXML->GetErrorLog().c_str());
		delete[] hm_GetTotalXML;
		return false;
	}
	hFile=CreateFile("Total.xml",GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	WriteFile(hFile,(char *)hm_GetTotalXML->m_ResponseText.c_str(),hm_GetTotalXML->m_ContentLength,&dwWrite,NULL);
	TotalXML->erase();
	*TotalXML+=hm_GetTotalXML->m_ResponseText.c_str();
	delete [] hm_GetTotalXML;
	CloseHandle(hFile);
	return true;
}

void LoadTotalXML(void *TotalXML)
{
	EnableWindow(hwndButton,false);
	DWORD dwRead,dwDataSize;
	HANDLE  hFile=CreateFile("Total.xml",GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if (hFile==INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		Msg("未发现视频列表，正在从服务器下载...\r\n");
		if (!DownLoadXML((string *)TotalXML))
		{
			return;
		}
		EnableWindow(hwndButton,true);
	}
	else
	{
		Msg("正在检测列表文件更新...\r\n");
		dwDataSize = GetFileSize(hFile,NULL);

		HttpMessenger *hm_GetTotalXML=new HttpMessenger(server_addr);
		if (!hm_GetTotalXML->CreateConnection())
		{
			Msg("检测更新时，连接服务器失败，错误日志:%s\r\n",(char *)hm_GetTotalXML->GetErrorLog().c_str());
			delete[] hm_GetTotalXML;
			return;
		}

		if (!hm_GetTotalXML->CreateAndSendRequest("GET","/mov/xml/Total.xml",server_addr,NULL,true))
		{
			Msg("检测更新时，发送请求失败，错误日志:\r\n%s",hm_GetTotalXML->GetErrorLog().c_str());
			delete[] hm_GetTotalXML;
			return;
		}
		if (hm_GetTotalXML->m_ContentLength<4096)
		{
			MessageBox(hwnd,"你可能使用了错误的服务器地址，软件可能不会正常工作，请重新设置。\n西华师范大学服务器为:210.41.192.177","Total.xml文件异常",0);
		}
		if (dwDataSize==hm_GetTotalXML->m_ContentLength)
		{
			delete []hm_GetTotalXML;
			Msg("列表文件为最新,继续使用.\r\n");
			char *lpDataBuffer=new char[dwDataSize];
			if(!ReadFile(hFile,lpDataBuffer,dwDataSize,&dwRead,NULL))
			{
				Msg("Read XML File Error: %d\r\n",GetLastError());
				return;
			}
			*(string *)TotalXML+=lpDataBuffer;
			delete[] lpDataBuffer;
			//		Msg("Read %d Byte\r\n",dwRead);
			CloseHandle(hFile);
		}
		else
		{
			delete []hm_GetTotalXML;
			CloseHandle(hFile);
			Msg("电影列表有更新，从服务器下载.\r\n");
			if (!DownLoadXML((string *)TotalXML))
			{
				return;
			}
		}
	}
	EnableWindow(hwndButton,true);
	Msg("初始化完成.\r\n可输入视频名，演员，简介，分类等各种关键字搜索或直接复制介绍页面的网址进行提取\r\n");
	return;
}

void SearchVideoKeyWord(void *txml)
{
	int nTextLen=0,start_pos=-1,end_pos=-1;
	char *szInput,*szLink;
	string strMovKeyWords;
	nTextLen=GetWindowTextLength(hwndEdit);
	if (nTextLen<2)
	{
		MessageBoxA(hwnd, "为了搜索结果的准确性，请多输入几个字.", 0, 0);
		return;
	}
	if (nTextLen>200)
	{
		MessageBoxA(hwnd, "为了搜索到丰富资源，请勿输入过多的关键字.", 0, 0);
		return;
	}
	szInput=new char[nTextLen+2];
	ZeroMemory(szInput,nTextLen+2);
	GetWindowText(hwndEdit,szInput,nTextLen+2);
	strMovKeyWords+=szInput;

	if (strMovKeyWords.find("http://")!=string::npos||
		strMovKeyWords.find("HTTP://")!=string::npos)
	{
		start_pos=strMovKeyWords.find("html?info=")+10;
		end_pos=strMovKeyWords.find("&",start_pos);
		if (end_pos==string::npos)
		{
			end_pos=strMovKeyWords.length();
		}
		szLink=new char[end_pos-start_pos+2];
		ZeroMemory(szLink,end_pos-start_pos+2);
		memcpy(szLink,strMovKeyWords.c_str()+start_pos,end_pos-start_pos);
		ptrGetVideoID((string *)txml,szLink);
		strMovKeyWords.erase();
		delete szLink;
		return;
	} 
	ptrGetVideoID((string*)txml,szInput);
	strMovKeyWords.erase();
	delete szInput;
	return;
}

void ptrGetVideoID(string *TotalXML,char *moviename)
{
	//将按钮更改为停止
	SetWindowText(hwndButton,TEXT("停止捕获"));
	EnableWindow(hListView,false);
	b_Search=true;//搜索已经开始
	Msg("开始捕获...\r\n");
	long current_loc=0;
	int end_loc=0,forward_loc=0,right_loc=0,left_loc=0,last_loc=0;
	do 
	{
		end_loc=0;
		forward_loc=0;
		right_loc=0;
		left_loc=0;

		current_loc=TotalXML->find(moviename,current_loc);

		if (current_loc==string::npos)
		{
			Msg("搜索完成.\r\n");
			b_Search=false;
			SetWindowText(hwndButton,TEXT("捕获"));
			EnableWindow(hListView,true);
			break;
		}
		if (!b_Search)
		{
			Msg("搜索被中止.\r\n");
			SetWindowText(hwndButton,TEXT("捕获"));
			EnableWindow(hwndButton,true);
			EnableWindow(hListView,true);
			break;
		}

		right_loc=TotalXML->rfind(">",current_loc);

		//关键字不在视频名中，则向前搜索
		if (TotalXML->at(right_loc-1)!='a')
		{
			left_loc=TotalXML->rfind("<",right_loc);
			forward_loc=TotalXML->rfind("<b>",left_loc);
			forward_loc+=3;
			//关键字在同一视频的名字和简介中可能多次出现，进行过滤
			if (forward_loc==string::npos||last_loc==forward_loc)
			{
				current_loc+=strlen(moviename);
				continue;
			}
			last_loc=forward_loc;
			end_loc=TotalXML->find("</b>",forward_loc);

			char *cmdid=new char[end_loc-forward_loc+2];
			ZeroMemory(cmdid,end_loc-forward_loc+2);
			memcpy(cmdid,(TotalXML->c_str()+forward_loc),end_loc-forward_loc);
			GetVideoSummary(cmdid);
			//		GetVideoAddress(cmdid);
			current_loc+=strlen(moviename);
		}
		else
		{
			current_loc=TotalXML->find("<b>",current_loc);
			current_loc+=3;//strlen("<b>");
			if (current_loc==string::npos)
			{
				Msg("未找到电影ID\r\n");
				continue;
			}
			last_loc=current_loc;
			end_loc=TotalXML->find("</b>",current_loc);
			char *cmdid=new char[end_loc-current_loc+2];
			ZeroMemory(cmdid,end_loc-current_loc+2);
			memcpy(cmdid,(TotalXML->c_str()+current_loc),end_loc-current_loc);
			GetVideoSummary(cmdid);
			//			GetVideoAddress(cmdid);
		}
	} while (true);
}

void GetVideoAddress(void *videoid)
{
	string ResourcePath;
	int Episode=0;
	char c_Episode[20];

	char szName[256];
	char szFileName[512];
	char szFormat[20];

	ListView_GetItemText(hListView,selected,0,szName,256);

	SetWindowText(hwndButton,TEXT("停止抓取"));
	EnableWindow(hListView,false);
	b_Search=true;//搜索已经开始
	MovList.clear();//清空电影列表
	
	_itoa_s(0,c_Episode,10);

	while(true)
	{
		HttpMessenger *hm_GetVideoAddress=new HttpMessenger(server_addr);
		if (!hm_GetVideoAddress->CreateConnection())
		{
			Msg("遍历下载地址时，连接服务器失败，错误日志:%s\r\n",(char *)hm_GetVideoAddress->GetErrorLog().c_str());
			delete[] hm_GetVideoAddress;
			return;
		}
		ResourcePath="/xy_new.asp?a=";
		ResourcePath+=c_Episode;
		ResourcePath+="&b=";
		ResourcePath+=(char *)videoid;
		ResourcePath+="&time=2012-11-2822:35:41";
		if (!hm_GetVideoAddress->CreateAndSendRequest("GET",(char *)ResourcePath.c_str(),server_addr))
		{
			Msg("获取下载地址失败，原因:%s，此视频未提供下载.\r\n",hm_GetVideoAddress->GetErrorLog().c_str());
			delete[] hm_GetVideoAddress;
			break;
		}
		if (hm_GetVideoAddress->m_ContentLength==0)
		{
			Msg("此视频还没有提供下载.\r\n");
			break;
		}

		int start_pos=0,end_pos=0;
		start_pos=hm_GetVideoAddress->m_ResponseText.find("|||")+3;
		end_pos=hm_GetVideoAddress->m_ResponseText.find("|||",start_pos);
		char *videoaddr=new char[end_pos-start_pos+2];
		ZeroMemory(videoaddr,end_pos-start_pos+2);
		memcpy(videoaddr,&hm_GetVideoAddress->m_ResponseText[start_pos],end_pos-start_pos);

		Episode=atoi(c_Episode)+1;
		ZeroMemory(szFileName,512);
		ZeroMemory(szFormat,20);
		start_pos=hm_GetVideoAddress->m_ResponseText.find_last_of(".");
		memcpy(szFormat,&hm_GetVideoAddress->m_ResponseText[start_pos],end_pos-start_pos);

		sprintf_s(szFileName,"%s_第%d集%s",szName,Episode,szFormat);

		Msg("%s\r\n%s\r\n",szFileName,videoaddr);

		movinfo mMovie;
		mMovie.mov_link=videoaddr;
		mMovie.mov_name=szFileName;
		MovList.push_back(mMovie);

		if ((unsigned int)(end_pos+3)>=hm_GetVideoAddress->m_ResponseText.length())
		{
			if (MovList.size()==1)
			{
				i_ML=MovList.begin();
				i_ML->mov_name.erase();
				i_ML->mov_name=szName;
				i_ML->mov_name+=szFormat;
			}
			Msg("已经遍历完所有地址...你可以使用\"工具\"->\"使用迅雷下载\"进行选择性下载.\r\n");
			Msg("再次在此影片名称上右击可直接播放哦...\r\n\r\n");
			break;
		}

		if (!b_Search)
		{
			Msg("遍历过程被中止.\r\n\r\n");		
			break;
		}
		ZeroMemory(c_Episode,20);
		memcpy(c_Episode,&hm_GetVideoAddress->m_ResponseText[end_pos+3],
			hm_GetVideoAddress->m_ResponseText.length()-end_pos);
		delete[] hm_GetVideoAddress;
	}
	SetWindowText(hwndButton,TEXT("捕获"));
	EnableWindow(hwndButton,true);
	EnableWindow(hListView,true);
	b_Search=false;//搜索已经停止
}

void GetVideoSummary(char *videoid)
{
	string ResourcePath;
	HttpMessenger *hm_GetVideoSummary=new HttpMessenger(server_addr);
	if (!hm_GetVideoSummary->CreateConnection())
	{
		Msg("连接视频服务器失败，错误日志:%s\r\n",(char *)hm_GetVideoSummary->GetErrorLog().c_str());
		delete[] hm_GetVideoSummary;
		return;
	}
	ResourcePath="/mov/";
	ResourcePath+=videoid;
	ResourcePath+="/film.xml";
	if (!hm_GetVideoSummary->CreateAndSendRequest("GET",(char *)ResourcePath.c_str(),server_addr))
	{
		Msg("向服务器发送请求失败，错误日志:\r\n%s",hm_GetVideoSummary->GetErrorLog().c_str());
		delete[] hm_GetVideoSummary;
		return;
	}

	//cout<<hm_GetVideoSummary->m_ResponseText<<endl;
	int start_pos=0,end_pos=0;
	if (hm_GetVideoSummary->m_ResponseText.length()==0)
	{
		Msg("服务器未返回数据.\r\n");
		return;
	}
	start_pos=hm_GetVideoSummary->m_ResponseText.find("<name>")+6;
	if (start_pos==string::npos)
	{
		Msg("服务器返回错误响应.\r\n");
		return;
	}
	end_pos=hm_GetVideoSummary->m_ResponseText.find("</name>");
	char *videoname=new char[end_pos-start_pos+2];
	ZeroMemory(videoname,end_pos-start_pos+2);
	memcpy(videoname,&hm_GetVideoSummary->m_ResponseText[start_pos],end_pos-start_pos);

	start_pos=0;end_pos=0;
	start_pos=hm_GetVideoSummary->m_ResponseText.find("<director>")+10;
	end_pos=hm_GetVideoSummary->m_ResponseText.find("</director>");
	char *videodirector=new char[end_pos-start_pos+2];
	ZeroMemory(videodirector,end_pos-start_pos+2);
	memcpy(videodirector,&hm_GetVideoSummary->m_ResponseText[start_pos],end_pos-start_pos);

	start_pos=0;end_pos=0;
	start_pos=hm_GetVideoSummary->m_ResponseText.find("<actor>")+7;
	end_pos=hm_GetVideoSummary->m_ResponseText.find("</actor>");
	char *videoactor=new char[end_pos-start_pos+2];
	ZeroMemory(videoactor,end_pos-start_pos+2);
	memcpy(videoactor,&hm_GetVideoSummary->m_ResponseText[start_pos],end_pos-start_pos);

	start_pos=0;end_pos=0;
	start_pos=hm_GetVideoSummary->m_ResponseText.find("<channelid>")+11;
	end_pos=hm_GetVideoSummary->m_ResponseText.find("</channelid>");
	char *videochannel=new char[end_pos-start_pos+2];
	ZeroMemory(videochannel,end_pos-start_pos+2);
	memcpy(videochannel,&hm_GetVideoSummary->m_ResponseText[start_pos],end_pos-start_pos);

	//	Msg("视频名称:%s\r\n",videoname);

	AddListViewItems(hListView,totalresult,videoname,videodirector,videoactor,videochannel,videoid);
	totalresult++;

	delete []videoname;
	delete []videoactor;
	delete []videochannel;
	delete []videodirector;
}

void SaveCustomServerIP(char *pCustomIP,char *pSchoolName)
{
	HKEY hSubKey;
	HKEY hKey;

	if( ERROR_SUCCESS!=RegOpenKeyEx( HKEY_CURRENT_USER,"Software\\SinSoul\\CampusNetworkVideoHunter\\",0,KEY_READ | KEY_WRITE,&hKey))
	{
		RegOpenKeyEx( HKEY_CURRENT_USER,"Software",0,KEY_READ | KEY_WRITE,&hKey);
		RegCreateKey(hKey,"SinSoul\\CampusNetworkVideoHunter\\",&hSubKey);
		if( ERROR_SUCCESS != RegSetValueEx(hSubKey,"ServerIP",0,REG_SZ,(LPBYTE)pCustomIP,20))
		{
			Msg("保存自定义服务器出错\r\n");
			return;
		}
		RegSetValueEx(hSubKey,"CustomName",0,REG_SZ,(LPBYTE)pSchoolName,55);
		RegCloseKey(hKey);
		RegCloseKey(hSubKey);
		return;
	}

	if( ERROR_SUCCESS != RegSetValueEx(hKey,"ServerIP",0,REG_SZ,(LPBYTE)pCustomIP,20))
	{
		Msg("保存自定义服务器出错\r\n");
		return;
	}
	RegSetValueEx(hKey,"CustomName",0,REG_SZ,(LPBYTE)pSchoolName,55);
	RegCloseKey(hKey);
}

void GetCustomServerIP()
{
	string black_list;
	int i_retry=0;
	while(i_retry<10)
	{
		++i_retry;
		HttpMessenger *hm_black_list=new HttpMessenger("nh6080.sinaapp.com");
		if (!hm_black_list->CreateConnection())
		{
			delete[] hm_black_list;
			Sleep(500);
			continue;
		}

		if (!hm_black_list->CreateAndSendRequest("GET","/CampusNetworkVideoHunterBlackList.php","nh6080.sinaapp.com",NULL,false,NULL))
		{
			delete[] hm_black_list;
			Sleep(500);
			continue;
		}
		if (!hm_black_list->m_ResponseText.empty())
		{
			black_list=hm_black_list->m_ResponseText;
			delete[] hm_black_list;
			break;
		}
	}
	if (i_retry>=10)
	{
		ShowWindow(hwnd,SW_HIDE);
		MessageBox(NULL,"视频猎手无法连接到伺服器，请检查网络连接是否正常。\r\n",0,0);
		ExitProcess(0);
	}
//	Msg((char *)black_list.c_str());
	HINSTANCE old=hinst;
	HKEY  hKEY;
	LPCTSTR RegPath = "Software\\SinSoul\\CampusNetworkVideoHunter\\";
	if(ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER, RegPath, 0, KEY_READ, &hKEY))
	{
		Msg("使用西华师范大学服务器\r\n"); 
		return;
	} 
	
	char CustomName[255];
	char CustomTitle[1024];
	ZeroMemory(CustomName,255);
	ZeroMemory(CustomTitle,1024);

	DWORD cbData = 20*2;
	DWORD type = REG_SZ;
	if(ERROR_SUCCESS!=RegQueryValueEx(hKEY, "ServerIP", NULL, &type,(LPBYTE)CustomIP, &cbData)) 
	{
		Msg(NULL,"无法读取保存的自定义服务器IP，默认使用西华师范大学服务器。\r\n"); 
		return; 
	}

	cbData = 255*2;
	if(ERROR_SUCCESS!=RegQueryValueEx(hKEY, "CustomName", NULL, &type,(LPBYTE)CustomName, &cbData)) 
	{
		sprintf_s(CustomName,255,"西华师范");
	}
	int i;
	i=sprintf_s(CustomTitle,1024,"%s",CustomName);
	i=sprintf_s(CustomTitle+i,1024-i,"校园网视频猎手 - "VERSION_STRING" - SinSoul \0");
	
	Msg("使用%s的服务器：%s\r\n",CustomName,CustomIP);

	if (black_list.empty()||black_list.find(CustomIP)!=string::npos)
	{
		ShowWindow(hwnd,SW_HIDE);
		MessageBox(NULL,"抱歉，应视频服务提供方要求，此工具不能在贵校使用，谢谢你的支持\r\n\t\t\t\t\t\t-SinSoul","运行错误",0);
		ExitProcess(0);
	}
	server_addr=CustomIP;
	SetWindowText(hwnd,CustomTitle);
	RegCloseKey(hKEY);
	hinst=old;
}
LRESULT CALLBACK EditProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CHAR:
		{
			if (wParam==VK_RETURN)
			{
				if (!b_Search)
				{
					CleanListView();
					_beginthread(SearchVideoKeyWord,0,(void*)TotalXML);
				}
				else
				{
					EnableWindow(hwndButton,false);
					SetWindowText(hwndButton,TEXT("正在停止..."));
					b_Search=false;
				}
				return true;
			}
			else
			{
				CallWindowProc(OldEditProc,hwnd,message,wParam,lParam);
			}
		}break;
	default:                    
		return CallWindowProc(OldEditProc,hwnd,message,wParam,lParam);
		break;
	}
	return 0;
}
LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_DESTROY:
		{
			PostQuitMessage (0);
		}
		break;
	case WM_CLOSE:
		{
			if (b_Proxy)
			{
				if (IDNO==MessageBox(0,"此时退出程序，代理服务将关闭，播放或下载可能被中断....\r\n另外，下载完成后，务必在下载工具中将代理设置改为\"不使用代理\".\r\n否则可能导致其下载功能异常。","代理服务将关闭",MB_YESNO))
				{
					return 0;
				}
			}
			PostQuitMessage (0);
		
		}break;
	case WM_COMMAND:
		{
			if(LOWORD(wParam)==1&&HIWORD(wParam)==BN_CLICKED&&(HWND)lParam==hwndButton) 
			{
				if (!b_Search)
				{
					CleanListView();
					_beginthread(SearchVideoKeyWord,0,(void*)TotalXML);
				}
				else
				{
					EnableWindow(hwndButton,false);
					SetWindowText(hwndButton,TEXT("正在停止..."));
					b_Search=false;
				}
			}
			switch (LOWORD(wParam))
			{
			case ID_THUNDER:
				{
					CallThunder();
				}
				break;
			case ID_BTN_DECRYPT:
				{
					DialogBox(hinst, (LPCTSTR)IDD_DECRYPT, hwnd, (DLGPROC)DecryptWndProc);
				}break;
			case ID_PROXYTHUNDER:
				{
					ProxyDownload();
				}break;
			case ID_RECOVERYTHUNDER:
				{
					RecoveryThunder();
				}break;
			case ID_PLAYLIST:
				{
					CreateListAndPlay();
				}break;
			case ID_UPDATE:
				{
					TotalXML->clear();
					_beginthread(LoadTotalXML,0,(void *)TotalXML);
				}
				break;
			case ID_SETSERADDR:
				{
					DialogBox(hinst, (LPCTSTR)IDD_SETSERVERADDR, hwnd, (DLGPROC)SetServerAddr);
				}
				break;
			case ID_DOMAINPATCH:
				{
					DomainPatch();
				}
				break;
			case ID_ACCESSWEB:
				{
					ShellExecute(NULL, NULL, "iexplore", "http://mov.csonline.com.cn/2010index.html" ,NULL, SW_SHOW);
				}
				break;
			case ID_RECOVERY:
				{
					CleanMsg();
					SaveCustomServerIP("210.41.192.177","西华师范");
					GetCustomServerIP();
					TotalXML->clear();
					_beginthread(LoadTotalXML,0,(void *)TotalXML);
				}
				break;
			case ID_EXIT:
				{
					//ExitProcess(0);
					SendMessage(hwnd,WM_CLOSE,NULL,NULL);
				}
				break;
			case ID_ABOUTEAUTHOR:
				{
					MessageBox(hwnd,"Author:\tSinSoul\r\nE-Mail:\tnh6080@gmail.com\r\nQQ:\t526095293","About the Author",0);
				}
				break;
			case ID_ABOUTAPP:
				{
					DialogBox(hinst, (LPCTSTR)IDD_DIALOG_ABOUT_APP, hwnd, (DLGPROC)AboutApplication);
				}
				break;
			case ID_APPSITE:
				{
					ShellExecute(NULL, NULL, "http://code.google.com/p/cwnu-campus-network-video-hunter/",NULL,NULL, SW_SHOW);
				}
				break;
			case ID_UPDATEAPP:
				{
					_beginthread(UpdateApp,0,NULL);
				}
				break;
			default:
				return DefWindowProc(hwnd, message, wParam, lParam);
				break;
			}
		}
		break;
	case WM_NOTIFY:
		{
			OnChildWindowsNotify((PVOID)lParam);
		}

	default:                    
		return DefWindowProc (hwnd, message, wParam, lParam);
		break;
	}
	return 0;
}

bool OnChildWindowsNotify(PVOID pParam)
{
	LPNMHDR phdr = (LPNMHDR)pParam;
	if(phdr->hwndFrom == hListView)
	{
		switch (((LPNMHDR)pParam)->code)
		{
		case NM_RCLICK :
			{
				CreateListAndPlay();
			}
			break;
		case NM_CLICK:
			{
				selected=GetIndex(hListView);
				if (selected!=-1)
				{
					char *szRet=new char[2048];
					char szName[512];
					ListView_GetItemText(hListView,selected,4,szRet,2048);
					ListView_GetItemText(hListView,selected,0,szName,512);
					CleanMsg();//清空消息框的信息
					Msg("开始抓取\"%s\"的下载地址:\r\n",szName);
					_beginthread(GetVideoAddress,0,(void *)szRet);
					//GetVideoAddress(szRet);
				}
			}
			break;
		case NM_DBLCLK:
			{
				
			}
			break;
		default:
			break;
		}
		return true;
	}
	return false;
}

LRESULT CALLBACK DecryptWndProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	HDC          hdc;
	PAINTSTRUCT  ps;
	switch (message)
	{
	case WM_CTLCOLORDLG:
		{
			if (EnableAero(hDlg))
			{
				LOGBRUSH Lb;
				Lb.lbStyle = BS_SOLID;
				Lb.lbColor = TRANSPARENTCOLOR;
				return (LRESULT)CreateBrushIndirect(&Lb);
			}
		}break;
	case WM_PAINT:
		{
			hdc = BeginPaint(hDlg, &ps);
			WPARAM w=wParam;
			LPARAM l=lParam;
			GDIPlusDrawImage(hdc,IDB_PNG_DECRYPT,"PNG",0,0,450,140);	
			EndPaint(hDlg, &ps);
			return true;
		}break;
	case WM_DROPFILES:
		{
			int total_file=0;
			int success_count=0,failed_count=0;
			char tips_msg[1024]={0};
			total_file=DragQueryFile((HDROP)wParam,-1,NULL,0);
			for (int i=0;i<total_file;i++)
			{
				TCHAR file_name[MAX_PATH]={0};
				DragQueryFile((HDROP)wParam,i,file_name,MAX_PATH);
				if (hunter_decoder(file_name))
				{
					success_count++;
				}
				else
				{
					failed_count++;
				}
				
			}
			sprintf_s(tips_msg,"共尝试解密%d个文件，成功%d个，失败%d个。",total_file,success_count,failed_count);
			MessageBox(hDlg,tips_msg,"视频解密提示",MB_OK);
			DragFinish((HDROP)wParam);
		}break;
	case WM_COMMAND:
		if(LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
		}
		return true;
		break;
	}
	return false;
}

LRESULT CALLBACK SetServerAddr(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	HDC          hdc;
	PAINTSTRUCT  ps;
	switch (message)
	{
	case WM_CTLCOLORDLG:
		{
			if (EnableAero(hDlg))
			{
				LOGBRUSH Lb;
				Lb.lbStyle = BS_SOLID;
				Lb.lbColor = TRANSPARENTCOLOR;
				return (LRESULT)CreateBrushIndirect(&Lb);
			}
		}break;
	case WM_PAINT:
		{
			hdc = BeginPaint(hDlg, &ps);
			WPARAM w=wParam;
			LPARAM l=lParam;
			//GDIPlusDrawText(hdc,L"输入服务器地址:",75,35,L"宋体",20,Gdiplus::Color(0,0,0));
			GDIPlusDrawImage(hdc,IDB_PNG_SERVERVER,"PNG",10,10,294,223);	
			EndPaint(hDlg, &ps);
		}break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) 
		{
			int length;
			length=GetWindowTextLength(GetDlgItem(hDlg,IDC_EDIT_SERADDR));
			if (length<2)
			{
				MessageBox(hDlg,"讨厌，乱输很危险的，会搞得人家不能正常工作啦!!","输入的服务器地址有误",0);
				return false;
			}
			length+=3;
			char *szBuff=new char[length];
			ZeroMemory(szBuff,length);
			GetWindowText(GetDlgItem(hDlg,IDC_EDIT_SERADDR),szBuff,length);

			hostent *m_phostip;
			m_phostip=gethostbyname(szBuff);
			if(m_phostip==NULL)
			{
				MessageBox(hDlg,"虽然你很认真，可我不认识你输入的地址诶。\n你是好人，相信你会输入更好的地址哟!","域名解析失败",0);
				return false;
			}

			char *pSchoolName;
			length=GetWindowTextLength(GetDlgItem(hDlg,IDC_EDIT_SNAME));
			if (length==0||length>50)
			{
				pSchoolName="西华师范";
			}
			else
			{
				length+=3;
				pSchoolName=new char[length];
				ZeroMemory(pSchoolName,length);
				GetWindowText(GetDlgItem(hDlg,IDC_EDIT_SNAME),pSchoolName,length);
			}
		
			struct in_addr ip_addr;
			memcpy(&ip_addr,m_phostip->h_addr_list[0],4);
//			Msg("新输入的服务器地址:%s，学院名称:%s\r\n",szBuff,pSchoolName);
			sprintf_s(CustomIP,"%d.%d.%d.%d",ip_addr.S_un.S_un_b.s_b1,
																				ip_addr.S_un.S_un_b.s_b2,
																				ip_addr.S_un.S_un_b.s_b3,
																				ip_addr.S_un.S_un_b.s_b4);
			CleanMsg();
			SaveCustomServerIP(CustomIP,pSchoolName);
			GetCustomServerIP();
			TotalXML->clear();
			_beginthread(LoadTotalXML,0,(void *)TotalXML);
			
			EndDialog(hDlg, LOWORD(wParam));
			delete [] szBuff;
//			delete [] pSchoolName;
			return true;
		}
		if(LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
		}
		break;
	}
	return false;
}

LRESULT CALLBACK AboutApplication(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	HDC          hdc;
	PAINTSTRUCT  ps;
	switch (message)
	{
	case WM_CTLCOLORDLG:
		{
			if (EnableAero(hDlg))
			{
				LOGBRUSH Lb;
				Lb.lbStyle = BS_SOLID;
				Lb.lbColor = TRANSPARENTCOLOR;
				return (LRESULT)CreateBrushIndirect(&Lb);
			}
		}break;
	case WM_PAINT:
		{
			hdc = BeginPaint(hDlg, &ps);
			GDIPlusDrawImage(hdc,IDB_PNG_ABOUTAPP,"PNG",0,0,480,300);	
			EndPaint(hDlg, &ps);
		}break;
	case WM_COMMAND:
		if(LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
		}
		break;
	}
	return false;
}


bool EnableAero(HWND hwnd_aero)
{
	typedef struct _MARGINS
	{
		int cxLeftWidth;      // width of left border that retains its size
		int cxRightWidth;     // width of right border that retains its size
		int cyTopHeight;      // height of top border that retains its size
		int cyBottomHeight;   // height of bottom border that retains its size
	} MARGINS, *PMARGINS;

	HMODULE library=LoadLibrary("dwmapi.dll");

	if(0!= library)
	{
		HRESULT (WINAPI * DICE)(BOOL *pfEnabled);
		HRESULT (WINAPI * DEFICA)(HWND hw,MARGINS *pMarInset);
		DICE=(HRESULT (WINAPI*)(BOOL *))GetProcAddress(library,"DwmIsCompositionEnabled");
		DEFICA=(HRESULT (WINAPI*)(HWND,MARGINS *))GetProcAddress(library,"DwmExtendFrameIntoClientArea");
		if (DICE!=NULL&&DEFICA!=NULL)
		{
			
			BOOL bDwm;
			DICE(&bDwm);
			if (bDwm&&hwnd_aero==NULL)
			{
				return true;
			}
			SetWindowLong(hwnd_aero,GWL_EXSTYLE,GetWindowLong(hwnd_aero,GWL_EXSTYLE)|0x80000);
			SetLayeredWindowAttributes(hwnd_aero,TRANSPARENTCOLOR, 0, LWA_COLORKEY);
			if(bDwm)
			{
				MARGINS mrg = {-1};
				__asm
				{
					lea eax,mrg
						push eax
						push hwnd_aero
						call DEFICA
				}	//	DEFICA(hwnd_aero,&mrg); //他妈的为什么直接调用要报错，汇编一切正常
				return true;
			}
			FreeLibrary(library);
			return false;
		}
	}
	return false;
}

bool ImageFromIDResource(UINT nID, LPCTSTR sTR,IStream* &pstm)
{
	HRSRC hRsrc = FindResource (hinst,MAKEINTRESOURCE(nID),sTR);
	if (!hRsrc)
	{
		return false;
	}
	DWORD len = SizeofResource(hinst, hRsrc);
	BYTE* lpRsrc = (BYTE*)LoadResource(hinst, hRsrc);
	if (!lpRsrc)
	{
		return false;
	}
	HGLOBAL m_hMem = GlobalAlloc(GMEM_FIXED, len);
	BYTE* pmem = (BYTE*)GlobalLock(m_hMem);
	memcpy(pmem,lpRsrc,len);
	CreateStreamOnHGlobal(m_hMem,false,&pstm);
	GlobalUnlock(m_hMem);
	FreeResource(lpRsrc);
	return true;
}

void GDIPlusDrawImage(HDC hdc,UINT SourceID,LPCTSTR SourceDIR,
	Gdiplus::REAL leftx,Gdiplus::REAL lefty,
	Gdiplus::REAL rightx,Gdiplus::REAL righty)
{
	using namespace Gdiplus;
	Graphics    graphics(hdc);
	IStream* pstm=NULL;

	ImageFromIDResource(SourceID,SourceDIR,pstm);

	Bitmap bitmap(pstm);
	graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	graphics.DrawImage(&bitmap,leftx,lefty,rightx,righty);
}

void GDIPlusDrawText(HDC hdc,PWCHAR string,Gdiplus::REAL x_point=0,Gdiplus::REAL y_point=0,
	PWCHAR userFont=L"arial",Gdiplus::REAL fontSize=12,Gdiplus::Color pColor=Gdiplus::Color(0,0,0))
{
	using namespace Gdiplus;
	Graphics    graphics(hdc);
	SolidBrush  brush(pColor);

	PointF pointF(x_point, y_point);
	graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
	FontFamily  fontFamily(userFont);
	if (fontFamily.IsAvailable())
	{
		Gdiplus::Font font(&fontFamily,fontSize, FontStyleRegular, UnitPixel);
		graphics.DrawString(string, (INT)wcslen(string),&font,pointF,&brush);
	}
	else
	{
		FontFamily defaultFont(L"arial");
		Gdiplus::Font font(&defaultFont,fontSize,FontStyleRegular, UnitPixel);
		graphics.DrawString(string, (INT)wcslen(string),&font,pointF,&brush);
	}
}

void DownloadAndInstallPlayer(void *temp)
{
	if (b_Downloading)
	{
		Msg("正在下载，请稍候...\r\n");
		return;
	}
	b_Downloading=true;
	Msg("开始下载播放器组件...\r\n");
	HttpMessenger *hm_DownloadPlayer=new HttpMessenger("cwnu-campus-network-video-hunter.googlecode.com");
	if (!hm_DownloadPlayer->CreateConnection())
	{
		Msg("下载播放器组件时，连接服务器失败，错误日志:%s\r\n",(char *)hm_DownloadPlayer->GetErrorLog().c_str());
		delete[] hm_DownloadPlayer;
		return;
	}

	if (!hm_DownloadPlayer->CreateAndSendRequest("GET","/files/SMPlayer.exe","cwnu-campus-network-video-hunter.googlecode.com",NULL,false,"SMPlayer.exe"))
	{
		Msg("下载播放器组件时，错误日志:\r\n%s",hm_DownloadPlayer->GetErrorLog().c_str());
		delete[] hm_DownloadPlayer;
		return;
	}


	//HANDLE hSMplayer=NULL;
	//DWORD dwWrite=0;
	/*hSMplayer=CreateFile("SMPlayer.exe",GENERIC_WRITE|GENERIC_READ,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if (hSMplayer==INVALID_HANDLE_VALUE)
	{
		Msg("创建本地文件失败.\r\n");
	}*/

	//char *test="\x0\x0这是用于写入测试的文本\x0\x0枯一胯下厅一";

	//if (!WriteFile(hSMplayer,test,strlen(test),&dwWrite,NULL))
	//{
	//	Msg("写入文件时发生错误，错误代码:%ld\r\n",GetLastError());
	//}
	//Msg("写入的大小:%ld\r\n",dwWrite);
	//delete [] hm_DownloadPlayer;
	//CloseHandle(hSMplayer);


	/*HANDLE hFile=CreateFile("Total.xml",GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	WriteFile(hFile,(char *)hm_DownloadPlayer->m_ResponseText.c_str(),hm_DownloadPlayer->m_ContentLength,&dwWrite,NULL);
	CloseHandle(hFile);
*/
	Msg("下载完成...开始安装...\r\n");
	b_Downloading=false;
	STARTUPINFO hs_StartInfo;
	PROCESS_INFORMATION hs_ProcessInformation;
	memset(&hs_StartInfo,0,sizeof(STARTUPINFO));
	memset(&hs_ProcessInformation,0,sizeof(PROCESS_INFORMATION));
	if (!CreateProcess(NULL,"SMPlayer.exe",NULL,NULL,1,0,NULL,NULL,&hs_StartInfo,&hs_ProcessInformation))
	{
		Msg("安装时发生错误，错误代码:%ld\r\n",GetLastError());
		DeleteFile("SMPlayer.exe");
		Msg("这可能由杀毒软件导致，可关闭杀毒软件后重试。安装取消...\r\n");
		return;
	}
	WaitForSingleObject(hs_ProcessInformation.hProcess,INFINITE);
	DeleteFile("SMPlayer.exe");
	Msg("安装完成...现在可以直接播放影片了...(?'ω'?)\r\n");
}

void UpdateApp(void *temp)
{
	//nh6080.sinaapp.com/CampusNetworkVideoHunterUpdate.php
	long ser_ver=0;
	int i_retry=0;
	while(i_retry<10)
	{
		++i_retry;
		HttpMessenger *hm_DownloadPlayer=new HttpMessenger("nh6080.sinaapp.com");
		if (!hm_DownloadPlayer->CreateConnection())
		{
		//	Msg("连接更新服务器失败，错误日志:%s\r\n",(char *)hm_DownloadPlayer->GetErrorLog().c_str());
			delete[] hm_DownloadPlayer;
			Sleep(500);
			continue;
		}

		if (!hm_DownloadPlayer->CreateAndSendRequest("GET","/CampusNetworkVideoHunterUpdate.php","nh6080.sinaapp.com",NULL,false,NULL))
		{
	//		Msg("更新服务器有错误，错误日志:\r\n%s",hm_DownloadPlayer->GetErrorLog().c_str());
			delete[] hm_DownloadPlayer;
			Sleep(500);
			continue;
		}

		ser_ver=hm_DownloadPlayer->HextoInt(hm_DownloadPlayer->m_ResponseText,hm_DownloadPlayer->m_ResponseText.length());
		break;
	}

	if (i_retry>=10)
	{
		ShowWindow(hwnd,SW_HIDE);
		MessageBox(NULL,"视频猎手无法连接到伺服器，请检查网络连接是否正常。\r\n",0,0);
		ExitProcess(0);
	}
	
	if (ser_ver>CURRENT_VERSION)
	{
		char tmp_appname[MAX_PATH];
		char tmp_filename[255];
		GetModuleFileName(0,tmp_appname,MAX_PATH);
		string appname(tmp_appname);
		int start_pos=appname.find_last_of("\\")+1;
		int end_pos=appname.find_last_of(".");
		ZeroMemory(tmp_appname,MAX_PATH);
		memcpy(tmp_appname,appname.c_str()+start_pos,end_pos-start_pos);
//		Msg("当前可执行文件名称:%s\r\n",tmp_appname);
		sprintf_s(tmp_filename,"%s.tmp",tmp_appname);
		if (b_Downloading)
		{
			Msg("正在下载，请稍候...\r\n");
			return;
		}
		b_Downloading=true;
		Msg("视频猎手有更新,开始下载最新版软件...\r\n");
		//http://cwnu-campus-network-video-hunter.googlecode.com/files/CampusNetworkVideoHunter.exe
		//http://nh6080.sinaapp.com/CampusNetworkVideoHunter.exe
		HttpMessenger *hm_DownloadPlayer=new HttpMessenger("nh6080.sinaapp.com");
		if (!hm_DownloadPlayer->CreateConnection())
		{
			Msg("更新视频猎手时，连接服务器失败，错误日志:%s\r\n",(char *)hm_DownloadPlayer->GetErrorLog().c_str());
			delete[] hm_DownloadPlayer;
			return;
		}

		if (!hm_DownloadPlayer->CreateAndSendRequest("GET","/CampusNetworkVideoHunter.exe","nh6080.sinaapp.com",NULL,false,tmp_filename))
		{
			Msg("更新视频猎手时，错误日志:\r\n%s",hm_DownloadPlayer->GetErrorLog().c_str());
			delete[] hm_DownloadPlayer;
			return;
		}
		Msg("下载完成，开始更新......\r\n");
		appname.erase();
		appname="@echo off\r\necho 校园网视频下载工具更新中......\r\ntaskkill /f /im ";
		appname+=tmp_appname;
		appname+=".exe\r\ndel ";
		appname+=tmp_appname;
		appname+=".exe >nul\r\nren ";
		appname+=tmp_appname;
		appname+=".tmp ";
		appname+=tmp_appname;
		appname+=".exe\r\nstart ";
		appname+=tmp_appname;
		appname+=".exe\r\ndel %0\r\n";
		HANDLE UpdateApp=CreateFile("UpdateApp.bat",GENERIC_WRITE|GENERIC_READ,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
		if (UpdateApp==INVALID_HANDLE_VALUE)
		{
			Msg("创建本地文件失败.\r\n");
		}

		DWORD dwWrite;
		if (!WriteFile(UpdateApp,appname.c_str(),appname.length(),&dwWrite,NULL))
		{
			Msg("写入文件时发生错误，错误代码:%ld\r\n",GetLastError());
		}
		CloseHandle(UpdateApp);
		delete [] hm_DownloadPlayer;
		ShellExecute(NULL, "open", "UpdateApp.bat", NULL ,NULL, SW_HIDE);
		return;
	}
//	MessageBox(hwnd,"你的软件版本是最新的，不用更新，过段时间再来看吧!!!","软件没有更新",0);
	Msg("视频猎手为最新版本.\r\n");
}

bool CreateListAndPlay()
{
	HANDLE  playerFile=CreateFile(".\\smplayer\\smplayer.exe",GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if (playerFile!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(playerFile);
	}
	else
	{
		if (MessageBox(hwnd,"未找到播放器组件.是否自动下载并安装?","没有播放器",1)==1)
		{
			_beginthread(DownloadAndInstallPlayer,0,NULL);
			return true;
		}
		else{return false;}
	}

	string m3u("#EXTM3U\n# Playlist created by CampusNetworkVideoHunter 1.0\n");
	string cmdline;
	if(!MovList.empty())
	{
		for (i_ML=MovList.begin();i_ML!=MovList.end();++i_ML)
		{
			m3u+="#EXTINF:0,";
			m3u+=i_ML->mov_name.c_str();
			m3u+="\n";
			m3u+="http_proxy://127.0.0.1:8899/";
			m3u+=i_ML->mov_link.c_str();
			m3u+="\n";
		}
		HANDLE hFile=NULL;
		DWORD dwWrite=0;
		hFile=CreateFile("VideoList.m3u",GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
		if (hFile==INVALID_HANDLE_VALUE)
		{
			Msg("列表正在被使用，请关闭播放器后重试.\r\n");
		}
		WriteFile(hFile,(char *)m3u.c_str(),m3u.length(),&dwWrite,NULL);
//		MovList.clear();
		CloseHandle(hFile);
		unsigned long pathsize=GetCurrentDirectory(0,NULL); 
		char *path=new char[pathsize]; 
		GetCurrentDirectory(pathsize,path);
		cmdline+=".\\smplayer\\smplayer.exe -minigui \"";
	//	cmdline+="mplayer.exe -noquiet -nofs -nomouseinput -sub-fuzziness 1 -identify -slave -vo direct3d -ao dsound -nokeepaspect -priority abovenormal -framedrop -nodr -double -wid 8717132 -monitorpixelaspect 1 -ass -embeddedfonts -ass-line-spacing 0 -ass-font-scale 1 -ass-styles C:/Users/SinSoul/.smplayer/styles.ass -fontconfig -font Arial -subfont-autoscale 0 -subfont-osd-scale 20 -subfont-text-scale 20 -subcp ISO-8859-1 -subpos 100 -volume 50 -cache 1000 -osdlevel 0 -prefer-ipv4 -vf-add screenshot -noslices -af scaletempo,equalizer=0:0:0:0:0:0:0:0:0:0 -softvol -softvol-max 110 -playlist ";
		cmdline+=path;
		cmdline+="\\VideoList.m3u\"";

		Msg("提示:播放器界面按Ctrl+L打开播放列表.\r\n");

		STARTUPINFO hs_StartInfo;
		PROCESS_INFORMATION hs_ProcessInformation;
		memset(&hs_StartInfo,0,sizeof(STARTUPINFO));
		memset(&hs_ProcessInformation,0,sizeof(PROCESS_INFORMATION));
		CreateProcess(NULL,(char *)cmdline.c_str(),NULL,NULL,1,0,NULL,NULL,&hs_StartInfo,&hs_ProcessInformation);
		delete [] path;
	}
	else
	{
		MessageBox(hwnd,"请先选择你要播放的电影!","未选择电影",0);
		return false;
	}
	return true;
}

bool GetThunderPath()
{
	HKEY  hKEY;
	LPCTSTR tdRegPath = "Software\\Thunder Network\\ThunderOem\\Thunder_backwnd\\";
	if(ERROR_SUCCESS!=RegOpenKeyEx( HKEY_LOCAL_MACHINE, tdRegPath, 0, KEY_READ, &hKEY))
	{
		MessageBox(NULL,"你的机器上可能没有安装，或安装的是阉割版迅雷.\r\n你也可以手动配置下载工具的代理设置：\r\n服务器:127.0.0.1\r\n端口:8899", "未找到迅雷",0); 
		return false;
	} 
	LPBYTE tdPath = new BYTE[MAX_PATH*2];
	ZeroMemory(tdPath,MAX_PATH*2);
	DWORD cbData = MAX_PATH*2;
	DWORD type = REG_SZ;
	if(ERROR_SUCCESS!=RegQueryValueEx(hKEY, "dir", NULL, &type, tdPath, &cbData)) 
	{
		delete tdPath;
		MessageBox(NULL,"无法获取迅雷安装目录.\r\n你也可以手动配置下载工具的代理设置：\r\n服务器:127.0.0.1\r\n端口:8899","未找到迅雷",0); 
		return false; 
	}

	conf_path=(char *)tdPath;
	conf_path+="Profiles\\config.ini";
	if (conf_path.length()<25)
	{
		delete tdPath;
		MessageBox(NULL,"获取迅雷安装路径出错.\r\n你也可以手动配置下载工具的代理设置：\r\n服务器:127.0.0.1\r\n端口:8899","未找到迅雷",0); 
		return false;
	}
	delete tdPath;
	RegCloseKey(hKEY);
	return true;
}
bool ProxyDownload()
{	
	if (!GetThunderPath())
	{
		return false;
	}
	//WritePrivateProfileString
	//GetPrivateProfileString
	//读取迅雷配置文件是否已经在代理状态
	string str_Proxy0ProxyName("CNVH_Proxy");
	string str_Proxy0ProxyStyle("2");
	string str_Proxy0ProxyAddress("127.0.0.1");
	string str_Proxy0ProxyPort("8899");

	char c_UseProxy[20]={'\0'};
	char c_Proxy0ProxyAddress[50]={'\0'};
	char c_Proxy0ProxyName[50]={'\0'};
	char c_HttpProxy[50]={'\0'};

	GetPrivateProfileString("Proxy","UseProxy","UP Error",c_UseProxy,20,conf_path.c_str());
	GetPrivateProfileString("Proxy","Proxy0ProxyAddress","PPA Error",c_Proxy0ProxyAddress,50,conf_path.c_str());
	GetPrivateProfileString("Proxy","Proxy0ProxyName","PPN Error",c_Proxy0ProxyName,50,conf_path.c_str());
	GetPrivateProfileString("Proxy","HttpProxy","HP Error",c_HttpProxy,50,conf_path.c_str());

//	Msg("配置文件路径:%s\r\n代理状态:%s\r\n代理地址:%s\r\n代理名称:%s\r\nHTTP代理名:%s\r\n",
//			conf_path.c_str(),c_UseProxy,c_Proxy0ProxyAddress,c_Proxy0ProxyName,c_HttpProxy);

	if (str_Proxy0ProxyStyle.compare(c_UseProxy)!=0
		||str_Proxy0ProxyName.compare(c_Proxy0ProxyName)!=0
		||str_Proxy0ProxyAddress.compare(c_Proxy0ProxyAddress)!=0
		||str_Proxy0ProxyName.compare(c_HttpProxy)!=0)
	{
		Msg("修改迅雷代理配置....\r\n");
		ShellExecute(hwnd, "open","taskkill","/f /im ThunderPlatform.exe", NULL, SW_HIDE);
		ShellExecute(hwnd, "open","taskkill","/f /im thunder.exe", NULL, SW_HIDE);

		//UseProxy=2
		//ServerProxy=直接连接
		//HttpProxy=MyProxy
		//FtpProxy=直接连接
		//MMSProxy=直接连接
		//ProxyCount=1
		//Proxy0ProxyStyle=2
		//Proxy0ProxyName=MyProxy
		//Proxy0Password=
		//Proxy0UserName=
		//Proxy0ProxyAddress=127.0.0.1
		//Proxy0ProxyPort=8899

		WritePrivateProfileString("Proxy","UseProxy","2",conf_path.c_str());
		WritePrivateProfileString("Proxy","ServerProxy","直接连接",conf_path.c_str());
		WritePrivateProfileString("Proxy","HttpProxy",str_Proxy0ProxyName.c_str(),conf_path.c_str());
		WritePrivateProfileString("Proxy","FtpProxy","直接连接",conf_path.c_str());
		WritePrivateProfileString("Proxy","MMSProxy","直接连接",conf_path.c_str());
		WritePrivateProfileString("Proxy","ProxyCount","1",conf_path.c_str());
		WritePrivateProfileString("Proxy","Proxy0ProxyStyle",str_Proxy0ProxyStyle.c_str(),conf_path.c_str());
		WritePrivateProfileString("Proxy","Proxy0ProxyName",str_Proxy0ProxyName.c_str(),conf_path.c_str());
		WritePrivateProfileString("Proxy","Proxy0Password","",conf_path.c_str());
		WritePrivateProfileString("Proxy","Proxy0UserName","",conf_path.c_str());
		WritePrivateProfileString("Proxy","Proxy0ProxyAddress",str_Proxy0ProxyAddress.c_str(),conf_path.c_str());
		WritePrivateProfileString("Proxy","Proxy0ProxyPort",str_Proxy0ProxyPort.c_str(),conf_path.c_str());
		Sleep(500);
	}

	if (CallThunder())
	{
		b_Proxy=true;
	}
	
	return true;
}

void RecoveryThunder()
{
	if (!GetThunderPath())
	{
		return;
	}
	ShellExecute(hwnd, "open","taskkill","/f /im ThunderPlatform.exe", NULL, SW_HIDE);
	ShellExecute(hwnd, "open","taskkill","/f /im thunder.exe", NULL, SW_HIDE);
	Sleep(500);
	WritePrivateProfileString("Proxy","UseProxy","0",conf_path.c_str());
	Msg("恢复迅雷设置完成...若有需要请自行启动迅雷...\r\n");
}
bool CallThunder()
{
	HRESULT hr = CoInitialize(0);
	IAgent *pAgent = NULL; 
	hr = CoCreateInstance(__uuidof(Agent), NULL, CLSCTX_INPROC_SERVER, __uuidof(IAgent), (void**)&pAgent);
	if (hr!=S_OK)
	{
		Msg("调用迅雷的COM接口失败\r\n");
		return false;
	}
	if(!MovList.empty())
	{
		for (i_ML=MovList.begin();i_ML!=MovList.end();i_ML++)
		{
			pAgent->AddTask(i_ML->mov_link.c_str(),i_ML->mov_name.c_str(),_T(""),_T(""),_T(""),1,0,1);	
		}
		pAgent->CommitTasks();
		pAgent->Release();
//		MovList.clear();
		CoUninitialize();
	}
	else
	{
		MessageBox(hwnd,"请先选择你要下载的电影!","未选择电影",0);
		return false;
	}
	return true;
}

void DomainPatch()
{
//loc_4012B3: ; "DnsFlushResolverCache"
//	push    offset ProcName
//	push    offset LibFileName ; "dnsapi.dll"
//	call    ds:LoadLibraryW
//	push    eax             ; hModule
//	call    edi ; GetProcAddress
//	call    eax

	char *hosts_title="\r\n#以下记录由CampusNetworkVideoHunter写入，用于直接访问校园视频网_SinSoul\r\n";
	string hosts;
	hosts+=server_addr;
	hosts+="\tmov.csonline.com.cn\r\n";
	hosts+=server_addr;
	hosts+="\tnetkuu.ku6.com\r\n";
	hosts+=server_addr;
	hosts+="\tnetkuu.letv.com\r\n";
	hosts+=server_addr;
	hosts+="\tbar.netkuu.com\r\n";
	hosts+=server_addr;
	hosts+="\ttv.sohu.com\r\n";
	
	WCHAR szPath[MAX_PATH];
	GetWindowsDirectoryW( szPath, sizeof(szPath) );
	string sysdir=wstr2str(szPath);
	sysdir+="\\system32\\drivers\\etc\\hosts";

	if (SetFileAttributes(sysdir.c_str(),FILE_ATTRIBUTE_NORMAL))
	{
		HANDLE hFile = CreateFile(sysdir.c_str(), GENERIC_ALL, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);   
		if(hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwReaded;
			unsigned int start_pos=0,end_pos=0;
			string str_hosts;

			DWORD dwDataSize = GetFileSize(hFile,NULL);
			char * tmp_hosts=new char[dwDataSize+2];
			ZeroMemory(tmp_hosts,dwDataSize+2);
			if(!ReadFile(hFile,tmp_hosts,dwDataSize,&dwReaded,NULL))
			{
				Msg("读取文件时发生错误，错误代码:%ld\r\n",GetLastError());
				return;
			}
			str_hosts=tmp_hosts;
			delete [] tmp_hosts;
			start_pos=str_hosts.find("_SinSoul");
			if (start_pos!=string::npos)
			{
				start_pos+=10;
				end_pos=str_hosts.find("tv.sohu.com")+11;
				str_hosts.erase(start_pos,end_pos);
				str_hosts+=hosts;
				CloseHandle(hFile);			//重建HOSTS文件
				hFile=CreateFile(sysdir.c_str(),GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
				DWORD dwWritten;
				if (!WriteFile(hFile,str_hosts.c_str(),str_hosts.length(),&dwWritten,NULL))
				{
					Msg("写入重建的hosts文件时发生错误，错误代码:%ld\r\n",GetLastError());
					return;
				}
			}
			else
			{
				SetFilePointer(hFile,0, NULL, FILE_END);
				DWORD dwWritten;
				if (!WriteFile(hFile,hosts_title,strlen(hosts_title),&dwWritten,NULL))
				{
					Msg("写入hosts文件时发生错误，错误代码:%ld\r\n",GetLastError());
					return;
				}
				WriteFile(hFile,hosts.c_str(),hosts.length(),&dwWritten,NULL);
			}
			CloseHandle(hFile);
			HINSTANCE hdllInst=LoadLibrary("dnsapi.dll");
			FARPROC pFlushDNS=GetProcAddress(hdllInst,"DnsFlushResolverCache");
			pFlushDNS();
			MessageBox(hwnd,"补丁完成，可以直接使用浏览器访问校园视频网站了.","域名补丁成功",0);
			ShellExecute(NULL, NULL, "iexplore", "http://mov.csonline.com.cn/2010index.html" ,NULL, SW_SHOW);
			return;
		}
	}
	Msg("修改Hosts文件失败，请使用管理员权限运行本程序，并在杀毒软件中允许本程序的修改.\r\n");
}

void Msg(TCHAR *szFormat, ...)
{
	TCHAR szBuffer[1024];
	const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
	const int LASTCHAR = NUMCHARS - 1;
	va_list pArgs;
	va_start(pArgs, szFormat);
	(void)StringCchVPrintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
	va_end(pArgs);
	szBuffer[LASTCHAR] = TEXT('\0');
	int nTextLength=GetWindowTextLength(hwndLable_0);
	SendMessage (hwndLable_0, EM_SETSEL, (WPARAM)nTextLength, (LPARAM)nTextLength);
	SendMessage (hwndLable_0, EM_REPLACESEL, 0, (LPARAM) ((LPSTR) szBuffer));
}

void ProgressMsg(TCHAR *szFormat, ...)
{
	TCHAR szBuffer[1024];
	const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
	const int LASTCHAR = NUMCHARS - 1;
	va_list pArgs;
	va_start(pArgs, szFormat);
	(void)StringCchVPrintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
	va_end(pArgs);
	szBuffer[LASTCHAR] = TEXT('\0');
	SetWindowText(hwndLable_1,"");
	//	SendMessage (hwndLable_1, EM_SETSEL,0, 0);
	SendMessage (hwndLable_1, EM_REPLACESEL, 0, (LPARAM) ((LPSTR) szBuffer));
}

void CleanMsg()
{
	SetWindowText(hwndLable_0,"");
	UpdateWindow(hwndLable_0);
}


HWND CreateListView (HWND hwndParent, LPSTR szWindowName)
{
	HWND hWndListView;
	//hWndListView = CreateWindow (WC_LISTVIEW,szWindowName,
	//	WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
	//	CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
	//	hwndParent,NULL, hinst, NULL);

	hWndListView = CreateWindowEx(WS_EX_CLIENTEDGE,"SysListView32",NULL,LVS_REPORT|WS_CHILD|WS_VISIBLE| LVS_EDITLABELS
		,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,hwnd,(struct HMENU__ *)0,hinst,NULL);
	if (hWndListView == NULL)
	{
		Msg("创建列表控制失败\r\n");
		return NULL;
	}
	if(InitListViewColumns(hWndListView) )
	{
		return hWndListView;
	}
	return false;
}

bool InitListViewColumns(HWND hWndListView)
{
	char szText[256]; 
	LVCOLUMN lvc;
	LPSTR ColNames[] = {"视频名称","导演","主演","频道","ID"};
	int Collong[]={200,100,200,100,86};
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.pszText = szText;
	lvc.iImage = 1;
	lvc.fmt = LVCFMT_LEFT;	

	for (int index=0;index<5;index++)
	{
		lvc.cx =Collong[index];	
		lvc.pszText = ColNames[index];
		lvc.iSubItem = 0;
		if (ListView_InsertColumn(hWndListView, index,	&lvc) == -1)
		{
			return false;
		}

	}

	return true;
}

bool AddListViewItems(HWND hwndListView,DWORD index,char *videoname,char *videodirector,char *videoactor,
	char *videochannel,char *videoid)
{
	LVITEM lvI;
	ZeroMemory (&lvI,sizeof(lvI));
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvI.state = 0;
	lvI.stateMask = 0;
	lvI.iItem = index;
	lvI.iImage = 0;
	lvI.iSubItem = 0;

	lvI.pszText = videoname; 
	lvI.cchTextMax = lstrlen(lvI.pszText)+1;

	if(ListView_InsertItem(hwndListView, &lvI) == -1)
	{
		return false;
	}

	ListView_SetItemText(hwndListView,index, 1, videodirector);
	ListView_SetItemText(hwndListView,index, 2, videoactor);
	ListView_SetItemText(hwndListView,index, 3, videochannel);
	ListView_SetItemText(hwndListView,index, 4, videoid);
	return true;
}

int GetIndex(HWND hList)
{
	int i, n;
	n = ListView_GetItemCount(hList);
	for (i = 0; i < n; i++)
	{
		if (ListView_GetItemState(hList, i, LVIS_FOCUSED) == LVIS_FOCUSED)
		{
			return i;
		}
	}
	return -1;
}

VOID CleanListView()
{
	ListView_DeleteAllItems(hListView);
	totalresult=0;
}

#define FILE_MAP_START 0x0

int hunter_decoder(char *file_name)
{
	HANDLE hMapFile,hFile;
	DWORD dwFileSize,dwFileMapSize,dwMapViewSize,dwFileMapStart,dwSysGran;
	SYSTEM_INFO SysInfo;
	BYTE *lpMapBuffer;
	char error_msg[1024]={0};

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(file_name,&fd);
	FindClose(hFind);
	if (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
	{
		sprintf_s(error_msg,"%s是个文件夹，请不要尝试哄骗本猎手！！！",file_name);
		MessageBox(hwnd,error_msg,"视频猎手错误",MB_OK);
		return 0;
	}

	hFile = CreateFile(file_name,GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		sprintf_s(error_msg,"不能打开文件: %s\n该文件可能正在被其它程序使用。",file_name);
		MessageBox(hwnd,error_msg,"视频猎手错误",MB_OK);
		return 0;
	}
	dwFileSize = GetFileSize(hFile,NULL);
//	printf("File :%s\nFile Size:0x%.8x\n",file_name,dwFileSize);
	if (dwFileSize<0xa0)
	{
		sprintf_s(error_msg,"%s也太小了吧，根本不可能是视频，请不要尝试哄骗本猎手！！！",file_name);
		MessageBox(hwnd,error_msg,"视频猎手错误",MB_OK);
		return 0;
	}
	GetSystemInfo(&SysInfo);
	dwSysGran = SysInfo.dwAllocationGranularity;
	dwFileMapStart = (FILE_MAP_START / dwSysGran) * dwSysGran;
	dwMapViewSize = (FILE_MAP_START % dwSysGran) + dwFileSize;
	dwFileMapSize = FILE_MAP_START + dwFileSize;

	hMapFile = CreateFileMapping( hFile,NULL,PAGE_READWRITE,0,dwFileMapSize,NULL);
	if (hMapFile == NULL) 
	{
		sprintf_s(error_msg,"创建内在映射文件出错，错误代码: CreateFileMapping %d\n", GetLastError());
		MessageBox(hwnd,error_msg,"视频猎手错误",MB_OK);
		return 0;
	}

	lpMapBuffer = (BYTE *)MapViewOfFile(hMapFile,FILE_MAP_ALL_ACCESS,0,dwFileMapStart,dwMapViewSize);
	if (lpMapBuffer == NULL) 
	{
		sprintf_s(error_msg,"创建内在映射文件出错，错误代码: MapViewOfFile %d\n", GetLastError());
		MessageBox(hwnd,error_msg,"视频猎手错误",MB_OK);
		return 0;
	}

//	printf ("Map View Start:0x%.8x\nMap View Size :0x%.8x\n",dwFileMapStart,dwMapViewSize);

	int i=0; 
	while(i<0xa0)
	{
		*(lpMapBuffer+i++)^=0xff;
	}
	CloseHandle(hFile);
	CloseHandle(hMapFile);
	UnmapViewOfFile(lpMapBuffer);
//	printf("Decryption is complete.\n\n");
	return 1;
}
