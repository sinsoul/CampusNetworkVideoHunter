//Author:SinSoul
//E-mail:nh6080@gmail.com
//Function:一些字符串处理函数
#include <iostream>
using namespace std;

wchar_t * MBCS2Unicode(wchar_t * buff, const char * str)
{
	wchar_t * wp = buff;
	char * p = (char *)str;
	while(*p)
	{
		if(*p & 0x80)
		{
			*wp = *(wchar_t *)p;
			p++;
		}
		else{
			*wp = (wchar_t) *p;
		}
		wp++;
		p++;
	}
	*wp = 0x0000;
	return buff;
}

char * Unicode2MBCS(char * buff, const wchar_t * str)
{
	wchar_t * wp = (wchar_t *)str;
	char * p = buff, * tmp;
	while(*wp){
		tmp = (char *)wp;
		if(*wp & 0xFF00){
			*p = *tmp;
			p++;tmp++;
			*p = *tmp;
			p++;
		}
		else{
			*p = *tmp;
			p++;
		}
		wp++;
	}
	*p = 0x00;
	return buff;
}

wstring str2wstr(string str)
{
	size_t len = str.size();
	wchar_t * b = (wchar_t *)malloc((len+1)*sizeof(wchar_t));
	MBCS2Unicode(b,str.c_str());
	wstring r(b);
	free(b);
	return r;
}

string wstr2str(wstring wstr)
{
	size_t len = wstr.size();
	char * b = (char *)malloc((2*len+1)*sizeof(char));
	Unicode2MBCS(b,wstr.c_str());
	string r(b);
	free(b);
	return r;
}


int wputs(const wchar_t * wstr)
{
	int len = wcslen(wstr);
	char * buff = (char *)malloc((len * 2 + 1)*sizeof(char));
	Unicode2MBCS(buff,wstr);
	Msg("%s",buff);
	free(buff);
	return 0;
}

string strconver(char *str)
{
	string strnew;
	for (unsigned int i=0;i<strlen(str);i++)
	{
		if (i%2==0)
		{
			strnew+="\\x";
		}
		strnew+=str[i];
	}
	return strnew;
}

//值传递方式，用于调试
//void GetVideoID(string TotalXML,char *moviename)
//{
//	int current_loc=0,end_loc=0,forward_loc=0,right_loc=0,left_loc=0,last_loc=0;
//	do 
//	{
//		end_loc=0;
//		forward_loc=0;
//		right_loc=0;
//		left_loc=0;
//		current_loc=TotalXML.find(moviename,current_loc);
//		if (current_loc==string::npos)
//		{
//			break;
//		}
//		cout<<"在"<<current_loc<<"处发现查找内容"<<endl;
//
//		right_loc=TotalXML.rfind(">",current_loc);
//
//		//关键字不在视频名中，则向前搜索
//		if (TotalXML[right_loc-1]!='a')
//		{
//			left_loc=TotalXML.rfind("<",right_loc);
//			forward_loc=TotalXML.rfind("<b>",left_loc);
//			forward_loc+=3;
//			//关键字在同一视频的名字和简介中可能多次出现，进行过滤
//			if (forward_loc==string::npos||last_loc==forward_loc)
//			{
//				current_loc+=strlen(moviename);
//				continue;
//			}
//
//			last_loc=forward_loc;
//			end_loc=TotalXML.find("</b>",forward_loc);
//
//			char *cmdid=new char[end_loc-forward_loc+2];
//			ZeroMemory(cmdid,end_loc-forward_loc+2);
//			memcpy(cmdid,&TotalXML[forward_loc],end_loc-forward_loc);
//			cout<<"发现ID："<<cmdid<<endl;
//			current_loc+=strlen(moviename);
//		}
//		else
//		{
//			current_loc=TotalXML.find("<b>",current_loc);
//			current_loc+=3;//strlen("<b>");
//			if (current_loc==string::npos)
//			{
//				printf("未找到电影ID");
//				continue;
//			}
//			end_loc=TotalXML.find("</b>",current_loc);
//			char *cmdid=new char[end_loc-current_loc+2];
//			ZeroMemory(cmdid,end_loc-current_loc+2);
//			memcpy(cmdid,&TotalXML[current_loc],end_loc-current_loc);
//			cout<<"发现ID："<<cmdid<<endl;
//		}
//	} while (true);
//}

//void GetVideoName(char *videoid)
//{
//	string ResourcePath;
//	HttpMessenger *hm_GetVideoName=new HttpMessenger(server_addr);
//	if (!hm_GetVideoName->CreateConnection())
//	{
//		printf("连接结果提交服务器失败，错误日志:%s\r\n",(char *)hm_GetVideoName->GetErrorLog().c_str());
//		delete[] hm_GetVideoName;
//		return;
//	}
//	ResourcePath="/mov/";
//	ResourcePath+=videoid;
//	ResourcePath+="/url.xml";
//	if (!hm_GetVideoName->CreateAndSendRequest("GET",(char *)ResourcePath.c_str(),server_addr))
//	{
//		printf("提交命令结果失败，错误日志:\r\n%s",hm_GetVideoName->GetErrorLog().c_str());
//		delete[] hm_GetVideoName;
//		return;
//	}
//}

