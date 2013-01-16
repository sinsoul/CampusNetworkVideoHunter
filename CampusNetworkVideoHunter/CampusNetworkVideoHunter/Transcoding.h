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


