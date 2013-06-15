//Author:SinSoul
//E-mail:nh6080@gmail.com
//Function:用于从西华师范大学校园网上下载视频

#include "MyStd.h"
#include <Gdiplus.h>
#include <GdiPlusEnums.h>
#pragma comment(lib,"gdiplus") 

#import "G:\\Independent\\Thunder 7\\BHO\\ThunderAgent.dll" 
using namespace ThunderAgentLib;

using namespace std;

extern char *server_addr;
extern char szClassName[21 ];
extern char *wndName;
extern string conf_path;
extern bool b_Proxy;
extern HWND hwndButton;
extern HWND hwndEdit; 
extern HWND hwndLable_0,hwndLable_1;
extern HWND hwnd; 
extern bool conn_status;
extern string *TotalXML;
extern HWND hListView;
extern HINSTANCE hinst;

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
bool OnChildWindowsNotify(PVOID pParam);
LRESULT CALLBACK SetServerAddr(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK AboutApplication(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);

void SearchVideoKeyWord(void *txml);
void GetVideoAddress(void *videoid);
void GetVideoSummary(char *videoid);
void LoadTotalXML(void *TotalXML);
void ptrGetVideoID(string *TotalXML,char *moviename);
void SaveCustomServerIP(char *pCustomIP,char *pSchoolName);
void GetCustomServerIP();

void Msg(TCHAR *szFormat, ...);
void ProgressMsg(TCHAR *szFormat, ...);
void CleanMsg();



HWND CreateListView (HWND hwndParent, LPSTR szWindowName);
bool AddListViewItems(HWND hwndListView,DWORD index,char *videoname,char *videodirector,char *videoactor,char *videochannel,char *videoid);
bool InitListViewColumns(HWND hWndListView);
int GetIndex(HWND hList);
void CleanListView();

bool CallThunder();
bool ProxyDownload();
void RecoveryThunder();
bool CreateListAndPlay();
void DomainPatch();
void UpdateApp(void *temp);

bool EnableAero(HWND hwnd_aero);
bool ImageFromIDResource(UINT nID, LPCTSTR sTR,IStream* &pstm);
void GDIPlusDrawImage(HDC hdc,UINT SourceID,LPCTSTR SourceDIR,Gdiplus::REAL leftx,Gdiplus::REAL lefty,Gdiplus::REAL rightx,Gdiplus::REAL righty);
void GDIPlusDrawText(HDC hdc,PWCHAR string,Gdiplus::REAL x_point,Gdiplus::REAL y_point,PWCHAR userFont,Gdiplus::REAL fontSize,Gdiplus::Color pColor);

#pragma comment(lib,"Comctl32.lib")
//#pragma comment(lib,"Strsafe.lib")
//#pragma comment(linker,"/SUBSYSTEM:Windows")