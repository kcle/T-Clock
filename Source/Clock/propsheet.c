/*-------------------------------------------
  propsheet.c - Kazubon 1997-2001
  show "properties for TClock"
---------------------------------------------*/
// Modified by Stoic Joker: Monday, 03/22/2010 @ 7:32:29pm
#include "tclock.h"

#define IDC_TAB					0x3020
#define ID_APPLY_NOW			0x3021
#define ID_WIZBACK				0x3023
#define ID_WIZNEXT				0x3024
#define ID_WIZFINISH			0x3025

static int CALLBACK PropSheetProc(HWND hDlg, UINT uMsg, LPARAM  lParam);
static LRESULT CALLBACK SubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// dialog procedures of each page
INT_PTR CALLBACK PageAboutProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageAlarmProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageMouseProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageColorProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageQuickyProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageFormatProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageHotKeyProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageMiscProc(HWND, UINT, WPARAM, LPARAM);

static WNDPROC m_oldWndProc; // to save default window procedure
static int m_startpage = 0; // page to open first

char g_bApplyTaskbar = 0;
char g_bApplyClock = 0;
//char g_bApplyClear = 0;

//================================================================================================
//----------------------------//-----------------------+++--> Show the (Tab Dialog) Property Sheet:
void MyPropertySheet(int page)   //---------------------------------------------------------+++-->
{
	const DLGPROC PageProc[]={
		PageAboutProc, PageAlarmProc,
		PageColorProc, PageFormatProc, PageMouseProc,
		PageQuickyProc,
		PageHotKeyProc, PageMiscProc,
	};
	if(!g_hwndSheet || !IsWindow(g_hwndSheet)) {
		#define PROPERTY_NUM sizeof(PageProc)/sizeof(DLGPROC)
		PROPSHEETPAGE psp[PROPERTY_NUM];
		PROPSHEETHEADER psh={sizeof(PROPSHEETHEADER)};
		HMODULE hInstance=GetModuleHandle(NULL);
		int i;
		// Allocate Clean Memory for Each Page
		for(i=0; i<PROPERTY_NUM; ++i) {
			memset(&psp[i], 0, sizeof(PROPSHEETPAGE));
			psp[i].dwSize = sizeof(PROPSHEETPAGE);
			psp[i].hInstance = hInstance;
			psp[i].pfnDlgProc = PageProc[i];
			psp[i].pszTemplate = MAKEINTRESOURCE(PROPERTY_BASE+i);
		}
		
		// set data of property sheet
		psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE | PSH_PROPTITLE | PSH_MODELESS | PSH_USECALLBACK | PSH_NOCONTEXTHELP;
		psh.hInstance = hInstance;
		psh.pszIcon = MAKEINTRESOURCE(IDI_MAIN);
		psh.pszCaption = "T-Clock Redux";
		psh.nPages = PROPERTY_NUM;
		psh.nStartPage = (page==-1?m_startpage:page);
		psh.ppsp = psp;
		psh.pfnCallback = PropSheetProc;
		// show it !
		g_hwndSheet = (HWND)PropertySheet(&psh);
	}
	ForceForegroundWindow(g_hwndSheet);
}
//================================================================================================
//--------------------------------------------------------+++--> Property Sheet Callback Procedure:
int CALLBACK PropSheetProc(HWND hDlg, UINT uMsg, LPARAM  lParam)   //-----------------------+++-->
{
	(void)lParam;
	if(uMsg == PSCB_INITIALIZED) {
		// subclass the window
		m_oldWndProc = (WNDPROC)GetWindowLongPtr(hDlg, GWL_WNDPROC);
		SetWindowLongPtr(hDlg, GWL_WNDPROC, (LONG_PTR)SubclassProc);
	}
	return 0;
}
/*--------------------------------------------------------
   window procedure of subclassed property sheet
---------------------------------------------------------*/
LRESULT CALLBACK SubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message){
	case WM_DESTROY:
		g_hwndSheet = NULL;
		m_startpage=(int)SendMessage((HWND)SendMessage(hwnd,PSM_GETTABCONTROL,0,0),TCM_GETCURSEL,0,0);
		if(m_startpage<0) m_startpage = 0;
		if(g_bApplyClock) {
			g_bApplyClock = 0;
			PostMessage(g_hwndClock, CLOCKM_REFRESHCLOCK, 0, 0);
		}
//		if(g_bApplyClear) {
//			g_bApplyClear = 0;
//			PostMessage(g_hwndClock, CLOCKM_REFRESHCLEARTASKBAR, 0, 0);
//		}
		if(g_bApplyTaskbar) {
			g_bApplyTaskbar = 0;
			PostMessage(g_hwndClock, CLOCKM_REFRESHTASKBAR, 0, 0);
		}
		if(g_hDlgTimer && IsWindow(g_hDlgTimer))
			PostMessage(g_hDlgTimer,WM_CLOSE,0,0);
		#ifndef _DEBUG
		EmptyWorkingSet(GetCurrentProcess());
		#endif
		break;
	case WM_SHOWWINDOW: // adjust the window position
		SetMyDialgPos(hwnd,21);
		break;
	case WM_ACTIVATE:
		if(LOWORD(wParam)==WA_ACTIVE || LOWORD(wParam)==WA_CLICKACTIVE){
			SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		}else{
			SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
			SetWindowPos((HWND)wParam,hwnd,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		}
		break;
	case WM_COMMAND:{
		/// apply settings if applicable
		if(LOWORD(wParam)==ID_APPLY_NOW){
			LRESULT ret=CallWindowProc(m_oldWndProc, hwnd, message, wParam, lParam);
			if(!IsWindowEnabled((HWND)lParam)){ // ID_APPLY_NOW disabled, settings applied
				if(g_bApplyClock) {
					g_bApplyClock = 0;
					PostMessage(g_hwndClock, CLOCKM_REFRESHCLOCK, 0, 0);
				}
//				if(g_bApplyClear) {
//					g_bApplyClear = 0;
//					PostMessage(g_hwndClock, CLOCKM_REFRESHCLEARTASKBAR, 0, 0);
//				}
				if(g_bApplyTaskbar) {
					g_bApplyTaskbar = 0;
					PostMessage(g_hwndClock, CLOCKM_REFRESHTASKBAR, 0, 0);
				}
			}
			return ret;
		}
		break;}
	case WM_SYSCOMMAND:// close by "x" button
		if((wParam & 0xfff0) == SC_CLOSE)
			PostMessage(hwnd, WM_COMMAND, IDCANCEL, 0);
		break;
	}
	return CallWindowProc(m_oldWndProc, hwnd, message, wParam, lParam);
}

//=================================================================================================
//------------------------------+++--> Adjust the Window Position Based on Taskbar Size & Location:
void SetMyDialgPos(HWND hwnd,int padding)   //-----------------------------------------------+++-->
{
	int wscreen, hscreen, wProp, hProp;
	RECT rc;
	HWND hwndTray;
	
	GetWindowRect(hwnd,&rc); // Properties Dialog Dimensions
	wProp = rc.right-rc.left;  //----------+++--> Width
	hProp = rc.bottom-rc.top; //----------+++--> Height
	
	wscreen = GetSystemMetrics(SM_CXSCREEN);  // Desktop Width
	hscreen = GetSystemMetrics(SM_CYSCREEN); // Desktop Height
	
	hwndTray = FindWindow("Shell_TrayWnd", NULL);
	if(!hwndTray) return;
	GetWindowRect(hwndTray,&rc);
	
	if(rc.right-rc.left > rc.bottom-rc.top) { // IF Width is Greater Than Height, Taskbar is
		rc.left=wscreen-wProp-padding; // at Either Top or Bottom of Screen
		if(rc.top < hscreen/2)
			rc.top=rc.bottom+padding; // Taskbar is on Top of Screen
		else // ELSE Taskbar is Where it Belongs! (^^^Mac Fag?^^^)
			rc.top-=hProp+padding;
		if(rc.top<0) rc.top=0;
	} else { //---+++--> ELSE Taskbar is on Left or Right Side of Screen
		rc.top=hscreen-hProp-padding; // Down is a Fixed Position
		if(rc.left < wscreen/2)
			rc.left=rc.right+padding; //--+++--> Taskbar is on Left Side of Screen
		else
			rc.left-=wProp+padding; // Taskbar is on Right Side of Screen
		if(rc.left<0) rc.left=0;
	}
	MoveWindow(hwnd,rc.left,rc.top,wProp,hProp,FALSE);
}
/*------------------------------------------------
   select file
--------------------------------------------------*/
BOOL SelectMyFile(HWND hDlg, const char* filter, DWORD nFilterIndex, const char* deffile, char* retfile)
{
	char fname[MAX_PATH], ftitle[MAX_PATH], initdir[MAX_PATH];
	OPENFILENAME ofn;
	BOOL r;
	
	memset(&ofn, 0, sizeof(OPENFILENAME));
	
	strcpy(initdir, g_mydir);
	if(deffile[0]) {
		WIN32_FIND_DATA fd;
		HANDLE hfind;
		hfind = FindFirstFile(deffile, &fd);
		if(hfind != INVALID_HANDLE_VALUE) {
			FindClose(hfind);
			strcpy(initdir, deffile);
			del_title(initdir);
		}
	}
	
	fname[0] = '\0';
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hDlg;
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = nFilterIndex;
	ofn.lpstrFile= fname;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = ftitle;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = initdir;
	ofn.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_FILEMUSTEXIST;
	
	r = GetOpenFileName(&ofn);
	if(!r) return r;
	
	strcpy(retfile, ofn.lpstrFile);
	
	return r;
}
