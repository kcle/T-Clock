/*-------------------------------------------
  utl.c - KAZUBON 1997-1999
---------------------------------------------*/
// Modified by Stoic Joker: Monday, 03/22/2010 @ 7:32:29pm
#include "globals.h"
#include "utl.h"
//#include <sys/types.h>
#include <sys/stat.h>
const char mykey[] = "Software\\Stoic Joker's\\T-Clock 2010";

BOOL bV7up=TRUE;
BOOL b2000=TRUE;
char g_bIniSetting = 0;
char g_inifile[MAX_PATH];
//================================================================================================
//---------------------------//-----+++--> Find Out If it's Older Then Windows 2000 If it is, Die!:
BOOL CheckSystemVersion()   //--------------------------------------------------------------+++-->
{
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if(!GetVersionEx((OSVERSIONINFO*) &osvi)) return FALSE;
	if(osvi.dwMajorVersion >= 6) bV7up = TRUE;
	if((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 0)) b2000 = TRUE;
	if(osvi.dwMajorVersion >= 5) return TRUE;
	return FALSE;
}
//================================================================================================
//--------------------------------------------------+++--> Force a ReDraw of T-Clock & the TaskBar:
void RefreshUs(void)   //-------------------------------------------------------------------+++-->
{
	char classname[GEN_BUFF] = {0};
	HWND hwndBar, hwndChild;
	
	hwndBar = FindWindow("Shell_TrayWnd", NULL);
	
	// find the clock window
	hwndChild = GetWindow(hwndBar, GW_CHILD);
	while(hwndChild) {
		GetClassName(hwndChild, classname, 80);
		if(lstrcmpi(classname, "TrayNotifyWnd") == 0) {
			hwndChild = GetWindow(hwndChild, GW_CHILD);
			while(hwndChild) {
				GetClassName(hwndChild, classname, 80);
				if(lstrcmpi(classname, "TrayClockWClass") == 0) {
					SendMessage(hwndChild, CLOCKM_REFRESHCLOCK, 0, 0);
					SendMessage(hwndChild, CLOCKM_REFRESHTASKBAR, 0, 0);
					break;
				}
			} break;
		}
		hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
	}
}
#ifndef S_ISDIR
#	define S_ISDIR(mode) (mode&S_IFDIR)
#endif // S_ISDIR
char PathExists(const char* path){
	struct stat st;
	if(stat(path,&st)==-1) return 0;
	return S_ISDIR(st.st_mode)?2:1;
}
/*--------------------------------------------------------
  Retreive a file name and option from a command string
----------------------------------------------------------*/
void GetFileAndOption(const char* command, char* fname, char* opt)
{
	const char* p, *pe;
	char* pd;
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	
	p = command; pd = fname;
	pe = NULL;
	for(; ;) {
		if(*p == ' ' || *p == 0) {
			*pd = 0;
			hfind = FindFirstFile(fname, &fd);
			if(hfind != INVALID_HANDLE_VALUE) {
				FindClose(hfind);
				pe = p;
			}
			if(*p == 0) break;
		}
		*pd++ = *p++;
	}
	if(pe == NULL) pe = p;
	
	p = command; pd = fname;
	for(; p != pe;) {
		*pd++ = *p++;
	}
	*pd = 0;
	if(*p == ' ') p++;
	
	pd = opt;
	for(; *p;) *pd++ = *p++;
	*pd = 0;
}
/*------------------------------------------------
  Open a file
--------------------------------------------------*/
BOOL ExecFile(HWND hwnd, const char* command)
{
	char fname[MAX_PATH], opt[MAX_PATH];
	if(*command){
		GetFileAndOption(command,fname,opt);
		if((intptr_t)ShellExecute(hwnd,NULL,fname,*opt?opt:NULL,NULL,SW_SHOWNORMAL)>32)
			return TRUE;
	}
	return FALSE;
}
void ToggleCalendar()
{
	if(FindWindowEx(NULL,NULL,"ClockFlyoutWindow",NULL))
		return;
	if(g_tos>=TOS_VISTA && !GetMyRegLong("Calendar","bCustom",0))
		PostMessage(g_hwndClock,WM_USER+102,1,0);//1=open, 0=close
	else
		ExecFile(g_hwndClock,"XPCalendar.exe");
}

int atox(const char* p)
{
	int r = 0;
	while(*p) {
		if('0' <= *p && *p <= '9') r = r * 16 + *p - '0';
		else if('A' <= *p && *p <= 'F') r = r * 16 + *p - 'A' + 10;
		else if('a' <= *p && *p <= 'f') r = r * 16 + *p - 'a' + 10;
		p++;
	}
	return r;
}

__inline int toupper(int c)
{
	if('a' <= c && c <= 'z') c -= 'a' - 'A';
	return c;
}

void add_title(char* path, const char* title)
{
	char* p;
	
	p = path;
	
	if(*p == 0) ;
	else if(*title && *(title + 1) == ':') ;
	else if(*title == '\\') {
		if(*p && *(p + 1) == ':') p += 2;
	} else {
		while(*p) {
			if((*p == '\\' || *p == '/') && *(p + 1) == 0) {
				break;
			}
			p = CharNext(p);
		}
		*p++ = '\\';
	}
	while(*title) *p++ = *title++;
	*p = 0;
}

void del_title(char* path)
{
	char* p, *ep;
	
	p = ep = path;
	while(*p) {
		if(*p == '\\' || *p == '/') {
			if(p > path && *(p - 1) == ':') ep = p + 1;
			else ep = p;
		}
		p = CharNext(p);
	}
	*ep = 0;
}

void get_title(char* dst, const char* path)
{
	const char* p, *ep;
	
	p = ep = path;
	while(*p) {
		if(*p == '\\' || *p == '/') {
			if(p > path && *(p - 1) == ':') ep = p + 1;
			else ep = p;
		}
		p = CharNext(p);
	}
	
	if(*ep == '\\' || *ep == '/') ep++;
	
	while(*ep) *dst++ = *ep++;
	*dst = 0;
}

int ext_cmp(const char* fname, const char* ext)
{
	const char* p, *sp;
	
	sp = NULL; p = fname;
	while(*p) {
		if(*p == '.') sp = p;
		else if(*p == '\\' || *p == '/') sp = NULL;
		p = CharNext(p);
	}
	
	if(sp == NULL) sp = p;
	if(*sp == '.') sp++;
	
	for(;;) {
		if(*sp == 0 && *ext == 0) return 0;
		if(toupper(*sp) != toupper(*ext))
			return (toupper(*sp) - toupper(*ext));
		sp++; ext++;
	}
}
/*
void parse(char* dst, char* src, int n)
{
	char* dp;
	int i;
	
	for(i = 0; i < n; i++) {
		while(*src && *src != ',') src++;
		if(*src == ',') src++;
	}
	if(*src == 0) {
		*dst = 0; return;
	}
	
	while(*src == ' ') src++;
	
	dp = dst;
	while(*src && *src != ',') *dst++ = *src++;
	*dst = 0;
	
	while(dst != dp) {
		dst--;
		if(*dst == ' ') *dst = 0;
		else break;
	}
}// */
/*
void parsechar(char* dst, char* src, char ch, int n)
{
	char* dp;
	int i;
	
	for(i = 0; i < n; i++) {
		while(*src && *src != ch) src++;
		if(*src == ch) src++;
	}
	if(*src == 0) {
		*dst = 0; return;
	}
	
	while(*src == ' ') src++;
	
	dp = dst;
	while(*src && *src != ch) *dst++ = *src++;
	*dst = 0;
	
	while(dst != dp) {
		dst--;
		if(*dst == ' ') *dst = 0;
		else break;
	}
}// */

void str0cat(char* dst, const char* src)
{
	char* p;
	p = dst;
	while(*p) { while(*p) p++; p++; }
	strcpy(p, src);
	while(*p) p++; p++; *p = 0;
}

/*---------------------------------------------
--------------------- returns a resource string
---------------------------------------------*/
char* MyString(UINT id)
{
	static char buf[80];
	
	buf[0] = 0;
	LoadString(GetModuleHandle(NULL), id, buf, 80);
	return buf;
}

int MyMessageBox(HWND hwnd, const char* msg, const char* title, UINT uType, UINT uBeep)
{
	MSGBOXPARAMS mbp;
	memset(&mbp, 0, sizeof(MSGBOXPARAMS));
	mbp.cbSize = sizeof(MSGBOXPARAMS);
	mbp.hwndOwner = hwnd;
	mbp.hInstance = GetModuleHandle(NULL);
	mbp.lpszText = msg;
	mbp.lpszCaption = title;
	mbp.dwStyle = MB_USERICON | uType;
	mbp.lpszIcon = MAKEINTRESOURCE(IDI_MAIN);
	mbp.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
	if(uBeep != 0xFFFFFFFF) MessageBeep(uBeep);
	return MessageBoxIndirect(&mbp);
}
//================================================================================================
//----------------------------------------//--------------------------+++--> 32bit x 32bit = 64bit:
DWORDLONG M32x32to64(DWORD a, DWORD b)   //-------------------------------------------------+++-->
{
	ULARGE_INTEGER r;
	DWORD* p1, *p2, *p3;
	memset(&r, 0, 8);
	p1 = &r.LowPart;
	p2 = (DWORD*)((BYTE*)p1 + 2);
	p3 = (DWORD*)((BYTE*)p2 + 2);
	*p1 = LOWORD(a) * LOWORD(b);
	*p2 += LOWORD(a) * HIWORD(b) + HIWORD(a) * LOWORD(b);
	*p3 += HIWORD(a) * HIWORD(b);
	return *(DWORDLONG*)(&r);
}
//================================================================================================
//-----------+++--> (Momentarily) Force Window X Into the Foreground So we Interact With IT (only):
//void ForceForegroundWindow(HWND hWnd)   //---{ Required to Dismiss Context Menu Properly }--+++-->
//{
//	DWORD thread1, thread2, pid;
//	
//	thread1 = GetWindowThreadProcessId(GetForegroundWindow(), &pid);
//	thread2 = GetCurrentThreadId();
//	
//	AttachThreadInput(thread2, thread1, TRUE);
//	SetForegroundWindow(hWnd);
//	
//	AttachThreadInput(thread2, thread1, FALSE);
//	BringWindowToTop(hWnd);
//}
#include <stddef.h>
void ForceForegroundWindow(HWND hwnd)
{
	DWORD fgthread=GetWindowThreadProcessId(GetForegroundWindow(),0);
	if(fgthread && _threadid^fgthread && AttachThreadInput(_threadid,fgthread,1)){
//		AllowSetForegroundWindow(ASFW_ANY);//does nothing... we become foreground, but won't receive window messages
//		SetFocus(hwnd);// "
		SetForegroundWindow(hwnd);
		BringWindowToTop(hwnd);
		AttachThreadInput(_threadid,fgthread,0);
		return;
	}
	SetForegroundWindow(hwnd);
	BringWindowToTop(hwnd);
}
//===============================================================================
//--+++-->
int GetMyRegStr(const char* section, const char* entry, char* val, int len, const char* defval)
{
	HKEY hkey;	char key[80];	DWORD regtype, size;	int r=0;
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);
	
	if(section && *section) {
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	} else {
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting) {
		r = GetPrivateProfileString(key, entry, defval, val,
									len, g_inifile);
	} else {
		if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey)==ERROR_SUCCESS) {
			size = len;
			if(RegQueryValueEx(hkey, entry, 0, &regtype, (LPBYTE)val, &size) == ERROR_SUCCESS) {
				r=size;
				if(r) --r;
			}else{
				if((r=(int)strlen(defval))<=len){
					strcpy(val,defval);
				}else{
					r=0;
				}
			}
			RegCloseKey(hkey);
		}
	}
	if(!r) *val='\0';
	return r;
}

int GetMyRegStrEx(const char* section, const char* entry, char* val, int len, const char* defval)
{
	HKEY hkey;	char key[80];	DWORD regtype, size;	int r=0;
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);
	
	if(section && *section) {
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	} else {
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting) {
		r = GetPrivateProfileString(key, entry, defval, val,
									len, g_inifile);
		if(r == len)
			SetMyRegStr(section, entry, defval);
	} else {
		if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey)==ERROR_SUCCESS) {
			size = len;
			if(RegQueryValueEx(hkey, entry, 0, &regtype, (LPBYTE)val, &size) == ERROR_SUCCESS) {
				r=size;
				if(r) --r;
			}else{
				if((r=(int)strlen(defval))<=len){
					SetMyRegStr(section, entry, defval);
					strcpy(val,defval);
				}else{
					r=0;
				}
			}
			RegCloseKey(hkey);
		}
	}
	if(!r) *val='\0';
	return r;
}

LONG GetMyRegLong(const char* section, const char* entry, LONG defval)
{
	char key[80];
	HKEY hkey;
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);
	
	if(section && *section) {
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	} else {
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting) {
		return GetPrivateProfileInt(key, entry, defval, g_inifile);
	} else {
		if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == 0) {
			DWORD regtype,size=sizeof(LONG);
			LONG dw=0;
			if(RegQueryValueEx(hkey,entry,0,&regtype,(LPBYTE)&dw,&size)==ERROR_SUCCESS && regtype==REG_DWORD)
				defval=dw;
			RegCloseKey(hkey);
		}
	}
	return defval;
}

LONG GetMyRegLongEx(const char* section, const char* entry, LONG defval)
{
	char key[80];
	HKEY hkey;
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);
	
	if(section && *section) {
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	} else {
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting) {
		return GetPrivateProfileInt(key,entry,defval,g_inifile);
	} else {
		if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == 0) {
			DWORD regtype,size=sizeof(LONG);
			LONG dw=0;
			if(RegQueryValueEx(hkey,entry,0,&regtype,(LPBYTE)&dw,&size)==ERROR_SUCCESS && regtype==REG_DWORD){
				defval=dw;
			}else{
				SetMyRegLong(section,entry,defval);
			}
			RegCloseKey(hkey);
		}
	}
	return defval;
}

/*------------------------------------------------
  get DWORD value from registry
--------------------------------------------------*/
/*
LONG GetRegLong(HKEY rootkey, char* section, char* entry, LONG defval)
{
	HKEY hkey;	DWORD regtype, size;	BOOL b = FALSE;	int r=0;
	
	if(RegOpenKey(rootkey, section, &hkey) == ERROR_SUCCESS) {
		size = 4;
		if(RegQueryValueEx(hkey, entry, 0, &regtype, (LPBYTE)&r, &size) == ERROR_SUCCESS) {
			if(size == 4) b = TRUE;
		}
		RegCloseKey(hkey);
	}
	if(b == FALSE) r = defval;
	return r;
}// */

int GetRegStr(HKEY rootkey, const char* section, const char* entry, char* val, int len, const char* defval)
{
	HKEY hkey;	DWORD regtype, size;	BOOL b = FALSE;	int r=0;
	
	if(RegOpenKey(rootkey, section, &hkey) == ERROR_SUCCESS) {
		size = len;
		if(RegQueryValueEx(hkey, entry, 0, &regtype, (LPBYTE)val, &size) == ERROR_SUCCESS) {
			if(size == 0) *val = 0;
			b = TRUE;
		}
		RegCloseKey(hkey);
	}
	
	if(b == FALSE) {
		strcpy(val, defval);
		r = (int)strlen(defval);
	}
	return r;
}

BOOL SetMyRegStr(const char* section, const char* entry, const char* val)
{
	HKEY hkey;	char key[80];	BOOL r=FALSE;
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);
	
	if(section && *section) {
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	} else {
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting) {
		r = FALSE;
		if(WritePrivateProfileString(key, entry, val, g_inifile))
			r = TRUE;
	} else {
		if(RegCreateKey(HKEY_CURRENT_USER, key, &hkey) == ERROR_SUCCESS) {
			if(RegSetValueEx(hkey, entry, 0, REG_SZ, (CONST BYTE*)val, (DWORD)(int)strlen(val)) == ERROR_SUCCESS) {
				r = TRUE;
			}
			RegCloseKey(hkey);
		}
	}
	return r;
}
/*
BOOL SetRegStr(HKEY rootkey, char* section, char* entry, char* val)
{
	HKEY hkey;	BOOL r = FALSE;
	
	if(RegCreateKey(rootkey, section, &hkey) == ERROR_SUCCESS) {
		if(RegSetValueEx(hkey, entry, 0, REG_SZ, (CONST BYTE*)val, (DWORD)(int)strlen(val)) == ERROR_SUCCESS) {
			r = TRUE;
		}
		RegCloseKey(hkey);
	}
	return r;
}// */

BOOL SetMyRegLong(const char* section, const char* entry, LONG val)
{
	HKEY hkey;
	BOOL r;
	char key[80];
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);
	
	if(section && *section) {
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	} else {
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting) {
		char s[20];
		wsprintf(s, "%d", val);
		r = FALSE;
		if(WritePrivateProfileString(key, entry, s, g_inifile))
			r = TRUE;
	} else {
		r = FALSE;
		if(RegCreateKey(HKEY_CURRENT_USER, key, &hkey) == 0) {
			if(RegSetValueEx(hkey,entry,0,REG_DWORD,(CONST BYTE*)&val,4)==ERROR_SUCCESS) {
				r = TRUE;
			}
			RegCloseKey(hkey);
		}
	}
	return r;
}

BOOL DelMyReg(const char* section, const char* entry)
{
	HKEY hkey;	char key[80];	BOOL r = FALSE;
	
	strcpy(key, mykey);
	
	if(section && *section) {
		strcat(key, "\\");
		strcat(key, section);
	}
	
	if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == ERROR_SUCCESS) {
		if(RegDeleteValue(hkey, entry) == ERROR_SUCCESS) r = TRUE;
		RegCloseKey(hkey);
	}
	return r;
}

BOOL DelMyRegKey(const char* section)
{
	char key[80];	BOOL r = FALSE;
	
	strcpy(key, mykey);
	
	if(section && *section) {
		strcat(key, "\\");
		strcat(key, section);
	}
	
	if(RegDeleteKey(HKEY_CURRENT_USER, key) == ERROR_SUCCESS) r = TRUE;
	return r;
}

/*
void Pause(HWND hWnd, LPCTSTR pszArgs)
{
	LONG lInterval = atoi(pszArgs);
	LONG lTime = GetTickCount();
	MSG msg;
	
	if(lInterval > 0) {
		while((LONG)(GetTickCount() - lTime) < lInterval) {
			if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
}// */
/// currently unused drawing stuff
/*
// tile an image vertically
void VerticalTileBlt(HDC hdcDest, int xDest, int yDest, int cxDest, int cyDest,
					 HDC hdcSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
					 BOOL ReverseBlt, BOOL useTrans)
{
	int y;
	
	if(ReverseBlt) {
		for(y = cyDest - cySrc; y > yDest - cySrc; y -= cySrc) {
			TC2DrawBlt(hdcDest,
					   xDest,
					   y,
					   cxDest,
					   cySrc,
					   hdcSrc,
					   xSrc,
					   ySrc,
					   cxSrc,
					   cySrc,
					   useTrans);
		}
	} else {
		for(y = 0; y < cyDest; y += cySrc) {
			TC2DrawBlt(hdcDest,
					   xDest,
					   yDest + y,
					   cxDest,
					   __min(cyDest - y, cySrc),
					   hdcSrc,
					   xSrc,
					   ySrc,
					   cxSrc,
					   __min(cyDest - y, cySrc),
					   useTrans);
		}
	}
}
// tile an image horizontally and vertically
void FillTileBlt(HDC hdcDest, int xDest, int yDest, int cxDest, int cyDest, HDC hdcSrc, int xSrc, int ySrc, int cxSrc, int cySrc, DWORD rasterOp)
{
	int x, y;
	
	for(y = 0; y < cyDest; y += cySrc) {
		for(x = 0; x < cxDest; x += cxSrc) {
			BitBlt(hdcDest,
				   xDest + x,
				   yDest + y,
				   cxSrc,
				   cySrc,
				   hdcSrc,
				   xSrc,
				   ySrc,
				   rasterOp);
		}
	}
}
void TileBlt(HDC hdcDest, int xDest, int yDest, int cxDest, int cyDest, HDC hdcSrc,
			 int xSrc, int ySrc, int cxSrc, int cySrc, BOOL useTrans)
{
	int y, x;
	
	for(y = yDest; y < cyDest; y = y + cySrc) {
		for(x = xDest; x < cxDest; x = x + cxSrc) {
			TC2DrawBlt(hdcDest, x, y, cxSrc, cySrc,
					   hdcSrc, xSrc, ySrc, cxSrc, cySrc, useTrans);
		}
	}
}// */
BOOL IsXPStyle()
{
	char temp[1024];
	
	GetRegStr(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager", "ThemeActive", temp, 1024, "0");
	if(_strnicmp(temp, "1", 1) == 0) return TRUE;
	else return FALSE;
}
/*
void Pause(HWND hWnd, LPCTSTR pszArgs)
{
	LONG lInterval = atoi(pszArgs);
	LONG lTime = GetTickCount();
	MSG msg;
	
	if(lInterval > 0) {
		while((LONG)(GetTickCount() - lTime) < lInterval) {
			if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
}// */