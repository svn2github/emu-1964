//    wingui.c The Windows User Interface

// 1964 Copyright (C) 1999-2004 Joel Middendorf, <schibo@emulation64.com> This
// program is free software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version. This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details. You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. To contact the
// authors: email: schibo@emulation64.com, rice1964@yahoo.com

#include "../stdafx.h"

#ifdef _DEBUG
#include "windebug.h"
#endif
struct EMU1964GUI	gui;
struct GUIOPTIONS	guioptions;
struct DIRECTORIES	directories;
struct GUISTATUS	guistatus;
int					ActiveApp;

/* the legal stuff */
unsigned char	Scratch0[700];
unsigned char	Scratch1[700];
unsigned char	Scratch2[700];

extern float DOUBLE_COUNT;

unsigned int		codecheckmenulist[8] = {
	ID_CPU_DYNACODECHECKING_NOCHECK,
	ID_CPU_DYNACODECHECKING_DMA,
	ID_CPU_DYNACODECHECKING_DWORD,
	ID_CPU_DYNACODECHECKING_QWORD,
	ID_CPU_DYNACODECHECKING_QWORDANDDMA,
	ID_CPU_DYNACODECHECKING_BLOCK,
	ID_CPU_DYNACODECHECKING_BLOCKANDDMA,
	ID_CPU_DYNACODECHECKING_PROTECTMEMORY};

char				recent_rom_directory_lists[MAX_RECENT_ROM_DIR][260];
char				recent_game_lists[MAX_RECENT_GAME_LIST][260];
char				game_country_name[10];
int					game_country_tvsystem = 0;
int					Audio_Is_Initialized = 0;
int					timer;
int					StateFileNumber = 0;
extern BOOL			Is_Reading_Rom_File;
extern BOOL			To_Stop_Reading_Rom_File;
extern BOOL			opcode_debugger_memory_is_allocated;
extern HINSTANCE	hinstControllerPlugin;
extern unsigned char	Scratch1[];
extern unsigned char	Scratch2[];

BOOL				NeedFreshromListAfterStop = TRUE;
BOOL				WindowScreenSaverStatus;

HANDLE				StopEmulatorEvent = NULL;
HANDLE				ResumeEmulatorEvent = NULL;
HANDLE				PauseEmulatorEvent = NULL;

#ifdef _DEBUG
void					ToggleDebugOptions(WPARAM wParam);
#endif
LRESULT APIENTRY		OptionsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
LRESULT APIENTRY		FoldersProc(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
LRESULT APIENTRY		RomInfoProc(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
LRESULT APIENTRY        UnavailableProc(HWND hDlg, unsigned message, WORD wParam, LONG lParam);

void					SelectVISpeed(WPARAM wParam);
void					SetupAdvancedMenus(void);
void					RegenerateStateSelectorMenus(void);
void					RegenerateRecentGameMenus(void);
void					DeleteRecentGameMenus(void);
void					RegerateRecentRomDirectoryMenus(void);
void					DeleteRecentRomDirectoryMenus(void);
void __cdecl			RefreshRecentGameMenus(char *newgamefilename);
void					RefreshRecentRomDirectoryMenus(char *newromdirectory);
void					ChangeToRecentDirectory(int id);
void					OpenRecentGame(int id);
void					UpdateCIC(void);
void					init_debug_options(void);
extern LRESULT APIENTRY PluginsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
long					OnNotifyStatusBar(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
long					OnPopupMenuCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
long					OnOpcodeDebuggerCommands(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void					OnFreshRomList(BOOL reload);
void					IncreaseSpeedLimit();
void					DecreaseSpeedLimit();
void					ResetSpeedLimit();
void					RomListSaveCurrentPosToRegistry(void);
void					RomListLoadCurrentPosFromRegistry(void);
void					ReloadDefaultPlugins();
void					ProcessToolTips(LPARAM lParam);
BOOL					LinkBoxArtImageByDialog(void);

void SetOCOptions(void);

typedef struct {
	UINT	id;
	BOOL	visible;	// is the menu visiable or deleted from the menu bar
}MenuStatus;

MenuStatus	recent_game_menus[MAX_RECENT_GAME_LIST] =
{
	{ID_FILE_RECENTGAMES_GAME1,		TRUE},
	{ID_FILE_RECENTGAMES_GAME2,		TRUE},
	{ID_FILE_RECENTGAMES_GAME3,		TRUE},
	{ID_FILE_RECENTGAMES_GAME4,		TRUE},
	{ID_FILE_RECENTGAMES_GAME5,		TRUE},
	{ID_FILE_RECENTGAMES_GAME6,		TRUE},
	{ID_FILE_RECENTGAMES_GAME7,		TRUE},
	{ID_FILE_RECENTGAMES_GAME8,		TRUE},
};
MenuStatus	recent_rom_directory_menus[MAX_RECENT_ROM_DIR] =
{
	{ID_FILE_ROMDIRECTORY1,		TRUE},
	{ID_FILE_ROMDIRECTORY2,		TRUE},
	{ID_FILE_ROMDIRECTORY3,		TRUE},
	{ID_FILE_ROMDIRECTORY4,		TRUE},
	{ID_FILE_ROMDIRECTORY5,		TRUE},
	{ID_FILE_ROMDIRECTORY6,		TRUE},
	{ID_FILE_ROMDIRECTORY7,		TRUE},
	{ID_FILE_ROMDIRECTORY8,		TRUE},
};

extern HANDLE hwndLV;

void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	if( Rom_Loaded) 
	{
		if( !emustatus.Emu_Is_Paused || emustatus.Emu_Is_Running) 
		{
			vips = (float)(viCountPerSecond);

			sprintf(generalmessage, "CF=1");

            SetStatusBarText(2, generalmessage);

			static int lasttime=0;
			static int lastdls=0;

			if(GetTickCount()-lasttime>1000)
			{
				vips = (float)emustatus.DListCount-lastdls;
				lastdls=emustatus.DListCount;
				lasttime=GetTickCount();
			}
			//extern WindowSettingStruct windowSetting;
			sprintf(generalmessage, " %d REAL FPS", (int) vips);
							

			viCountPerSecond = 0;
			QueryPerformanceCounter(&LastSecondTime);

			if( guistatus.IsFullScreen == FALSE) 
			{
				SetStatusBarText(1, generalmessage);

				if( guioptions.display_profiler_status || emuoptions.AutoFrameSkip ) 
				{
					format_profiler_result_msg(generalmessage);
					reset_profiler();
				}

				if( guioptions.display_profiler_status ) 
				{
					SetStatusBarText(0, generalmessage);
				} 
				else if(guioptions.display_detail_status) 
				{
					sprintf
						(
						generalmessage,
						"PC=%08x, DList=%d, AList=%d, PI=%d, Cont=%d",
						gHWS_pc,
						emustatus.DListCount,
						emustatus.AListCount,
						emustatus.PIDMACount,
						emustatus.ControllerReadCount
						);
					SetStatusBarText(0, generalmessage);
				}
			}

			/* Apply the hack codes */
			if(emuoptions.auto_apply_cheat_code)
			{
#ifndef CHEATCODE_LOCK_MEMORY
				CodeList_ApplyAllCode(INGAME);
#endif
			}
		}

		// Using the TimerProc to make sure GUI thread won't get lost 
		MSG msg;
		if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if(GetMessage(&msg, NULL, 0, 0))
			{
				if(!TranslateAccelerator(gui.hwnd1964main, gui.hAccTable, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
                    //Sleep(10);
				}
			}
		}

	}
}

//Test: Creating property pages for all options
void CreateOptionsDialog(int nStartPage)
	{
	PROPSHEETPAGE	psp[8]; //Change this array size if you change the number of pages.
    PROPSHEETHEADER psh;
	
    TBBUTTONINFO ButtonInfo;
    int Result;

    __try
    {
        char test[40];

        strcpy(test, romlist[rlstatus.selected_rom_index]->pinientry->Game_Name);
        Result = 1;
    }
    __except(NULL, EXCEPTION_EXECUTE_HANDLER)
    {
        Result = 0;
    }

	if (Result)
    {
        psp[PSH_CHEAT_CODES].pszTemplate = "CHEAT_HACK";
	    psp[PSH_CHEAT_CODES].pfnDlgProc = (DLGPROC) CheatAndHackDialog;
    }
    else
    {
        psp[PSH_CHEAT_CODES].pszTemplate = "UNAVAILABLE";
	    psp[PSH_CHEAT_CODES].pfnDlgProc = (DLGPROC) UnavailableProc;
    }

	psp[PSH_CHEAT_CODES].dwSize = sizeof(PROPSHEETPAGE);
	psp[PSH_CHEAT_CODES].dwFlags = PSP_USETITLE;
	psp[PSH_CHEAT_CODES].hInstance = gui.hInst;
	psp[PSH_CHEAT_CODES].pszIcon = NULL;
	psp[PSH_CHEAT_CODES].pszTitle = TranslateStringByString("Cheat Codes");
	psp[PSH_CHEAT_CODES].lParam = 0;

	psp[PSH_WINDOW].dwSize = sizeof(PROPSHEETPAGE);
	psp[PSH_WINDOW].dwFlags = PSP_USETITLE;
	psp[PSH_WINDOW].hInstance = gui.hInst;
	psp[PSH_WINDOW].pszTemplate = "OPTIONS";
	psp[PSH_WINDOW].pszIcon = NULL;
	psp[PSH_WINDOW].pfnDlgProc = (DLGPROC) OptionsDialog;
	psp[PSH_WINDOW].pszTitle = TranslateStringByString("Window");
	psp[PSH_WINDOW].lParam = 0;

	psp[PSH_PLUGINS].dwSize = sizeof(PROPSHEETPAGE);
	psp[PSH_PLUGINS].dwFlags = PSP_USETITLE;
	psp[PSH_PLUGINS].hInstance = gui.hInst;
	psp[PSH_PLUGINS].pszIcon = NULL;

	
    // Check the button state of the Open Rom Button.
    // If it's disabled, then we will not show the plugins dialog.
    ButtonInfo.cbSize = sizeof(TBBUTTONINFO);
	ButtonInfo.dwMask = TBIF_STATE;
	SendMessage(gui.hToolBar, TB_GETBUTTONINFO, ID_BUTTON_OPEN_ROM,
	(LPARAM)(LPTBBUTTONINFO) &ButtonInfo);

	if (ButtonInfo.fsState & TBSTATE_ENABLED && emustatus.Emu_Is_Running == FALSE )
    {
	    psp[PSH_PLUGINS].pszTemplate = "PLUGINS";
        psp[PSH_PLUGINS].pfnDlgProc = (DLGPROC) PluginsDialog;
	}
    else
    {
        psp[PSH_PLUGINS].pszTemplate = "UNAVAILABLE";
        psp[PSH_PLUGINS].pfnDlgProc = (DLGPROC) UnavailableProc;
	}
	psp[PSH_PLUGINS].pszTitle = TranslateStringByString("Plug-ins");
	psp[PSH_PLUGINS].lParam = 0;

    
	if (Result)
    {
	    psp[PSH_ROM_OPTIONS].pszTemplate = "ROM_OPTIONS";
        psp[PSH_ROM_OPTIONS].pfnDlgProc = (DLGPROC) RomListDialog;
    }
    else
    {
        psp[PSH_ROM_OPTIONS].pszTemplate = "UNAVAILABLE";
	    psp[PSH_ROM_OPTIONS].pfnDlgProc = (DLGPROC) UnavailableProc;
    }
    
    psp[PSH_ROM_OPTIONS].dwSize = sizeof(PROPSHEETPAGE);
	psp[PSH_ROM_OPTIONS].dwFlags = PSP_USETITLE;
	psp[PSH_ROM_OPTIONS].hInstance = gui.hInst;
	psp[PSH_ROM_OPTIONS].pszIcon = NULL;
	psp[PSH_ROM_OPTIONS].pszTitle = TranslateStringByString("ROM Properties");
	psp[PSH_ROM_OPTIONS].lParam = 0;


	psp[PSH_ROM_BROWSER].dwSize = sizeof(PROPSHEETPAGE);
	psp[PSH_ROM_BROWSER].dwFlags = PSP_USETITLE;
	psp[PSH_ROM_BROWSER].hInstance = gui.hInst;
	psp[PSH_ROM_BROWSER].pszTemplate = "COL_SELECT";
	psp[PSH_ROM_BROWSER].pszIcon = NULL;
	psp[PSH_ROM_BROWSER].pfnDlgProc = (DLGPROC) ColumnSelectDialog;
	psp[PSH_ROM_BROWSER].pszTitle = TranslateStringByString("ROM Browser");
	psp[PSH_ROM_BROWSER].lParam = 0;


    if (Result)
    {
    	psp[PSH_ROM_INFORMATION].pszTemplate = "ROM_INFO";
        psp[PSH_ROM_INFORMATION].pfnDlgProc = (DLGPROC) RomInfoProc;
    }
    else
    {
        psp[PSH_ROM_INFORMATION].pszTemplate = "UNAVAILABLE";
        psp[PSH_ROM_INFORMATION].pfnDlgProc = (DLGPROC) UnavailableProc;
    }

	psp[PSH_ROM_INFORMATION].dwSize = sizeof(PROPSHEETPAGE);
	psp[PSH_ROM_INFORMATION].dwFlags = PSP_USETITLE;
	psp[PSH_ROM_INFORMATION].hInstance = gui.hInst;
	psp[PSH_ROM_INFORMATION].pszIcon = NULL;
	psp[PSH_ROM_INFORMATION].pszTitle = TranslateStringByString("ROM Information");
	psp[PSH_ROM_INFORMATION].lParam = 0;

    psp[PSH_FOLDERS].dwSize = sizeof(PROPSHEETPAGE);
	psp[PSH_FOLDERS].dwFlags = PSP_USETITLE;
	psp[PSH_FOLDERS].hInstance = gui.hInst;
	psp[PSH_FOLDERS].pszTemplate = "FOLDERS";
	psp[PSH_FOLDERS].pszIcon = NULL;
	psp[PSH_FOLDERS].pfnDlgProc = (DLGPROC) FoldersProc;
	psp[PSH_FOLDERS].pszTitle = TranslateStringByString("Folders");
	psp[PSH_FOLDERS].lParam = 0;

    psp[PSH_ABOUT].dwSize = sizeof(PROPSHEETPAGE);
	psp[PSH_ABOUT].dwFlags = PSP_USETITLE;
	psp[PSH_ABOUT].hInstance = gui.hInst;
	psp[PSH_ABOUT].pszTemplate = "ABOUTBOX";
	psp[PSH_ABOUT].pszIcon = NULL;
	psp[PSH_ABOUT].pfnDlgProc = (DLGPROC) About;
	psp[PSH_ABOUT].pszTitle = TranslateStringByString("About 1964");
	psp[PSH_ABOUT].lParam = 0;

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW| PSH_USEICONID ;
	psh.hwndParent = gui.hwnd1964main;
	psh.hInstance = gui.hInst;
	psh.pszIcon = MAKEINTRESOURCE(IDI_ICON2);
	psh.pszCaption = (LPSTR) "Options";
	psh.nStartPage = nStartPage;
	psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
	psh.ppsp = (LPCPROPSHEETPAGE) & psp;

	{
	HWND hCOP2Vecswnd;
	hCOP2Vecswnd = (HWND) PropertySheet(&psh);
	}
}

extern HWND WINAPI CreateTT(HWND hwndOwner);
extern HANDLE	AudioThreadStopEvent;
extern HANDLE	AudioThreadEvent;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
BOOL WindowMsgLoop()
{
	MSG msg;

	if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if(GetMessage(&msg, NULL, 0, 0))
		{
			if(!TranslateAccelerator(gui.hwnd1964main, gui.hAccTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			return TRUE;
		}
	}
	
	return FALSE;
}


int APIENTRY aWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	MSG msg;
	BOOL pluginInitResult;
	BOOL needchooseromdirectory;

	if(hPrevInstance) return FALSE;
	SaveCmdLineParameter(lpszCmdLine);

	char szTitle[64];
	sprintf(szTitle,"1964 %s", BUILD_NUMBER);
	gui.szBaseWindowTitle = szTitle;
	gui.hwnd1964main = NULL;		/* handle to main window */
	gui.hwndRomList = NULL;			/* Handle to the rom list child window */
	gui.hStatusBar = NULL;			/* Window Handle of the status bar */
	gui.hToolBar = NULL;			/* Window Handle of the toolbar */
	gui.hClientWindow = NULL;		/* Window handle of the client child window */
	gui.hMenu1964main = NULL;
	gui.hMenuRomListPopup = NULL;

	Rom_Loaded = FALSE;
	guistatus.block_menu = TRUE;	/* block all menu commands during startup */
	emustatus.cpucore = DYNACOMPILER;
	emustatus.Emu_Is_Resetting = FALSE;
	guistatus.IsFullScreen = FALSE;
	emustatus.Emu_Is_Running = FALSE;
	emustatus.Emu_Is_Paused = FALSE;
	emustatus.Emu_Keep_Running = FALSE;
	emustatus.processing_exception = FALSE;

	DWORD dwTempOC=REGISTRY_ReadDWORD("OCSpeed",100);	

	SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &WindowScreenSaverStatus, 0); 
	
	memset(emustatus.lastVideoPluginLoaded,0,sizeof(emustatus.lastVideoPluginLoaded));
	memset(emustatus.lastInputPluginLoaded,0,sizeof(emustatus.lastInputPluginLoaded));
	memset(emustatus.lastAudioPluginLoaded,0,sizeof(emustatus.lastAudioPluginLoaded));
	memset(emustatus.lastRSPPluginLoaded,0,sizeof(emustatus.lastRSPPluginLoaded));
	memset(&rlstatus,0,sizeof(ROMLIST_STATUS));

	memset(languageFileNames,0,sizeof(languageFileNames));

	StopEmulatorEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

	if (StopEmulatorEvent == NULL)
	{ 
		DisplayError( "Error creating StopEmulatorEvent events");
	} 

	ResumeEmulatorEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	AudioThreadStopEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	AudioThreadEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    PauseEmulatorEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

	if (ResumeEmulatorEvent == NULL)
	{ 
		DisplayError( "Error creating ResumeEmulatorEvent events");
	} 

#ifdef _DEBUG
	init_debug_options();
#endif

	gui.hInst = hInstance;
	Set_1964_Directory();

	gui.hwnd1964main = InitWin98UI(hInstance, nCmdShow);
	if( gui.hwnd1964main == NULL) {
		MessageBox(0, TranslateStringByString("Could not get a windows handle."), TranslateStringByString("Exit"), 0);
		exit(1);
	}

	ReadConfiguration();

	ManageMenus();
	CheckLanguages();

	gui.hStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_OVERLAPPED, gui.staturbar_field.field_1, gui.hwnd1964main, 0x1122);

	SetupToolBar();
	InitStatusBarParts();

	InitVirtualMemory();
	InitPluginData();

	SetStatusBarText(0, TranslateStringByString("Load Rom Setting from ROM_Properties.ini"));
	FileIO_Load1964Ini();

	emustatus.cpucore = defaultoptions.Emulator;

	SetStatusBarText(3, defaultoptions.RDRAM_Size == RDRAMSIZE_4MB ? "4MB" : "8MB");
	SetStatusBarText(4, "D");
	SetStatusBarText(2, "CF=1");


	gui.hwndRomList = NewRomList_CreateListViewControl(gui.hwnd1964main);	/* this must be before the video plugin init */

	EnableRadioButtons(FALSE);

	ShowWindow(gui.hwnd1964main, SW_SHOW);
	UpdateWindow(gui.hwnd1964main);

	if(guistatus.WindowIsMaximized)
	{
		ShowWindow(gui.hwnd1964main, SW_SHOWMAXIMIZED);
	}

	timer = SetTimer(gui.hwnd1964main, 1, 1000, TimerProc);

	needchooseromdirectory = FALSE;
	if( StartGameByCommandLine() )
	{
		pluginInitResult = TRUE;
	}
	else
	{
		NeedFreshromListAfterStop = FALSE;
		if (guioptions.display_romlist)
		{
			if( strlen(directories.rom_directory_to_use) > 4 && PathFileExists(directories.rom_directory_to_use) )
			{
				NewRomList_ListViewChangeWindowRect();

				SetStatusBarText(0, TranslateStringByString("Looking for ROM files in the ROM directory and Generating List"));
				RomListReadDirectory(directories.rom_directory_to_use,TRUE);
			}
			else
			{
				needchooseromdirectory = TRUE;
			}
		}

		pluginInitResult = LoadPlugins(LOAD_ALL_PLUGIN);
	}
	SetFocus(gui.hwnd1964main);
	RomListLoadCurrentPosFromRegistry();

	guistatus.block_menu = FALSE;	/* allow menu commands */
    Set_Ready_Message();
	RomListLoadCurrentPosFromRegistry();
	SetFocus(gui.hwnd1964main);
	SetOCOptions();

	if( needchooseromdirectory )
	{
		ChangeDirectory();
	}

	if( pluginInitResult == FALSE )
	{
		SendMessage(gui.hwnd1964main, WM_COMMAND, IDM_PLUGINS, 0);
	}


_HOPPITY:
 
    if(GetMessage(&msg, NULL, 0, 0))
	{
		if(!TranslateAccelerator(gui.hwnd1964main, gui.hAccTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
            //Sleep(10);
		}
	}
	else
	{
        return msg.wParam;
	}

	goto _HOPPITY;
}

void SetOCOptions(void)
{
	char szTitle[64];
	sprintf(szTitle,"1964 [%dMHz] %s",(int)(DOUBLE_COUNT*100), BUILD_NUMBER);
	SetWindowText(gui.hwnd1964main,szTitle);
	
	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_25MHZ, MF_UNCHECKED);
	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_50MHZ, MF_UNCHECKED);
	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_100MHZ, MF_UNCHECKED);
	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_200MHZ, MF_UNCHECKED);
	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_300MHZ, MF_UNCHECKED);
	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_400MHZ, MF_UNCHECKED);
	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_500MHZ, MF_UNCHECKED);
	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_600MHZ, MF_UNCHECKED);

	if(DOUBLE_COUNT==0.25f)	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_25MHZ, MF_CHECKED);
	if(DOUBLE_COUNT==0.5f)	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_50MHZ, MF_CHECKED);
	if(DOUBLE_COUNT==1.0f)	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_100MHZ, MF_CHECKED);
	if(DOUBLE_COUNT==2.0f)	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_200MHZ, MF_CHECKED);
	if(DOUBLE_COUNT==3.0f)	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_300MHZ, MF_CHECKED);
	if(DOUBLE_COUNT==4.0f)	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_400MHZ, MF_CHECKED);
	if(DOUBLE_COUNT==5.0f)	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_500MHZ, MF_CHECKED);
	if(DOUBLE_COUNT==6.0f)	CheckMenuItem( gui.hMenu1964main,ID_OVERCLOCK_600MHZ, MF_CHECKED);

	REGISTRY_WriteDWORD("OCSpeed",(DWORD)(DOUBLE_COUNT*100));	

}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
HWND InitWin98UI(HANDLE hInstance, int nCmdShow)
{
	WNDCLASS	wc;

	wc.style = CS_SAVEBITS;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = (HINSTANCE) hInstance;
	wc.hIcon = LoadIcon((HINSTANCE) hInstance, MAKEINTRESOURCE(IDI_ICON2));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = "WINGUI_MENU";
	wc.lpszClassName = "WinGui";
	RegisterClass(&wc);

	guistatus.clientwidth = REGISTRY_ReadDWORD("ClientWindowWidth", 640);
	if( guistatus.clientwidth < 200 )	guistatus.clientwidth = 640;

	guistatus.clientheight = REGISTRY_ReadDWORD("ClientWindowHeight", 600);
	if( guistatus.clientheight < 300 ) guistatus.clientheight = 600;

	guistatus.window_position.top = REGISTRY_ReadDWORD("1964WindowTOP", 100);
	guistatus.window_position.left = REGISTRY_ReadDWORD("1964WindowLeft", 100);

	gui.hwnd1964main = CreateWindow
		(
			"WinGui",
			gui.szBaseWindowTitle,
			WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
			guistatus.window_position.left,
			guistatus.window_position.top,
			guistatus.clientwidth,
			guistatus.clientheight,
			NULL,
			NULL,
			(HINSTANCE) hInstance,
			NULL
		);

	ShowWindow(gui.hToolBar, SW_SHOW);

	if(gui.hwnd1964main == NULL)
	{
		MessageBox(NULL, TranslateStringByString("CreateWindow() failed: Cannot create a window."), "Error", MB_OK);
		return(NULL);
	}

	gui.hAccTable = LoadAccelerators(gui.hInst, (LPCTSTR) WINGUI_ACC);
	gui.hMenu1964main = GetMenu(gui.hwnd1964main);

	return gui.hwnd1964main;
}

extern void DynaBufferOverrun();
void SwitchLanguage(int id, BOOL refreshRomList);
void ResetToDefaultLanguage();
void ProcessMenuCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int PlayButtonState; //For pause/Play	
	
	if(guistatus.block_menu)
			return; /* ok, all menu commands are blocked */

	switch(LOWORD(wParam))
	{
		case ID_OVERCLOCK_25MHZ: DOUBLE_COUNT=0.25f; SetOCOptions();break;
		case ID_OVERCLOCK_50MHZ: DOUBLE_COUNT=0.5f; SetOCOptions();break;
		case ID_OVERCLOCK_100MHZ: DOUBLE_COUNT=1.0f; SetOCOptions();break;
		case ID_OVERCLOCK_200MHZ: DOUBLE_COUNT=2.0f; SetOCOptions();break;
		case ID_OVERCLOCK_300MHZ: DOUBLE_COUNT=3.0f; SetOCOptions();break;
		case ID_OVERCLOCK_400MHZ: DOUBLE_COUNT=4.0f; SetOCOptions();break;
		case ID_OVERCLOCK_500MHZ: DOUBLE_COUNT=5.0f; SetOCOptions();break;
		case ID_OVERCLOCK_600MHZ: DOUBLE_COUNT=6.0f; SetOCOptions();break;

	case ID_ROM_STOP:
	case ID_BUTTON_STOP:
		CloseROM();
		break;

	case ID_ROM_START:
	case ID_BUTTON_RESET:
		if (!emustatus.Emu_Is_Running) 
			return;
		emustatus.Emu_Is_Resetting = 1;
		Play(emuoptions.auto_full_screen);
		emustatus.Emu_Is_Resetting = FALSE;
		break;

	case ID_ROM_PAUSE:
		ChangeButtonState(ID_BUTTON_PAUSE);
		PlayButtonState = ChangeButtonState(ID_BUTTON_PLAY);
		if((PlayButtonState & TBSTATE_CHECKED) != TBSTATE_CHECKED)
		{
			emustatus.Emu_Is_Paused = 0;
			PauseEmulator();
		}
		else
		{
			emustatus.Emu_Is_Paused = 1;
			ResumeEmulator(DO_NOTHING_AFTER_PAUSE);
		}
		break;
	case ID_BUTTON_PLAY:
		ResumeEmulator(DO_NOTHING_AFTER_PAUSE);
		break;
	case ID_BUTTON_PAUSE:	
		PauseEmulator();
		break;
	case ID_CPU_KILL:
		KillCPUThread();
		break;
	case ID_OPENROM:
	case ID_BUTTON_OPEN_ROM:
		if (!guistatus.IsFullScreen)
			OpenROM();
		break;
	case ID_CLOSEROM:
		CloseROM();
		break;
	case ID_FILE_ROMINFO:
	case ID_BUTTON_ROM_PROPERTIES:
		if (!guistatus.IsFullScreen)
			RomListRomOptions(rlstatus.selected_rom_index);
		break;
	case ID_FILE_FRESHROMLIST:
		if (!guistatus.IsFullScreen)
			OnFreshRomList(TRUE);
		break;
	case ID_PREFERENCE_OPTIONS:
		if (!guistatus.IsFullScreen)
			CreateOptionsDialog(PSH_WINDOW);
		break;
	case ID_CHANGEDIRECTORY:
	case ID_POPUPMENU_CHANGEROMDIRECTORY:
		if (!guistatus.IsFullScreen)
			ChangeDirectory();
		break;
	case ID_FILE_ROMDIRECTORY1:
		if (!guistatus.IsFullScreen)
			ChangeToRecentDirectory(0);
		break;
	case ID_FILE_ROMDIRECTORY2:
		if (!guistatus.IsFullScreen)
			ChangeToRecentDirectory(1);
		break;
	case ID_FILE_ROMDIRECTORY3:
		if (!guistatus.IsFullScreen)
			ChangeToRecentDirectory(2);
		break;
	case ID_FILE_ROMDIRECTORY4:
		if (!guistatus.IsFullScreen)
			ChangeToRecentDirectory(3);
		break;
	case ID_FILE_ROMDIRECTORY5:
		if (!guistatus.IsFullScreen)
			ChangeToRecentDirectory(4);
		break;
	case ID_FILE_ROMDIRECTORY6:
		if (!guistatus.IsFullScreen)
			ChangeToRecentDirectory(5);
		break;
	case ID_FILE_ROMDIRECTORY7:
		if (!guistatus.IsFullScreen)
			ChangeToRecentDirectory(6);
		break;
	case ID_FILE_ROMDIRECTORY8:
		if (!guistatus.IsFullScreen)
			ChangeToRecentDirectory(7);
		break;
	case ID_FILE_RECENTGAMES_GAME1:
		OpenRecentGame(0);
		break;
	case ID_FILE_RECENTGAMES_GAME2:
		OpenRecentGame(1);
		break;
	case ID_FILE_RECENTGAMES_GAME3:
		OpenRecentGame(2);
		break;
	case ID_FILE_RECENTGAMES_GAME4:
		OpenRecentGame(3);
		break;
	case ID_FILE_RECENTGAMES_GAME5:
		OpenRecentGame(4);
		break;
	case ID_FILE_RECENTGAMES_GAME6:
		OpenRecentGame(5);
		break;
	case ID_FILE_RECENTGAMES_GAME7:
		OpenRecentGame(6);
		break;
	case ID_FILE_RECENTGAMES_GAME8:
		OpenRecentGame(7);
		break;	
	case ID_FILE_CHEAT:
		if (!guistatus.IsFullScreen)
			if(emustatus.Emu_Is_Running)
			{
				PauseEmulator();
				CreateOptionsDialog(PSH_CHEAT_CODES);
				ResumeEmulator(DO_NOTHING_AFTER_PAUSE);
			}
			else
			{
				CreateOptionsDialog(PSH_CHEAT_CODES);
			}
			break;

	case ID_ABOUT:
		if (!guistatus.IsFullScreen)
			CreateOptionsDialog(PSH_ABOUT);
		break;

	case ID_CHEATS_APPLY:
		CodeList_ApplyAllCode(GSBUTTON);
		break;

	case ID_EMULATION_AUTOFRAMESKIP:
	case ID_BUTTON_FRAMESKIP:
		//User clicked menu? If so, make the corresponding button the right setting.            
		if ((LOWORD(wParam)) == ID_EMULATION_AUTOFRAMESKIP)
			CheckButton(ID_BUTTON_FRAMESKIP,
			emuoptions.AutoFrameSkip ? 
			FALSE : 
			TRUE);

		CheckMenuItem( gui.hMenu1964main, 
			ID_EMULATION_AUTOFRAMESKIP, 
			emuoptions.AutoFrameSkip ? 
			MF_UNCHECKED : 
			MF_CHECKED);

		emuoptions.AutoFrameSkip^=1;
		REGISTRY_WriteAutoFrameSkip();
		break;

	case ID_BUTTON_SYNC_SPEED:
	case ID_CPU_AUDIOSYNC:
		//User clicked menu? If so, make the corresponding button the right setting.            
		if ((LOWORD(wParam)) == ID_CPU_AUDIOSYNC)
			CheckButton(ID_BUTTON_SYNC_SPEED, emuoptions.SyncVI ? FALSE : TRUE);
		CheckMenuItem( gui.hMenu1964main, ID_CPU_AUDIOSYNC, emuoptions.SyncVI ? MF_UNCHECKED : MF_CHECKED);
		emuoptions.SyncVI^=1;
		REGISTRY_WriteVISync();
		break;

	case ID_VIDEO_CONFIG:
		if (!guistatus.IsFullScreen)
		{
			if( strcmp(gRegSettings.VideoPlugin,emustatus.lastVideoPluginLoaded) != 0 
				&& !emustatus.Emu_Is_Running )
			{
				LoadPlugins(LOAD_VIDEO_PLUGIN);
			}
			VIDEO_DllConfig(hWnd);
			InitStatusBarParts();
		}
		break;
	case ID_AUD_CONFIG:
		if (!guistatus.IsFullScreen)
		{
			if(emustatus.Emu_Is_Running)
			{
				SuspendThread(CPUThreadHandle);
				AUDIO_DllConfig(hWnd);
				ResumeThread(CPUThreadHandle);
			}
			else
			{
				if( strcmp(gRegSettings.AudioPlugin,emustatus.lastAudioPluginLoaded) != 0 )
				{
					LoadPlugins(LOAD_AUDIO_PLUGIN);
				}
				AUDIO_DllConfig(hWnd);
			}
		}
		break;
	case ID_DI_CONFIG:
		if (!guistatus.IsFullScreen)
		{
			if( strcmp(gRegSettings.InputPlugin,emustatus.lastInputPluginLoaded) != 0 
				&& !emustatus.Emu_Is_Running)
			{
				LoadPlugins(LOAD_INPUT_PLUGIN);
			}
			CONTROLLER_DllConfig(hWnd);
		}
		break;
	case ID_RSP_CONFIG:
		if (!guistatus.IsFullScreen)
		{
			if( strcmp(gRegSettings.RSPPlugin,emustatus.lastRSPPluginLoaded) != 0 
				&& !emustatus.Emu_Is_Running )
			{
				LoadPlugins(LOAD_RSP_PLUGIN);
			}
			RSPDllConfig(hWnd);
		}
		break;
	case ID_INTERPRETER:
		CheckMenuItem(gui.hMenu1964main, ID_INTERPRETER, MF_CHECKED);
		CheckMenuItem(gui.hMenu1964main, ID_STATICCOMPILER, MF_UNCHECKED);
		CheckMenuItem(gui.hMenu1964main, ID_DYNAMICCOMPILER, MF_UNCHECKED);
		EmulatorSetCore(INTERPRETER);
		break;
	case ID_STATICCOMPILER:
		CheckMenuItem(gui.hMenu1964main, ID_INTERPRETER, MF_UNCHECKED);
		CheckMenuItem(gui.hMenu1964main, ID_STATICCOMPILER, MF_CHECKED);
		CheckMenuItem(gui.hMenu1964main, ID_DYNAMICCOMPILER, MF_UNCHECKED);
		EmulatorSetCore(1);
		break;
	case ID_DYNAMICCOMPILER:
		CheckMenuItem(gui.hMenu1964main, ID_INTERPRETER, MF_UNCHECKED);
		CheckMenuItem(gui.hMenu1964main, ID_STATICCOMPILER, MF_UNCHECKED);
		CheckMenuItem(gui.hMenu1964main, ID_DYNAMICCOMPILER, MF_CHECKED);
		EmulatorSetCore(DYNACOMPILER);
		break;
	case ID_INC_SPEED_LIMIT:
	case ID_VARIABLESPEEDLIMITS_INCREASESPEED:
		IncreaseSpeedLimit();
		break;
	case ID_DEC_SPEED_LIMIT:
	case ID_VARIABLESPEEDLIMITS_DECREASESPEED:
		DecreaseSpeedLimit();
		break;
	case ID_RESET_SPEED_LIMIT:
	case ID_VARIABLESPEEDLIMITS_RESTORECORRECTSPEED:
		ResetSpeedLimit();
		break;
	case ID_CPU_DYNACODECHECKING_NOCHECK:
		SetCodeCheckMethod(1);
		break;
	case ID_CPU_DYNACODECHECKING_DMA:
		SetCodeCheckMethod(2);
		break;
	case ID_CPU_DYNACODECHECKING_DWORD:
		SetCodeCheckMethod(3);
		break;
	case ID_CPU_DYNACODECHECKING_QWORD:
		SetCodeCheckMethod(4);
		break;
	case ID_CPU_DYNACODECHECKING_QWORDANDDMA:
		SetCodeCheckMethod(5);
		break;
	case ID_CPU_DYNACODECHECKING_BLOCK:
		SetCodeCheckMethod(6);
		break;
	case ID_CPU_DYNACODECHECKING_BLOCKANDDMA:
		SetCodeCheckMethod(7);
		break;
	case ID_CPU_DYNACODECHECKING_PROTECTMEMORY:
		SetCodeCheckMethod(8);
		break;
	case ID_BUTTON_FULL_SCREEN:
	case IDM_FULLSCREEN:
		if(emustatus.Emu_Is_Running && PauseEmulator()) {
			VIDEO_ChangeWindow(guistatus.IsFullScreen);
			ResumeEmulator(DO_NOTHING_AFTER_PAUSE);
		}
		break;
	case ID_PLUGINS_SCREENSHOTS:
		CaptureScreenToFile();
		break;
	case IDM_PLUGINS:
	case ID_BUTTON_SETUP_PLUGINS:
		if (!guistatus.IsFullScreen)
		{
			guioptions.displayDefaultPlugins = TRUE;
			CreateOptionsDialog(PSH_PLUGINS);
		}
		break;
	case ID_CHECKWEB:
	case ID_BUTTON_HOME_PAGE:
		if (!guistatus.IsFullScreen)
			ShellExecute(gui.hwnd1964main, "open", "http://code.google.com/p/emu-1964", NULL, NULL, SW_SHOWNORMAL);
		break;
	case ID_HELP_HELP:
	case ID_BUTTON_HELP:
		if (!guistatus.IsFullScreen)
		{
			char filename[512];
			strcpy(filename,directories.main_directory);
			strcat(filename, "Help/1964.chm");
			ShellExecute(gui.hwnd1964main, "open", filename, NULL, NULL, SW_SHOWNORMAL);
		}
		break;
	case ID_ONLINE_HELP:
		if (!guistatus.IsFullScreen)
			ShellExecute(gui.hwnd1964main, "open", "http://1964emu.emulation64.com/help.htm", NULL, NULL, SW_SHOWNORMAL);
		break;
	case ID_CONFIGURE_VIDEO:
		VIDEO_DllConfig(hWnd);
		break;
	case ID_HELP_FINDER:
		DisplayError("Help contents");
		break;

	case ID_OPCODEDEBUGGER:
	case ID_OPCODEDEBUGGER_BLOCK_ONLY:
	case ID_DIRTYONLY:
		OnOpcodeDebuggerCommands(hWnd, message, wParam, lParam);
		break;
	case ID_SAVE_STATE_1:
	case ID_SAVE_STATE_2:
	case ID_SAVE_STATE_3:
	case ID_SAVE_STATE_4:
	case ID_SAVE_STATE_5:
	case ID_SAVE_STATE_6:
	case ID_SAVE_STATE_7:
	case ID_SAVE_STATE_8:
	case ID_SAVE_STATE_9:
	case ID_SAVE_STATE_0:
		SaveStateByNumber(wParam);
		break;
	case ID_LOAD_STATE_1:
	case ID_LOAD_STATE_2:
	case ID_LOAD_STATE_3:
	case ID_LOAD_STATE_4:
	case ID_LOAD_STATE_5:
	case ID_LOAD_STATE_6:
	case ID_LOAD_STATE_7:
	case ID_LOAD_STATE_8:
	case ID_LOAD_STATE_9:
	case ID_LOAD_STATE_0:
		LoadStateByNumber(wParam);
		break;
	case ID_SAVESTATE:
		if (!guistatus.IsFullScreen)
			SaveStateByDialog(SAVE_STATE_1964_FORMAT);
		break;
	case ID_LOADSTATE:
		if (!guistatus.IsFullScreen)
			LoadStateByDialog(SAVE_STATE_1964_FORMAT);
		break;
	case ID_CPU_IMPORTPJ64STATE:
		if (!guistatus.IsFullScreen)
			LoadStateByDialog(SAVE_STATE_PJ64_FORMAT);
		break;
	case ID_CPU_EXPORTPJ64STATE:
		if (!guistatus.IsFullScreen)
			SaveStateByDialog(SAVE_STATE_PJ64_FORMAT);
		break;
	case ID_POPUP_LOADPLAY:
	case ID_POPUP_LOADPLAYINFULLSCREEN:
	case ID_POPUP_LOADPLAYINWINDOWMODE:
	case ID_POPUP_ROM_SETTING:
	case ID_POPUP_CHEATCODE:
	case ID_HEADERPOPUP_SHOW_INTERNAL_NAME:
	case ID_HEADERPOPUP_SHOWALTERNATEROMNAME:
	case ID_HEADERPOPUP_SHOWROMFILENAME:
	case ID_HEADERPOPUP_1_SORT_ASCENDING:
	case ID_HEADERPOPUP_1_SORT_DESCENDING:
	case ID_HEADERPOPUP_2_SORT_ASCENDING:
	case ID_HEADERPOPUP_2_SORT_DESCENDING:
	case ID_HEADERPOPUP_1_SELECTING:
	case ID_HEADERPOPUP_2_SELECTING:
	case ID_POPUPMENU_PLUGINSETTING:
	case ID_POPUPMENU_ROMINFORMATION:
	case ID_HEADERPOPUP3_SHOWROMLIST:
	case ID_HEADERPOPUP3_SHOWBOXART:
	case ID_BOXARTIMAGE_ZOOM_IN:
	case ID_BOXARTIMAGE_ZOOM_OUT:
	case ID_BOXARTIMAGE_DECREASEXSPACING:
	case ID_BOXARTIMAGE_INCREASEXSPACING:
	case ID_BOXARTIMAGE_DECREASEYSPACING:
	case ID_BOXARTIMAGE_INCREASEYSPACING:
	case ID_POPUPMENU_ASSIGNBOXARTIMAGE:
	case ID_POPUPMENU_REMOVEBOXARTIMAGELINK:
	case ID_HEADERPOPUP3_REFRESH:
		OnPopupMenuCommand(hWnd, message, wParam, lParam);
		break;
	case ID_LANGUAGE_ENGLISH:
		ResetToDefaultLanguage();
		break;
	case ID_EXIT:
		Exit1964();
		break;
	default:
		if( LOWORD(wParam) >= NEW_LANGUAGE_MENU_START && LOWORD(wParam) < NEW_LANGUAGE_MENU_START + 200 )
		{
			SwitchLanguage(LOWORD(wParam)-NEW_LANGUAGE_MENU_START, TRUE);
		}
		else
		{
#ifdef _DEBUG
			ProcessDebugMenuCommand(wParam);
#endif
		}
		break;
	}
}

void OnWindowSize(WPARAM wParam)
{
	//abcdefgh
	RECT rcStatusBar;
	RECT rcRomList;
	RECT rcToolBar;

	if(gui.hToolBar != NULL) {
		GetClientRect(gui.hToolBar, &rcToolBar);
	}

	GetWindowRect(gui.hStatusBar, &rcStatusBar);

	if (gui.hwndRomList != NULL) {
		GetClientRect(gui.hwnd1964main, &rcRomList);
		if(gui.hToolBar != NULL) {
			rcRomList.top += (rcToolBar.bottom - rcToolBar.top - 1);
			rcRomList.bottom -= (rcToolBar.bottom - rcToolBar.top - 1);
		}
		rcRomList.bottom -= (rcStatusBar.bottom - rcStatusBar.top);
		SetWindowPos(gui.hwndRomList, HWND_BOTTOM, 0, rcRomList.top, rcRomList.right, rcRomList.bottom, 0);
	}

	InitStatusBarParts();

	if(wParam == SIZE_MAXIMIZED && !guistatus.window_is_maximized) {
		guistatus.window_is_maximized = TRUE;
		InitStatusBarParts();
	} else if( wParam == SIZE_MINIMIZED && !guistatus.window_is_minimized) {
		guistatus.window_is_minimized = TRUE;
	} else if( guistatus.window_is_maximized || guistatus.window_is_minimized) {
		InitStatusBarParts();
	}

	REGISTRY_WriteDWORD( "1964WindowIsMaximized", guistatus.WindowIsMaximized);
}

void ProcessKeyboardInput(UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL ctrlkey;
	/* Disable Alt Key for menus in full screen */
     BYTE keymap[256];

    if ((wParam == VK_CONTROL) && (guistatus.IsFullScreen)) 
	{
		GetKeyboardState (keymap);
        keymap [(lParam & 0x1000000) ? VK_RCONTROL:VK_LCONTROL] &= ~0x80;
        SetKeyboardState (keymap);
    } 
	else if (wParam == VK_MENU) 
	{
		GetKeyboardState (keymap);
        keymap [(lParam & 0x1000000) ? VK_RMENU:VK_LMENU] &= ~0x80;
        SetKeyboardState (keymap);
    }

    if (message == WM_SYSKEYUP && wParam == VK_MENU) {
        /* ignore ALT key up event to stop it activating the menus */
		return;
    }

	ctrlkey = GetKeyState(VK_CONTROL) & 0xFF000000;
	switch(wParam)
	{
	case VK_F5:
		SaveState();
		break;

	case VK_F7:
		LoadState();
		break;

	case VK_ESCAPE:
    case VK_F4:
		if (guistatus.IsFullScreen)
		{
			if(emustatus.Emu_Is_Running) 
			{
				if(PauseEmulator())	
				{
					VIDEO_ChangeWindow(guistatus.IsFullScreen);
					ResumeEmulator(DO_NOTHING_AFTER_PAUSE);
				}
			}
		}
		if (wParam == VK_F4) 
			CloseROM();
		break;

	case 0x30:	
	case 0x31:	
	case 0x32:	
	case 0x33:
	case 0x34:	
	case 0x35:	
	case 0x36:	
	case 0x37:
	case 0x38:	
	case 0x39:
		if(!ctrlkey) 
		{
			StateSetNumber(wParam - 0x30);
		}
		break;

	default:
		CONTROLLER_WM_KeyUp(wParam, lParam);
		break;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
long FAR PASCAL MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static PAINTSTRUCT	ps;
	static int			ok = 0;
	static BOOL			gamepausebyinactive = FALSE;	/* static for this looks like a bad idea. */

	switch(message)
	{
	case WM_KILLFOCUS:
	//	if( guioptions.pause_at_inactive && emustatus.Emu_Is_Running && emustatus.Emu_Is_Paused == FALSE )
	//	{
	//		PauseEmulator();
	//		gamepausebyinactive = TRUE;
	//	}
		break;
	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0;
	break;
	case WM_SETFOCUS:
		if( emustatus.Emu_Is_Running )
		{
			//if( emustatus.Emu_Is_Paused  && gamepausebyinactive )
			//{
			//	Resume();
			//	gamepausebyinactive = FALSE;
			//}
		}
		else
		{
			ActiveApp = wParam;

			if(rlstatus.romlist_count > 0) RomListSelectRom(rlstatus.selected_rom_index);
			SetFocus(gui.hwndRomList);
		}
	break;

     case WM_SHOWWINDOW:
        SetForegroundWindow(hWnd);
        break;

	case WM_MOVE:
	{
		int xPos, yPos;

		xPos = (int) (short) LOWORD(lParam);		/* horizontal position */
		yPos = (int) (short) HIWORD(lParam);		/* vertical position */

		if(emustatus.Emu_Is_Running)
		{
			/*~~~~~~~~~~~*/
			VIDEO_MoveScreen(xPos, yPos);
		}
	}
	break;
	
	case WM_EXITSIZEMOVE:
	{
		REGISTRY_WriteSizeMove();
	}
	break;
		
	case WM_SIZE:
		//abcdefgh
		DefFrameProc(hWnd, gui.hStatusBar, message, wParam, lParam);
		DefFrameProc(hWnd, gui.hReBar, message, wParam, lParam);
		OnWindowSize(wParam);
	break;

	case WM_KEYDOWN:
		CONTROLLER_WM_KeyDown(wParam, lParam);
	break;
	
	case WM_KEYUP:
    case WM_SYSKEYUP:
		ProcessKeyboardInput(message, wParam, lParam);
	break;

	case WM_NOTIFY:
		//switch (((LPNMHDR) lParam)->code) 
		if( ((LPNMHDR) lParam)->code == TTN_GETDISPINFO ) 
		{ 
			//Tooltips slow down emulation a LOT!
			ProcessToolTips(lParam);
			break;
		}
		else if(((LPNMHDR) lParam)->hwndFrom == gui.hwndRomList)
		{
			OnNotifyRomList(hWnd, message, wParam, lParam);
		}
		else if(((LPNMHDR) lParam)->hwndFrom == gui.hStatusBar )
		{
			OnNotifyStatusBar(hWnd, message, wParam, lParam);
		}
		else if(((LPNMHDR) lParam)->hwndFrom == ListView_GetHeader(gui.hwndRomList) )
		{
			OnNotifyRomListHeader(hWnd, message, wParam, lParam);
		}
		else
		{
			return(DefWindowProc(hWnd, message, wParam, lParam));
		}
		break;

	case WM_COMMAND:
		ProcessMenuCommand(hWnd, message, wParam, lParam);
	break;

	case WM_CLOSE:
		Exit1964();
	break;

	case WM_SYSCOMMAND:
		switch (wParam) 
		{
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			if(emustatus.Emu_Is_Running)
				return 0;	//Disable screen saver
		case SC_MOVE:
		case SC_SIZE:
		case SC_MAXIMIZE:
		case SC_KEYMENU:
			if (guistatus.IsFullScreen)
				return 1;
		default:
			return(DefWindowProc(hWnd, message, wParam, lParam));
		}
	break;
	}
	
	return(DefWindowProc(hWnd, message, wParam, lParam));
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Resume(void)
{
	if(emustatus.Emu_Is_Running && emustatus.Emu_Is_Paused)
	{
		QueryPerformanceCounter(&LastSecondTime);
		ResumeEmulator(DO_NOTHING_AFTER_PAUSE);
	}
}

void AfterStop(void);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void KillCPUThread(void)
{
	if(emustatus.Emu_Is_Running)
	{
		SuspendThread(CPUThreadHandle);
		TerminateThread(CPUThreadHandle, 1);
		CloseHandle(CPUThreadHandle);

		if(currentromoptions.Code_Check == CODE_CHECK_PROTECT_MEMORY) UnprotectAllBlocks();

		AUDIO_RomClosed();
		CONTROLLER_RomClosed();
		VIDEO_RomClosed();

		AfterStop();
	}

	//To finish off the window placement and settings
	Rom_Loaded = TRUE;
	CloseROM();
	ReloadDefaultPlugins();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Kill(void)
{
	StopEmulator();
	CPUThreadHandle = NULL;
}
/*
 =======================================================================================================================
 =======================================================================================================================
 */

extern void (__stdcall StartCPUThread) (void *pVoid);
extern void (__stdcall StartAIInterruptThread) (void *pVoid);
extern void StartAudio(void);

void __cdecl Play(BOOL WithFullScreen)
{
	int core;
	DWORD ThreadID;

	if(Rom_Loaded)
	{
		if(emustatus.Emu_Is_Running) 
		{
			Stop();
            Sleep(1000); //Fix for Reset: if user holds down F2 long time it works, and does not crash.
		}

		PrepareBeforePlay(guistatus.IsFullScreen);

		if( !emustatus.Emu_Is_Resetting )	
			LoadROMSpecificPlugins();

		core = currentromoptions.Emulator;
		if(core == DYNACOMPILER)
		{					/* Dynarec */
			CheckMenuItem(gui.hMenu1964main, ID_INTERPRETER, MF_UNCHECKED);
			CheckMenuItem(gui.hMenu1964main, ID_DYNAMICCOMPILER, MF_CHECKED);
			emustatus.cpucore = DYNACOMPILER;
		}
		else
		{					/* Interpreter */
			CheckMenuItem(gui.hMenu1964main, ID_INTERPRETER, MF_CHECKED);
			CheckMenuItem(gui.hMenu1964main, ID_DYNAMICCOMPILER, MF_UNCHECKED);
			emustatus.cpucore = INTERPRETER;
		}

		if ((GFX_PluginRECT.UseThis == TRUE) && (emustatus.Emu_Is_Resetting == FALSE))
		{
			RECT Rect;

			GetWindowRect(gui.hwnd1964main, &Rect);
			SetWindowPos
			(
			gui.hwnd1964main,
			NULL,
			Rect.left,
			Rect.top,
			GFX_PluginRECT.rect.right - GFX_PluginRECT.rect.left + 1,
			GFX_PluginRECT.rect.bottom - GFX_PluginRECT.rect.top + 1,
			SWP_NOZORDER | SWP_SHOWWINDOW
			);
		}

		if (emustatus.Emu_Is_Resetting == 0)
			InitStatusBarParts();

		r4300i_Reset();

		emustatus.Emu_Keep_Running = TRUE;
		emustatus.processing_exception = FALSE;

		AUDIO_Initialize(Audio_Info);

		EnableMenuItem(gui.hMenu1964main, ID_ROM_PAUSE, MF_ENABLED);
		EnableMenuItem(gui.hMenu1964main, ID_ROM_STOP, MF_ENABLED);
		EnableRadioButtons(TRUE);
		CheckButton(ID_BUTTON_PLAY, TRUE);
		EnableStateMenu();

		if(Audio_Is_Initialized == 1 && CoreDoingAIUpdate == 0 )
		{
			StartAudio();	// Start the audio thread
		}

		r4300i_Reset();
		CPUThreadHandle = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)StartCPUThread,NULL,0, &ThreadID);


		sprintf(generalmessage, "%s - %s", gui.szWindowTitle, TranslateStringByString("Running"));

		if(WithFullScreen && (emustatus.Emu_Is_Resetting == 0))
		{
			if(guistatus.IsFullScreen == 0)
			{
				VIDEO_ChangeWindow(guistatus.IsFullScreen);
			}
		}
	}
	else
		DisplayError("Please load a ROM first.");
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */

extern int stopEntryCount;
void Stop()
{
	if(emustatus.Emu_Is_Running)
	{
		//MessageBox(0, "Trying to StopEmulator", "Stop", 0);	
		if( stopEntryCount > 0 )
		{
			StopEmulator();
		}
		else
		{
			StopEmulator();
			if (emustatus.Emu_Is_Resetting == 0)
			{
				if(guistatus.IsFullScreen)
				{
					VIDEO_ChangeWindow(guistatus.IsFullScreen);
				}
				//ShowCursor(TRUE);

				AfterStop();
			}
		}

		// Enable screen saver
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, WindowScreenSaverStatus, 0, 0); 
		REGISTRY_WriteDWORD("1964RunningStatus", FALSE);
	}
	else if(Is_Reading_Rom_File)
	{
		SetEvent( StopEmulatorEvent );
		To_Stop_Reading_Rom_File = TRUE;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void __cdecl RomListGetGoodRomNameToDisplay(char *buf, int index)
{
	switch( rlstatus.romlistNameToDisplay )
	{
	case ROMLIST_DISPLAY_ALTER_NAME:
		if(strlen(romlist[index]->pinientry->Alt_Title) > 1)
		{
			strcpy(buf, romlist[index]->pinientry->Alt_Title);
			break;
		}
	case ROMLIST_DISPLAY_INTERNAL_NAME:
		if(InternalNameIsValid(romlist[index]->pinientry->Game_Name))
		{
			strcpy(buf, romlist[index]->pinientry->Game_Name);
			break;
		}		
	case ROMLIST_DISPLAY_FILENAME:
		{
			char	drive[_MAX_DIR], dir[_MAX_DIR];
			char	fname[_MAX_DIR], ext[_MAX_EXT];
			
			_splitpath(romlist[index]->romfilename, drive, dir, fname, ext);
			strcat(fname, ext);
			strcpy(buf, fname);
		}
		break;
	}
}
/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CloseROM(void)
{
	WINDOWPLACEMENT placement;

	GetWindowPlacement(gui.hwnd1964main, &placement);
	
	if(emustatus.Emu_Is_Running)
	{
		Stop();

		if (placement.showCmd == SW_SHOWMAXIMIZED)
		{
			guistatus.WindowIsMaximized = TRUE;
			ShowWindow(gui.hwnd1964main, SW_SHOWNORMAL);
			ShowWindow(gui.hwnd1964main, SW_SHOWMAXIMIZED);
		}

		Close_iPIF();
		FreeVirtualRomMemory();

		Rom_Loaded = FALSE;

		EnableMenuItem(gui.hMenu1964main, ID_OPENROM, MF_ENABLED);
		EnableButton(ID_BUTTON_OPEN_ROM, TRUE);
		EnableButton(ID_BUTTON_SETUP_PLUGINS, TRUE);
		EnableMenuItem(gui.hMenu1964main, ID_ROM_START, MF_GRAYED);
		EnableRadioButtons(FALSE);

		EnableMenuItem(gui.hMenu1964main, ID_ROM_PAUSE, MF_GRAYED);

		EnableMenuItem(gui.hMenu1964main, ID_CLOSEROM, MF_GRAYED);
	}

}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void OpenROM(void)
{
	if(Rom_Loaded) 
	{
		CloseROM();
	}
	if(WinLoadRom() == TRUE)	/* If the user opened a rom, */
	{
		EnableMenuItem(gui.hMenu1964main, ID_ROM_START, MF_ENABLED);
		EnableRadioButtons(TRUE);
		EnableMenuItem(gui.hMenu1964main, ID_CLOSEROM, MF_ENABLED);
		EnableMenuItem(gui.hMenu1964main, ID_FILE_ROMINFO, MF_ENABLED);
		EnableButton(ID_BUTTON_ROM_PROPERTIES, TRUE);
		EnableMenuItem(gui.hMenu1964main, ID_FILE_CHEAT, MF_ENABLED);

		Play(emuoptions.auto_full_screen); /* autoplay */
	}
	else
	{
		EnableButton(ID_BUTTON_OPEN_ROM, TRUE);
		EnableMenuItem(gui.hMenu1964main, ID_OPENROM, MF_ENABLED);
		EnableButton(ID_BUTTON_SETUP_PLUGINS, TRUE);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
BOOL WinLoadRom(void)
{
	OPENFILENAME	ofn;
	char			szFileName[MAXFILENAME];
	char			szFileTitle[MAXFILENAME];
	char			filter[256];

	memset(&szFileName, 0, sizeof(szFileName));
	memset(&szFileTitle, 0, sizeof(szFileTitle));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = gui.hwnd1964main;
	sprintf(filter, "ROMs (.zip .v64 .bin .rom .z64 .n64 .usa .pal .j64)%c*.ZIP;*.V64;*.BIN;*.ROM;*.Z64;*.N64;*.USA;*.PAL;*.J64%c%s (*.*)%c*.*%c%c", 0, 0, TranslateStringByString("All Files"),0,0,0);
	ofn.lpstrFilter = filter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAXFILENAME;
	ofn.lpstrInitialDir = directories.rom_directory_to_use;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = MAXFILENAME;
	ofn.lpstrTitle = TranslateStringByString("Open ROM");
	ofn.lpstrDefExt = "TXT";
	ofn.Flags = OFN_HIDEREADONLY;

	if(!GetOpenFileName((LPOPENFILENAME) & ofn))
	{
		return FALSE;
	}

	_getcwd(directories.rom_directory_to_use, MAX_PATH);
	if ( strcmp(directories.last_rom_directory, directories.rom_directory_to_use) != 0)
	{ 
		NeedFreshromListAfterStop = TRUE;
	}
	strcpy(directories.last_rom_directory, directories.rom_directory_to_use);

	WriteConfiguration();

	if(WinLoadRomStep2(szFileName))
	{
		/*~~~~~~~~~~~~~~~~~*/
		INI_ENTRY	*pentry;
		long		filesize;
		/*~~~~~~~~~~~~~~~~~*/

		/* Check and create romlist entry for this new loaded rom */
		pentry = GetNewIniEntry();
		ReadRomHeaderInMemory(pentry);
		filesize = ReadRomHeader(szFileName, pentry);
		RomListAddEntry(pentry, szFileName, filesize);
		DeleteIniEntryByEntry(pentry);

		/* Read hack code for this rom */
		CodeList_ReadCode((char*)rominfo.name,cheatfilename);
		RefreshRecentGameMenus(szFileName);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*
=======================================================================================================================
=======================================================================================================================
*/
BOOL __cdecl WinLoadRomStep2(char *szFileName)
{
	EnableMenuItem(gui.hMenu1964main, ID_ROM_PAUSE, MF_ENABLED);

	lstrcpy(gui.szWindowTitle, gui.szBaseWindowTitle);
	lstrcat(gui.szWindowTitle, " - ");
	
	if(ReadRomData(szFileName) == FALSE) 
	{
		Rom_Loaded = FALSE;
		return FALSE;
	}
	
	lstrcat(gui.szWindowTitle, (LPCSTR)rominfo.name);
	
	memcpy(&HeaderDllPass[0], &gMemoryState.ROM_Image[0], 0x40);
	EnableMenuItem(gui.hMenu1964main, ID_CLOSEROM, MF_ENABLED);
	
	Rom_Loaded = TRUE;
	gHWS_pc = 0xA4000040;	/* We do it in r4300i_inithardware */
	
	UpdateCIC();
	sprintf(generalmessage, "%s - %s", gui.szWindowTitle, TranslateStringByString("Loaded"));
	Set_Ready_Message();
	
	EnableMenuItem(gui.hMenu1964main, ID_ROM_PAUSE, MF_GRAYED);
	return TRUE;
}

BOOL StartGameByCommandLine()
{
	char szFileName[300];
	GetCmdLineParameter(CMDLINE_GAME_FILENAME, szFileName);
	if( strlen(szFileName) == 0 )
	{
		return FALSE;
	}

	if(WinLoadRomStep2(szFileName))
	{
		/*~~~~~~~~~~~~~~~~~*/
		INI_ENTRY	*pentry;
		long		filesize;
		char		tempstr[20];
		/*~~~~~~~~~~~~~~~~~*/
		
		/* Check and create romlist entry for this new loaded rom */
		pentry = GetNewIniEntry();
		ReadRomHeaderInMemory(pentry);
		filesize = ReadRomHeader(szFileName, pentry);
		RomListAddEntry(pentry, szFileName, filesize);
		DeleteIniEntryByEntry(pentry);
		
		/* Read hack code for this rom */
		CodeList_ReadCode((char*)rominfo.name,cheatfilename);
		RefreshRecentGameMenus(szFileName);

		EnableMenuItem(gui.hMenu1964main, ID_ROM_START, MF_ENABLED);
		EnableRadioButtons(TRUE);
		EnableMenuItem(gui.hMenu1964main, ID_CLOSEROM, MF_ENABLED);
		
		GetCmdLineParameter(CMDLINE_FULL_SCREEN_FLAG, tempstr);
		if( strlen(tempstr) > 0 )
		{
			Play(TRUE);
		}
		else
		{
			Play(emuoptions.auto_full_screen); /* autoplay */
		}

		return TRUE;
	}
	else
	{
		return FALSE;
	}
	
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void OpenRecentGame(int id)
{
	if(emustatus.Emu_Is_Running) 
		return;

	if(id >= 0 && id < MAX_RECENT_GAME_LIST )
	{
		if(WinLoadRomStep2(recent_game_lists[id]))
		{
			/*~~~~~~~~~~~~~~~~~*/
			INI_ENTRY	*pentry;
			long		filesize;
			/*~~~~~~~~~~~~~~~~~*/

			/* Check and create romlist entry for this new loaded rom */
			pentry = GetNewIniEntry();
			ReadRomHeaderInMemory(pentry);
			filesize = ReadRomHeader(recent_game_lists[id], pentry);
			RomListAddEntry(pentry, recent_game_lists[id], filesize);
			DeleteIniEntryByEntry(pentry);

			/* Read hack code for this rom */
			CodeList_ReadCode((char*)rominfo.name,cheatfilename);

			EnableMenuItem(gui.hMenu1964main, ID_ROM_START, MF_ENABLED);
			EnableRadioButtons(TRUE);
			EnableMenuItem(gui.hMenu1964main, ID_CLOSEROM, MF_ENABLED);
			EnableMenuItem(gui.hMenu1964main, ID_FILE_ROMINFO, MF_ENABLED);
			EnableButton(ID_BUTTON_ROM_PROPERTIES, TRUE);
			EnableMenuItem(gui.hMenu1964main, ID_FILE_CHEAT, MF_ENABLED);

			strcpy(generalmessage, recent_game_lists[id]);
			RefreshRecentGameMenus(generalmessage);

			Play(emuoptions.auto_full_screen); /* autoplay */
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */

extern BYTE __cdecl GetButtonState( int	nID );

void SaveState(void)
{
	if(Rom_Loaded && emustatus.Emu_Is_Running)
	{ 
        //Please don't change this, It's needed because menu pauses interfere with other Pauses.
        if ( !(GetButtonState(ID_BUTTON_PAUSE)&TBSTATE_CHECKED))
		{
            emustatus.Emu_Is_Paused = 0;
            PauseEmulator();
        }

		sprintf(generalmessage, "%s - %s %d", gui.szWindowTitle, TranslateStringByString("Saving State"), StateFileNumber);
		SetStatusBarText(0, generalmessage);

		FileIO_gzSaveState();
         
        if ( !(GetButtonState(ID_BUTTON_PAUSE)&TBSTATE_CHECKED))
        {
            emustatus.Emu_Is_Paused = 1;
			ResumeEmulator(DO_NOTHING_AFTER_PAUSE);
		}
	}
	else
	{
		sprintf(generalmessage, "%s - %s %d", gui.szWindowTitle, TranslateStringByString("Saving State"), StateFileNumber);
		SetStatusBarText(0, generalmessage);

		FileIO_gzSaveState();
	}
    Set_Ready_Message();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void LoadState(void)
{
	/*~~~~~~~~~~~~~~~~~~~~*/
	int was_running = FALSE;
	/*~~~~~~~~~~~~~~~~~~~~*/

	if(Rom_Loaded)
	{
		if(emustatus.Emu_Is_Running)
		{
            //Please don't change this, It's needed because menu pauses interfere with other Pauses.
            if ( !(GetButtonState(ID_BUTTON_PAUSE)&TBSTATE_CHECKED))
            {
                emustatus.Emu_Is_Paused = 0;
                PauseEmulator();
            }
			{
				sprintf(generalmessage, "%s - %s %d", gui.szWindowTitle, TranslateStringByString("Loading State"), StateFileNumber);
				SetStatusBarText(0, generalmessage);
				FileIO_gzLoadState();
				Init_Count_Down_Counters();
                if ( !(GetButtonState(ID_BUTTON_PAUSE)&TBSTATE_CHECKED))
                {
                    emustatus.Emu_Is_Paused = 1;
                    ResumeEmulator(DO_NOTHING_AFTER_PAUSE);
                }
			}
		}
		else
		{
			sprintf(generalmessage, "%s - %s %d", gui.szWindowTitle, TranslateStringByString("Loading State"), StateFileNumber);
			SetStatusBarText(0, generalmessage);
			FileIO_gzLoadState();
			Init_Count_Down_Counters();
		}
	}
    Set_Ready_Message();
}

unsigned int	statesavemenulist[10] =
{
	ID_SAVE_STATE_0,
	ID_SAVE_STATE_1,
	ID_SAVE_STATE_2,
	ID_SAVE_STATE_3,
	ID_SAVE_STATE_4,
	ID_SAVE_STATE_5,
	ID_SAVE_STATE_6,
	ID_SAVE_STATE_7,
	ID_SAVE_STATE_8,
	ID_SAVE_STATE_9
};
unsigned int	stateloadmenulist[10] =
{
	ID_LOAD_STATE_0,
	ID_LOAD_STATE_1,
	ID_LOAD_STATE_2,
	ID_LOAD_STATE_3,
	ID_LOAD_STATE_4,
	ID_LOAD_STATE_5,
	ID_LOAD_STATE_6,
	ID_LOAD_STATE_7,
	ID_LOAD_STATE_8,
	ID_LOAD_STATE_9
};

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void StateSetNumber(int number)
{
	CheckMenuItem(gui.hMenu1964main, statesavemenulist[StateFileNumber], MF_UNCHECKED);
	CheckMenuItem(gui.hMenu1964main, stateloadmenulist[StateFileNumber], MF_UNCHECKED);
	StateFileNumber = number;
	CheckMenuItem(gui.hMenu1964main, statesavemenulist[StateFileNumber], MF_CHECKED);
	CheckMenuItem(gui.hMenu1964main, stateloadmenulist[StateFileNumber], MF_CHECKED);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void EnableStateMenu(void)
{
	/*~~*/
	int i;
	/*~~*/

	EnableMenuItem(gui.hMenu1964main, ID_SAVESTATE, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, ID_LOADSTATE, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, ID_CPU_IMPORTPJ64STATE, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, ID_CPU_EXPORTPJ64STATE, MF_ENABLED);

	for(i = 0; i < 10; i++)
	{
		EnableMenuItem(gui.hMenu1964main, statesavemenulist[i], MF_ENABLED);
		EnableMenuItem(gui.hMenu1964main, stateloadmenulist[i], MF_ENABLED);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void DisableStateMenu(void)
{
	int i;

	EnableMenuItem(gui.hMenu1964main, ID_SAVESTATE, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_LOADSTATE, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_CPU_IMPORTPJ64STATE, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_CPU_EXPORTPJ64STATE, MF_GRAYED);

	for(i = 0; i < 10; i++)
	{
		EnableMenuItem(gui.hMenu1964main, statesavemenulist[i], MF_GRAYED);
		EnableMenuItem(gui.hMenu1964main, stateloadmenulist[i], MF_GRAYED);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SaveStateByNumber(WPARAM wparam)
{
	/*~~*/
	int i;
	/*~~*/

	for(i = 0; i < 10; i++)
	{
		CheckMenuItem(gui.hMenu1964main, statesavemenulist[i], MF_UNCHECKED);
		CheckMenuItem(gui.hMenu1964main, stateloadmenulist[i], MF_UNCHECKED);
		if(statesavemenulist[i] == wparam)
		{
			StateFileNumber = i;
			CheckMenuItem(gui.hMenu1964main, statesavemenulist[i], MF_CHECKED);
			CheckMenuItem(gui.hMenu1964main, stateloadmenulist[i], MF_CHECKED);
		}
	}

	SaveState();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void LoadStateByNumber(WPARAM wparam)
{
	/*~~*/
	int i;
	/*~~*/

	for(i = 0; i < 10; i++)
	{
		CheckMenuItem(gui.hMenu1964main, statesavemenulist[i], MF_UNCHECKED);
		CheckMenuItem(gui.hMenu1964main, stateloadmenulist[i], MF_UNCHECKED);
		if(stateloadmenulist[i] == wparam)
		{
			StateFileNumber = i;
			CheckMenuItem(gui.hMenu1964main, statesavemenulist[i], MF_CHECKED);
			CheckMenuItem(gui.hMenu1964main, stateloadmenulist[i], MF_CHECKED);
		}
	}

	LoadState();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
//CHECKME
void SaveStateByDialog(int format)
{
	if(!Rom_Loaded || !PauseEmulator()) 
		return;
	else
	{
		OPENFILENAME	ofn;
		char			szFileName[MAXFILENAME];
		char			szFileTitle[MAXFILENAME];
		char			szPath[_MAX_PATH];
		char			filter[256];

		memset(&szFileName, 0, sizeof(szFileName));
		memset(&szFileTitle, 0, sizeof(szFileTitle));
		memset(szPath, 0, _MAX_PATH);

		strcpy(szPath, directories.save_directory_to_use);

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = gui.hwnd1964main;

		if(format == SAVE_STATE_1964_FORMAT)
		{
			sprintf(filter, "1964 %s (*.sav?)%c*.SAV?%c%s (*.*)%c*.*%c%c", TranslateStringByString("State File"), 0, 0, TranslateStringByString("All Files"), 0, 0, 0);
			ofn.lpstrFilter = filter;
		}
		else
		{
			sprintf(filter, "Project 64 %s (*.pj?)%c*.PJ?%c%s (*.*)%c*.*%c%c", TranslateStringByString("State File"), 0, 0, TranslateStringByString("All Files"), 0, 0, 0);
			ofn.lpstrFilter = filter;
		}

		ofn.lpstrCustomFilter = NULL;
		ofn.nMaxCustFilter = 0;
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAXFILENAME;
		ofn.lpstrInitialDir = szPath;
		ofn.lpstrFileTitle = szFileTitle;
		ofn.nMaxFileTitle = MAXFILENAME;
		ofn.lpstrTitle = TranslateStringByString("Saving State");
		ofn.lpstrDefExt = "";
		ofn.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY;

		if(GetSaveFileName((LPOPENFILENAME) & ofn))
		{
			char drive[_MAX_DRIVE], dir[_MAX_DIR];
			char filename[_MAX_FNAME], ext[_MAX_EXT];
			_splitpath(szFileName, drive, dir, filename, ext);
			_strlwr(ext);	/* Convert file extension to lower case */

			if(format == SAVE_STATE_1964_FORMAT)
			{
				if( strcmp (ext,".sav") != 0)
					strcat(szFileName, ".sav");
				FileIO_gzSaveStateFile_099(szFileName);

			}
			else
			{
				if( strcmp(ext,".pj") != 0 )
					strcat(szFileName, ".pj");
				FileIO_ExportPJ64State(szFileName);
			}
		}

		if(emustatus.Emu_Is_Running) 
		{
			emustatus.Emu_Is_Paused = 1;
			ResumeEmulator(DO_NOTHING_AFTER_PAUSE);
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void LoadStateByDialog(int format)
{
	if(!Rom_Loaded || !PauseEmulator())
		return;
	else
	{
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		OPENFILENAME	ofn;
		char			szFileName[MAXFILENAME];
		char			szFileTitle[MAXFILENAME];
		char			szPath[_MAX_PATH];
		char			filter[256];
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

		memset(&szFileName, 0, sizeof(szFileName));
		memset(&szFileTitle, 0, sizeof(szFileTitle));
		memset(szPath, 0, _MAX_PATH);

		strcpy(szPath, directories.save_directory_to_use);

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = gui.hwnd1964main;

		if(format == SAVE_STATE_1964_FORMAT)
		{
			sprintf(filter, "1964 %s (*.sav?)%c*.SAV?%c%s (*.*)%c*.*%c%c", TranslateStringByString("State File"), 0, 0, TranslateStringByString("All Files"), 0, 0, 0);
			ofn.lpstrFilter = filter;
		}
		else
		{
			sprintf(filter, "Project 64 %s (*.pj?)%c*.PJ;*.PJ?%c%s (*.*)%c*.*%c%c", TranslateStringByString("State File"), 0, 0, TranslateStringByString("All Files"), 0, 0, 0);
			ofn.lpstrFilter = filter;
		}

		ofn.lpstrCustomFilter = NULL;
		ofn.nMaxCustFilter = 0;
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAXFILENAME;
		ofn.lpstrInitialDir = szPath;
		ofn.lpstrFileTitle = szFileTitle;
		ofn.nMaxFileTitle = MAXFILENAME;
		ofn.lpstrTitle = TranslateStringByString("Loading State");
		ofn.lpstrDefExt = "";
		ofn.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY;

		if(GetOpenFileName((LPOPENFILENAME) & ofn))
		{
			if(format == SAVE_STATE_1964_FORMAT)
				FileIO_gzLoadStateFile(szFileName);
			else
				FileIO_ImportPJ64State(szFileName);
		}

		if(emustatus.Emu_Is_Running) 
		{
			emustatus.Emu_Is_Paused = 1;
			ResumeEmulator(REFRESH_DYNA_AFTER_PAUSE);
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
INT CALLBACK BrowseCallbackProc(HWND hwnd, 
								UINT uMsg,
								LPARAM lp, 
								LPARAM pData) 
{
	TCHAR szDir[MAX_PATH];

	switch(uMsg) 
	{
	case BFFM_INITIALIZED: 
		if (GetCurrentDirectory(sizeof(szDir)/sizeof(TCHAR), szDir))
		{
			// WParam is TRUE since you are passing a path.
			// It would be FALSE if you were passing a pidl.
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)directories.rom_directory_to_use);
		}
		break;

	case BFFM_SELCHANGED: 
		// Set the status window to the currently selected path.
		if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
		{
			SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
		}
		break;
	}
	return 0;
}

BOOL SelectDirectory(char *title, char buffer[MAX_PATH])
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	BROWSEINFO		bi;
	char			pszBuffer[MAX_PATH];
	LPITEMIDLIST	pidl;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	ZeroMemory(&bi,sizeof(bi));
	bi.hwndOwner = gui.hwnd1964main;
	bi.pidlRoot = NULL;
	//bi.pidlRoot = directories.rom_directory_to_use;
	bi.pszDisplayName = pszBuffer;
	bi.lpszTitle = title;
	bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
	bi.lpfn = NULL;
	bi.lParam = 0;
	//bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
	bi.lpfn = BrowseCallbackProc;

	if((pidl = SHBrowseForFolder(&bi)) != NULL)
	{
		if(SHGetPathFromIDList(pidl, buffer))
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void ChangeDirectory(void)
{
	/*~~~~~~~~~~~~~~~~~~~*/
	char	path[MAX_PATH];
	/*~~~~~~~~~~~~~~~~~~~*/

	if(emustatus.Emu_Is_Running) return;

	if(SelectDirectory(TranslateStringByString("Select a ROM folder"), path))
	{
		strcpy(directories.rom_directory_to_use, path);
		strcpy(directories.last_rom_directory, path);

		RefreshRecentRomDirectoryMenus(path);

		ClearRomList();
		SetStatusBarText(0, TranslateStringByString("Looking for ROM files in the ROM directory and Generating List"));
		RomListReadDirectory(directories.rom_directory_to_use,FALSE);
		NewRomList_ListViewFreshRomList();
		Set_Ready_Message();
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void ChangeToRecentDirectory(int id)
{
	if(emustatus.Emu_Is_Running) return;

	if(id >= 0 && id < MAX_RECENT_ROM_DIR)
	{
		strcpy(generalmessage, recent_rom_directory_lists[id]);
		strcpy(directories.rom_directory_to_use, generalmessage);
		strcpy(directories.last_rom_directory, generalmessage);
		WriteConfiguration();
		RefreshRecentRomDirectoryMenus(generalmessage);

		ClearRomList();
		SetStatusBarText(0, TranslateStringByString("Looking for ROM files in the ROM directory and Generating List"));
		RomListReadDirectory(directories.rom_directory_to_use,TRUE);
		NewRomList_ListViewFreshRomList();
		Set_Ready_Message();
	}
}

void OptionsDialog_OnInit(HWND hDlg)
{
	SendDlgItemMessage( hDlg, IDC_OPTION_AUTOFULLSCREEN, BM_SETCHECK,
		emuoptions.auto_full_screen ? BST_CHECKED : BST_UNCHECKED, 0);

	SendDlgItemMessage(	hDlg, IDC_DEFAULTOPTIONS_PAUSEWHENINACTIVE,	BM_SETCHECK,
		guioptions.pause_at_inactive ? BST_CHECKED : BST_UNCHECKED,	0);

	SendDlgItemMessage(	hDlg, IDC_ENABLE_DIRECTORY_LIST, BM_SETCHECK,
		guioptions.show_recent_rom_directory_list ? BST_CHECKED : BST_UNCHECKED, 0);

	SendDlgItemMessage(	hDlg, IDC_ENABLE_GAME_LIST,	BM_SETCHECK,
		guioptions.show_recent_game_list ? BST_CHECKED : BST_UNCHECKED, 0);

	SendDlgItemMessage( hDlg, IDC_ENABLE_DETAIL_STATUS,	BM_SETCHECK,
		guioptions.display_detail_status ? BST_CHECKED : BST_UNCHECKED,	0);

	SendDlgItemMessage(	hDlg, IDC_ENABLE_PROFILER, BM_SETCHECK,
		guioptions.display_profiler_status ? BST_CHECKED : BST_UNCHECKED, 0);

    SendDlgItemMessage(	hDlg, IDC_NONE, BM_SETCHECK,
        (!guioptions.display_profiler_status && !guioptions.display_detail_status) ? BST_CHECKED : BST_UNCHECKED, 0);

}

void FoldersDialog_OnInit(HWND hDlg)
{
	SendDlgItemMessage( hDlg, IDC_OPTIONS_USE_DEFAULT_SAVE_DIRECTORY, BM_SETCHECK,
		guioptions.use_default_save_directory ? BST_CHECKED : BST_UNCHECKED, 0);

	SendDlgItemMessage( hDlg, IDC_DEFAULTOPTIONS_USE1964PLUGINDIRECTORY, BM_SETCHECK,
		guioptions.use_default_plugin_directory ? BST_CHECKED : BST_UNCHECKED, 0);

	SendDlgItemMessage( hDlg, IDC_DEFAULTOPTIONS_UseLastRomDirectory, BM_SETCHECK,
		guioptions.use_last_rom_directory ? BST_CHECKED : BST_UNCHECKED, 0);

	SetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_STATESAVEDIRECTORY, user_set_state_save_directory);
	SetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_ROMDIRECTORY, user_set_rom_directory);
	SetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_SAVEDIRECTORY, user_set_save_directory);
	SetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_PLUGINDIRECTORY, user_set_plugin_directory);
}

void FoldersDialog_OnApply(HWND hDlg)
{
	if( guioptions.use_default_save_directory
		!= ( SendDlgItemMessage( hDlg, IDC_OPTIONS_USE_DEFAULT_SAVE_DIRECTORY, BM_GETCHECK, 0, 0)
			 == BST_CHECKED))
	{
		guioptions.use_default_save_directory ^= 1;
		REGISTRY_WriteDWORD( "UseDefaultSaveDirectory", guioptions.use_default_save_directory);
	}

	if( guioptions.use_default_plugin_directory
		!= ( SendDlgItemMessage( hDlg, IDC_DEFAULTOPTIONS_USE1964PLUGINDIRECTORY, BM_GETCHECK, 0, 0)
			 == BST_CHECKED))
	{
		guioptions.use_default_plugin_directory ^= 1;
		REGISTRY_WriteDWORD( "UseDefaultPluginDirectory", guioptions.use_default_plugin_directory);
	}

	if( guioptions.use_last_rom_directory
		!= ( SendDlgItemMessage( hDlg, IDC_DEFAULTOPTIONS_UseLastRomDirectory, BM_GETCHECK, 0, 0)
			 == BST_CHECKED))
	{
		guioptions.use_last_rom_directory ^= 1;
		REGISTRY_WriteDWORD( "UseLastRomDirectory", guioptions.use_last_rom_directory);
	}


	GetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_ROMDIRECTORY, user_set_rom_directory, _MAX_PATH);
	REGISTRY_WriteStringByIndex( IDS_KEY_ROM_PATH, user_set_rom_directory);
	strcpy( directories.rom_directory_to_use, 
		guioptions.use_last_rom_directory ?
			directories.last_rom_directory : user_set_rom_directory);

	GetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_SAVEDIRECTORY, user_set_save_directory, _MAX_PATH);
	REGISTRY_WriteStringByIndex( IDS_KEY_SAVE_PATH, user_set_save_directory);
	strcpy( directories.save_directory_to_use, guioptions.use_default_save_directory ?	default_save_directory : user_set_save_directory);
	if( !FileIO_CheckAndCreateFolder(directories.save_directory_to_use) )
	{
		if( strcmp(directories.save_directory_to_use, default_save_directory) != 0 )
		{
			strcpy(directories.save_directory_to_use, default_save_directory);
			REGISTRY_WriteStringByIndex( IDS_KEY_SAVE_PATH, default_save_directory);
			if( !FileIO_CheckAndCreateFolder(directories.save_directory_to_use) )
			{
				DisplayError("Can not create save directory: %s, please check it manually", default_save_directory);
			}
		}
	}

	GetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_STATESAVEDIRECTORY, user_set_state_save_directory, _MAX_PATH);
	REGISTRY_WriteStringByIndex( IDS_KEY_STATE_SAVE_PATH, user_set_state_save_directory);
	strcpy( directories.state_save_directory_to_use, 
		guioptions.use_default_state_save_directory ?
			default_state_save_directory : user_set_state_save_directory);

	GetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_PLUGINDIRECTORY, user_set_plugin_directory, _MAX_PATH);
	REGISTRY_WriteStringByIndex( IDS_KEY_PLUGIN_PATH, user_set_plugin_directory);
	strcpy( directories.plugin_directory_to_use, 
		guioptions.use_default_plugin_directory ?
			default_plugin_directory : user_set_plugin_directory);
}

void OptionsDialog_OnApply(HWND hDlg)
{
	if(	emuoptions.auto_full_screen
		!= ( SendDlgItemMessage( hDlg, IDC_OPTION_AUTOFULLSCREEN, BM_GETCHECK, 0, 0)
			== BST_CHECKED))
	{
		emuoptions.auto_full_screen ^= 1;
		REGISTRY_WriteDWORD( "AutoFullScreen", emuoptions.auto_full_screen);
	}

	if( guioptions.pause_at_inactive
		!= ( SendDlgItemMessage( hDlg, IDC_DEFAULTOPTIONS_PAUSEWHENINACTIVE, BM_GETCHECK, 0, 0) == BST_CHECKED))
	{
		guioptions.pause_at_inactive ^= 1;
		REGISTRY_WriteDWORD( "PauseWhenInactive", guioptions.pause_at_inactive);
	}

	if( guioptions.display_detail_status
		!= ( SendDlgItemMessage( hDlg, IDC_ENABLE_DETAIL_STATUS, BM_GETCHECK, 0, 0) == BST_CHECKED))
	{
		guioptions.display_detail_status ^= 1;
		REGISTRY_WriteDWORD( "DisplayDetailStatus", guioptions.display_detail_status);
	}

	if( guioptions.display_detail_status )
	{
		guioptions.display_profiler_status = 0;
		REGISTRY_WriteDWORD( "DisplayProfilerStatus", guioptions.display_profiler_status);
	}
	else
	{
		if( guioptions.display_profiler_status
			!= ( SendDlgItemMessage( hDlg, IDC_ENABLE_PROFILER, BM_GETCHECK, 0, 0) == BST_CHECKED))
		{
			guioptions.display_profiler_status ^= 1;
			REGISTRY_WriteDWORD( "DisplayProfilerStatus", guioptions.display_profiler_status);
		}
	}

	if( guioptions.show_recent_rom_directory_list 
		!= ( SendDlgItemMessage( hDlg, IDC_ENABLE_DIRECTORY_LIST, BM_GETCHECK, 0, 0)
		     == BST_CHECKED))
	{
		guioptions.show_recent_rom_directory_list ^= 1;
		REGISTRY_WriteDWORD( "RomDirectoryListMenu", 
			guioptions.show_recent_rom_directory_list);
		if(guioptions.show_recent_rom_directory_list)
			RegerateRecentRomDirectoryMenus();
		else
			DeleteRecentRomDirectoryMenus();
	}

	if(	guioptions.show_recent_game_list 
		!= ( SendDlgItemMessage( hDlg, IDC_ENABLE_GAME_LIST, BM_GETCHECK, 0, 0)
			 == BST_CHECKED))
	{
		guioptions.show_recent_game_list ^= 1;
		REGISTRY_WriteDWORD( "GameListMenu",
			guioptions.show_recent_game_list);
		if(guioptions.show_recent_game_list)
			RegenerateRecentGameMenus();
		else
			DeleteRecentGameMenus();
	}
	
    if (!guioptions.display_profiler_status && !guioptions.display_detail_status)
    {
        Set_Ready_Message();
    }
}

LRESULT APIENTRY FoldersProc(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
	char	path[MAX_PATH];

	switch(message)
	{
	case WM_INITDIALOG:
		TranslateDialag(hDlg, "FOLDERS");
		FoldersDialog_OnInit(hDlg);
        return(TRUE);
    
    //Propertysheet handling
	case WM_NOTIFY:
		{
		LPNMHDR lpnm = (LPNMHDR) lParam;

        switch (lpnm->code)
            {
			case PSN_APPLY:
				SendMessage(hDlg, WM_COMMAND, IDOK, lParam);
                EndDialog(lpnm->hwndFrom, TRUE);
				break;

            case PSN_RESET :
                //Handle a Cancel button click, if necessary
                EndDialog(lpnm->hwndFrom, TRUE);
				break;
			}
		}
		return(TRUE);

	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_DEFAULTOPTIONS_BUTTON_SAVEDIR:
			if(SelectDirectory(TranslateStringByString("Select Game Save Folder"), path)) {
				strcat(path, "\\");
				SetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_SAVEDIRECTORY, path);
			}
			break;
		case IDC_DEFAULTOPTIONS_BUTTON_PLUGINDIR:
			if(SelectDirectory(TranslateStringByString("Select Plugin Folder"), path)) {
				strcat(path, "\\");
				SetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_PLUGINDIRECTORY, path);
			}
			break;
		case IDC_DEFAULTOPTIONS_BUTTON_ROMDIR:
			if(SelectDirectory(TranslateStringByString("Select ROM Folder"), path))
			{
				strcat(path, "\\");
				SetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_ROMDIRECTORY, path);
			}
			break;


        case IDOK:
			FoldersDialog_OnApply(hDlg);
			EndDialog(hDlg, TRUE);
			return(TRUE);

		case IDCANCEL:
			EndDialog(hDlg, TRUE);
			return(TRUE);
	    }
    }


    return(FALSE);
}



// When a ROM has been loaded, use this Procedure for 
// PropertySheets that are disabled. 
// For example, the Plugins dialog is disabled when a rom is loaded.
LRESULT APIENTRY UnavailableProc(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		TranslateDialag(hDlg, "UNAVAILABLE");
        return(TRUE);

    //Propertysheet handling
	case WM_NOTIFY:
		{
		LPNMHDR lpnm = (LPNMHDR) lParam;

        switch (lpnm->code)
            {
			case PSN_APPLY:
				SendMessage(hDlg, WM_COMMAND, IDOK, lParam);
                EndDialog(lpnm->hwndFrom, TRUE);
				break;

            case PSN_RESET :
                //Handle a Cancel button click, if necessary
                EndDialog(lpnm->hwndFrom, TRUE);
				break;
			}
		}
        return(TRUE);

	case WM_COMMAND:
		switch(wParam)
		{

		case IDOK:
            EndDialog(hDlg, TRUE);
			return(TRUE);

		case IDCANCEL:
			EndDialog(hDlg, TRUE);
			return(TRUE);
		}
	}

	return(FALSE);
}

LRESULT APIENTRY OptionsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		TranslateDialag(hDlg, "OPTIONS");
		OptionsDialog_OnInit(hDlg);
		return(TRUE);


    //Propertysheet handling
	case WM_NOTIFY:
		{
		LPNMHDR lpnm = (LPNMHDR) lParam;

        switch (lpnm->code)
            {
			case PSN_APPLY:
				SendMessage(hDlg, WM_COMMAND, IDOK, lParam);
                EndDialog(lpnm->hwndFrom, TRUE);
				break;

            case PSN_RESET :
                //Handle a Cancel button click, if necessary
                EndDialog(lpnm->hwndFrom, TRUE);
				break;
			}
		}
        return(TRUE);

	case WM_COMMAND:
		switch(wParam)
		{

		case IDOK:
			OptionsDialog_OnApply(hDlg);
			EndDialog(hDlg, TRUE);
			return(TRUE);

		case IDCANCEL:
			EndDialog(hDlg, TRUE);
			return(TRUE);
		}
	}
	return(FALSE);
}


/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SetCodeCheckMethod(int method)
{
	if(emustatus.Emu_Is_Running)
	{
		CheckMenuItem(gui.hMenu1964main, codecheckmenulist[emustatus.CodeCheckMethod - 1], MF_UNCHECKED);
		if(PauseEmulator())
		{
			Dyna_Check_Codes = Dyna_Code_Check[method - 1];
			emustatus.CodeCheckMethod = method;
			ResumeEmulator(REFRESH_DYNA_AFTER_PAUSE);	/* Need to init emu */
			CheckMenuItem(gui.hMenu1964main, codecheckmenulist[method - 1], MF_CHECKED);
		}
	}
	else
	{
		CheckMenuItem(gui.hMenu1964main, codecheckmenulist[emustatus.CodeCheckMethod - 1], MF_UNCHECKED);
		emustatus.CodeCheckMethod = method;
		defaultoptions.Code_Check = (CODECHECKTYPE)method;
		CheckMenuItem(gui.hMenu1964main, codecheckmenulist[method - 1], MF_CHECKED);
	}
}

WINDOWPLACEMENT window_placement_save_romlist;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void RememberWindowSize(void)
{
	/* Try to remember the main window position and size before playing a game */
	window_placement_save_romlist.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(gui.hwnd1964main, &window_placement_save_romlist);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void ResetWindowSizeAsRemembered(void)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	RECT	*prect = &(window_placement_save_romlist.rcNormalPosition);
	RECT    Rect;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	GetWindowRect(gui.hwnd1964main, &Rect);
	SetWindowPos
	(
		gui.hwnd1964main,
		NULL,
		Rect.left,
		Rect.top,
		prect->right - prect->left ,
		prect->bottom - prect->top ,
		SWP_NOZORDER | SWP_SHOWWINDOW
	);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void PrepareBeforePlay(int IsFullScreen)
{
	int i;
	if (IsFullScreen == 0 && !emustatus.Emu_Is_Resetting )	RememberWindowSize();

	REGISTRY_WriteDWORD("1964RunningStatus", TRUE);
	REGISTRY_WriteDWORD("ScreenSaverStatus", WindowScreenSaverStatus);

	// Disable screen saver
	SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, 0, 0); 

	ResetEvent(StopEmulatorEvent);
	ResetEvent(AudioThreadEvent);
	ResetEvent(AudioThreadStopEvent);
	ResetEvent(ResumeEmulatorEvent);

	if (IsFullScreen == 0)
	{
		/* Hide romlist */
		RomListSaveCurrentPos();
		ListView_SetExtendedListViewStyle(gui.hwndRomList, LVS_EX_FULLROWSELECT);
		NewRomList_ListViewHideHeader(gui.hwndRomList);
		ShowWindow(gui.hwndRomList, SW_HIDE);
		EnableWindow(gui.hwndRomList, FALSE);
	}

	EnableMenuItem(gui.hMenu1964main, ID_CPU_KILL, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, ID_ROM_STOP, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, ID_ROM_PAUSE, MF_ENABLED);

	EnableMenuItem(gui.hMenu1964main, ID_SAVESTATE, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, ID_LOADSTATE, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, ID_INTERPRETER, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, ID_DYNAMICCOMPILER, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, IDM_FULLSCREEN, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, IDM_PLUGINS, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_CHANGEDIRECTORY, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_FILE_FRESHROMLIST, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_VARIABLESPEEDLIMITS_INCREASESPEED, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, ID_VARIABLESPEEDLIMITS_DECREASESPEED, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, ID_VARIABLESPEEDLIMITS_RESTORECORRECTSPEED, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, ID_CHEATS_APPLY, MF_ENABLED);

	for( i=0; i<8; i++ )
	{
		EnableMenuItem(gui.hMenu1964main, recent_game_menus[i].id, MF_GRAYED);
		EnableMenuItem(gui.hMenu1964main, recent_rom_directory_menus[i].id,  MF_GRAYED);
	}

	if(GfxPluginVersion == 0x0103)
	{
		EnableMenuItem(gui.hMenu1964main, ID_PLUGINS_SCREENSHOTS, MF_ENABLED);
	}
	else
	{
		EnableMenuItem(gui.hMenu1964main, ID_PLUGINS_SCREENSHOTS, MF_GRAYED);
	}

	/* Setting options */
	RomListSelectLoadedRomEntry();
	GenerateCurrentRomOptions();
	init_whole_mem_func_array();					/* Needed here. The tlb function pointers change. */
	ResetRdramSize(currentromoptions.RDRAM_Size);

	if(strcpy(current_cheatcode_rom_internal_name, currentromoptions.Game_Name) != 0)
		CodeList_ReadCode(currentromoptions.Game_Name,cheatfilename);

	emustatus.CodeCheckMethod = currentromoptions.Code_Check;

	/*
	 * Using the Check_QWORD to boot, will switch to ROM specified
	 * emustatus.CodeCheckMethod ?
	 * at first FPU exception. I don't know why use NoCheck method will not boot ?
	 * Game like SuperMario should not need to do DynaCodeCheck but how the ROM does
	 * not boot ?
	 * with DynaCodeCheck, need debug
	 */
	if(emustatus.CodeCheckMethod == CODE_CHECK_NONE || emustatus.CodeCheckMethod == CODE_CHECK_DMA_ONLY)
	{
		Dyna_Check_Codes = Dyna_Code_Check_None_Boot;
		TRACE0("Set code check method = Dyna_Code_Check_None_Boot / Check_DMA_only");
	}
	else
	{
		Dyna_Check_Codes = Dyna_Code_Check[emustatus.CodeCheckMethod - 1];
	}

	emustatus.cpucore = currentromoptions.Emulator;
	SendMessage
	(
		gui.hwnd1964main,
		WM_COMMAND,
		emustatus.cpucore == DYNACOMPILER ? ID_DYNAMICCOMPILER : ID_INTERPRETER,
		0
	);

	/* About FPU usage exceptions */
	if((gHWS_COP0Reg[STATUS] & SR_CU1) || currentromoptions.FPU_Hack == USEFPUHACK_NO)
	{
		DisableFPUUnusableException();
	}
	else
	{
        EnableFPUUnusableException();		
	}

	Flashram_Init();
	Init_iPIF();

	emustatus.DListCount = 0;
	emustatus.AListCount = 0;
	emustatus.PIDMACount = 0;
	emustatus.viframeskip = 0;
	emustatus.ControllerReadCount = 0;
	emustatus.FrameBufferProtectionNeedToBeUpdate = TRUE;

	if(!QueryPerformanceFrequency(&Freq))
	{
		currentromoptions.Max_FPS = MAXFPS_NONE;	/* ok, this computer does not support */
		/* accurate timer, don't use speed limiter */
	}

	if(rominfo.TV_System == 0)					/* PAL */
	{
		vips_speed_limits[MAXFPS_AUTO_SYNC] = vips_speed_limits[MAXFPS_PAL_50];
	}
	else	/* NTSC */
	{
		vips_speed_limits[MAXFPS_AUTO_SYNC] = vips_speed_limits[MAXFPS_NTSC_60];
	}

	if (IsFullScreen == 0)
	{
		if( emustatus.VideoPluginSupportingFrameBuffer )
		{
			InitFrameBufferProtection();
		}
		SetStatusBarText(2, generalmessage);
		CheckMenuItem(gui.hMenu1964main, codecheckmenulist[emustatus.CodeCheckMethod - 1], MF_UNCHECKED);
		CheckMenuItem(gui.hMenu1964main, codecheckmenulist[emustatus.CodeCheckMethod - 1], MF_CHECKED);
		SetStatusBarText(4, emustatus.cpucore == DYNACOMPILER ? "D" : "I");
	}
    
    //This is CRITICAL to make the rom list lose focus.
    //Rom list in focus during game play causes problems.
    SetFocus(gui.hwnd1964main);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AfterStop(void)
{
	int i;
#ifdef CHEATCODE_LOCK_MEMORY
	CloseCheatCodeEngineMemoryLock();
#endif
  
	Close_Save();
	Close_iPIF();

	if( !emustatus.Emu_Is_Resetting )	ResetWindowSizeAsRemembered();

	emustatus.Emu_Is_Running = FALSE;
	EnableMenuItem(gui.hMenu1964main, ID_OPENROM, MF_ENABLED);
	EnableButton(ID_BUTTON_OPEN_ROM, TRUE);
	EnableMenuItem(gui.hMenu1964main, IDM_PLUGINS, MF_ENABLED);

	EnableButton(ID_BUTTON_SETUP_PLUGINS, TRUE);
	EnableMenuItem(gui.hMenu1964main, ID_CHANGEDIRECTORY, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, ID_FILE_FRESHROMLIST, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, ID_CLOSEROM, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, ID_ROM_START, MF_ENABLED);
	EnableMenuItem(gui.hMenu1964main, ID_ROM_PAUSE, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_VARIABLESPEEDLIMITS_INCREASESPEED, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_VARIABLESPEEDLIMITS_DECREASESPEED, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_VARIABLESPEEDLIMITS_RESTORECORRECTSPEED, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, IDM_FULLSCREEN, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_CHEATS_APPLY, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_INTERPRETER, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_DYNAMICCOMPILER, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_SAVESTATE, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_LOADSTATE, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_CPU_KILL, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_ROM_STOP, MF_GRAYED);
	EnableMenuItem(gui.hMenu1964main, ID_ROM_PAUSE, MF_GRAYED);

	for( i=0; i<16; i++ )
	{
		EnableMenuItem(gui.hMenu1964main, recent_game_menus[i].id, MF_ENABLED);
		EnableMenuItem(gui.hMenu1964main, recent_rom_directory_menus[i].id,  MF_ENABLED);
	}

	EnableRadioButtons(FALSE);

	/* EnableMenuItem(gui.hMenu1964main, ID_ROM_STOP, MF_GRAYED); */
	EnableMenuItem(gui.hMenu1964main, ID_PLUGINS_SCREENSHOTS, MF_GRAYED);
	DisableStateMenu();

	ShowWindow(gui.hwndRomList, SW_SHOW);

	EnableWindow(gui.hwndRomList, TRUE);
	ListView_SetExtendedListViewStyle(gui.hwndRomList, LVS_EX_FULLROWSELECT);	/* | LVS_EX_TRACKSELECT ); */
	NewRomList_ListViewShowHeader(gui.hwndRomList);

	/* refresh the rom list, just to prevent user has changed resolution */
	NewRomList_ListViewChangeWindowRect();
	InitStatusBarParts();
	RomListUseSavedPos();

	/* Reset some of the default options */
	defaultoptions.Emulator = DYNACOMPILER;
	emustatus.cpucore = defaultoptions.Emulator;
	SendMessage
	(
		gui.hwnd1964main,
		WM_COMMAND,
		emustatus.cpucore == DYNACOMPILER ? ID_DYNAMICCOMPILER : ID_INTERPRETER,
		0
	);
	SetStatusBarText(4, emustatus.cpucore == DYNACOMPILER ? "D" : "I");
	SetCodeCheckMethod(defaultoptions.Code_Check);

	/* Flash the status bar */
	ShowWindow(gui.hStatusBar, SW_HIDE);
	ShowWindow(gui.hStatusBar, SW_SHOW);
	SetStatusBarText(3, defaultoptions.RDRAM_Size == RDRAMSIZE_4MB ? "4MB" : "8MB");

	sprintf(generalmessage, "%s - %s", gui.szWindowTitle, TranslateStringByString("Stopped"));
	
	Set_Ready_Message();
	SetStatusBarText(1, " 0 VI/s");

	if( NeedFreshromListAfterStop == TRUE )
	{
		NeedFreshromListAfterStop = FALSE;
		OnFreshRomList(TRUE);
	}

    //Set focus to RomList
    SetFocus(gui.hwndRomList);
	if(rlstatus.romlist_count > 0) 
		RomListSelectRom(rlstatus.selected_rom_index);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void InitStatusBarParts(void)
{
	/*~~~~~~~*/
	RECT	rc;
	/*~~~~~~~*/

	if(gui.hStatusBar == NULL)
		return;
	else
	{
		/*~~~~~~~~~*/
		int sizes[6];
		/*~~~~~~~~~*/

		GetWindowRect(gui.hStatusBar, &rc);

		/*
		 * sizes[5] = rc.right-rc.left-25; ?
		 * sizes[4] = sizes[5]-40;
		 */
		sizes[4] = rc.right - rc.left - 25;
		sizes[3] = sizes[4] - 15;
		sizes[2] = sizes[3] - 30;
		sizes[1] = sizes[2] - 75; //CF
		sizes[0] = sizes[1] - 80;

		SendMessage(gui.hStatusBar, SB_SETPARTS, 5, (LPARAM) sizes);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void __cdecl SetStatusBarText(int fieldno, char *text)
{
	if( guistatus.IsFullScreen )
	{
		if( fieldno == 0 )
		{
			VIDEO_SetOnScreenText(text);
		}
	}
	else
	{
		SendMessage(gui.hStatusBar, SB_SETTEXT, fieldno, (LPARAM) text);
	}
}

HMENU			file_submenu;
HMENU			CPU_submenu;
HMENU			recent_rom_directory_submenu;
HMENU			recent_game_submenu;
HMENU			switch_compiler_submenu;
MENUITEMINFO	switch_compiler_submenu_info;
HMENU			dyna_code_checking_submenu;
HMENU			counter_hack_submenu;
HMENU			state_save_submenu;
HMENU			state_load_submenu;
UINT			recent_rom_directory_submenu_pos;
UINT			recent_game_submenu_pos;
UINT			switch_compiler_submenu_pos;
UINT			dyna_code_checking_submenu_pos;
UINT			counter_hack_submenu_pos;
UINT			state_save_submenu_pos;
UINT			state_load_submenu_pos;

MENUITEMINFO	seperator_menuitem;
UINT			seperator_menuitem_pos;


/*
 =======================================================================================================================
 =======================================================================================================================
 */

void ModifyMenuText(UINT menuid, char *newtext)
{
	ModifyMenu(gui.hMenu1964main, menuid, MF_BYCOMMAND, menuid, newtext);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void RegenerateRecentGameMenus(void)
{
	LoadString(gui.hInst, IDS_RECENT_ROMS, (LPSTR)Scratch1, 700);
	if(guioptions.show_recent_rom_directory_list)
	{
		InsertMenu
		(
			file_submenu,
			recent_game_submenu_pos,
			MF_BYPOSITION | MF_POPUP,
			(UINT) recent_game_submenu,
			(LPCSTR)Scratch1
		);
	}
	else
	{
		InsertMenu
		(
			file_submenu,
			recent_rom_directory_submenu_pos,
			MF_BYPOSITION | MF_POPUP,
			(UINT) recent_game_submenu,
			(LPCSTR)Scratch1
		);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void DeleteRecentGameMenus(void)
{
	/*~~~~~~~~~~~~~~~*/
	int		k, i, j, n;
	char	str[100];
	/*~~~~~~~~~~~~~~~*/

	i = GetMenuItemCount(gui.hMenu1964main);
	for(k = 0; k < i; k++)
	{
		GetMenuString(gui.hMenu1964main, k, str, 80, MF_BYPOSITION);
		if(strcmp(str, "&File") == 0)
		{
			file_submenu = GetSubMenu(gui.hMenu1964main, k);
			j = GetMenuItemCount(file_submenu);
			for(n = j - 1; n >= 0; n--) /* I have to delete the menu in reverse order */
			{
				GetMenuString(file_submenu, n, str, 80, MF_BYPOSITION);
				LoadString(gui.hInst, IDS_RECENT_ROMS, (LPSTR)Scratch1, 700);
				if(strcmp(str, (LPSTR)Scratch1) == 0)
				{
					recent_game_submenu = GetSubMenu(file_submenu, n);
					recent_game_submenu_pos = n;
					RemoveMenu(file_submenu, n, MF_BYPOSITION);
				}
			}
		}
	}
}

void __cdecl RefreshRecentGameMenus(char *newgamefilename)
{
	int i;
	char str[256];

    __try
    {
		for(i = 0; i < MAX_RECENT_GAME_LIST; i++)
		{
			if(strcmp(recent_game_lists[i], newgamefilename) == 0 || strcmp(recent_game_lists[i], "Empty Game Slot" ) == 0 ) 
				break;
		}

		if(i == MAX_RECENT_GAME_LIST)
		{
			i = MAX_RECENT_GAME_LIST-1;	/* if not found */
		}

		if( recent_game_menus[i].visible == FALSE && i< 8 )
		{
			HMENU file_submenu = GetSubMenu(gui.hMenu1964main, 0);
			HMENU recentgame_submenu = GetSubMenu(file_submenu, 11);
			AppendMenu
				(
				recentgame_submenu,
				MF_ENABLED|MF_STRING,
				recent_game_menus[i].id,
				"Empty Game Slot"
				);
			recent_game_menus[i].visible = TRUE;
		}

		/* need to move the most recent file to the 1st position */
		for(; i > 0; i--)
		{
			strcpy(recent_game_lists[i], recent_game_lists[i - 1]);
			ModifyMenuText(recent_game_menus[i].id, recent_game_lists[i]);
		}

		strcpy(recent_game_lists[0], newgamefilename);
		ModifyMenuText(recent_game_menus[0].id, newgamefilename);

		// Save the list into registry
		for(i = 0; i < MAX_RECENT_GAME_LIST; i++) 
		{
			sprintf(str, "RecentGame%d", i);
			REGISTRY_WriteStringByName( str, recent_game_lists[i]);
		}
    }
    __except(NULL, EXCEPTION_EXECUTE_HANDLER)
    {
    }

	return;
}

/*
 =======================================================================================================================
    char recent_rom_directory_lists[MAX_RECENT_ROM_DIR][260];
 =======================================================================================================================
 */
void RefreshRecentRomDirectoryMenus(char *newromdirectory)
{
	int i;
	char str[256];

	for(i = 0; i < MAX_RECENT_ROM_DIR; i++)
	{
		if(strcmp(recent_rom_directory_lists[i], newromdirectory) == 0 || strcmp(recent_rom_directory_lists[i], "Empty Rom Folder Slot" ) == 0 ) 
			break;
	}

	if(i == MAX_RECENT_ROM_DIR) 
		i = MAX_RECENT_ROM_DIR - 1; /* if not found */

	if( recent_rom_directory_menus[i].visible == FALSE && i< 8)
	{
		HMENU file_submenu = GetSubMenu(gui.hMenu1964main, 0);
		HMENU recentfolder_submenu = GetSubMenu(file_submenu, 10);
		AppendMenu
			(
			recentfolder_submenu,
			MF_ENABLED|MF_STRING,
			recent_rom_directory_menus[i].id,
			"Empty Rom Folder Slot"
			);
		recent_rom_directory_menus[i].visible = TRUE;
	}

	/* need to move the most recent file to the 1st position */
	for(; i > 0; i--)
	{
		strcpy(recent_rom_directory_lists[i], recent_rom_directory_lists[i - 1]);
		ModifyMenuText(recent_rom_directory_menus[i].id, recent_rom_directory_lists[i]);
	}

	strcpy(recent_rom_directory_lists[0], newromdirectory);
	ModifyMenuText(recent_rom_directory_menus[0].id, newromdirectory);

	for(i = 0; i < MAX_RECENT_ROM_DIR; i++)	
	{
		sprintf(str, "RecentRomDirectory%d", i);
		REGISTRY_WriteStringByName( str, recent_rom_directory_lists[i]);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void RegerateRecentRomDirectoryMenus(void)
{
	InsertMenu
	(
		file_submenu,
		recent_rom_directory_submenu_pos,
		MF_BYPOSITION | MF_POPUP,
		(UINT) recent_rom_directory_submenu,
		"Recent ROM Folders"
	);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void DeleteRecentRomDirectoryMenus(void)
{
	int		i, j, n;
	char	str[100];

	i = GetMenuItemCount(gui.hMenu1964main);
	file_submenu = GetSubMenu(gui.hMenu1964main, 0);
	j = GetMenuItemCount(file_submenu);
	for(n = j - 1; n >= 0; n--) /* I have to delete the menu in reverse order */
	{
		GetMenuString(file_submenu, n, str, 80, MF_BYPOSITION);
		if(strcmp(str, "Recent ROM Folders") == 0)
		{
			recent_rom_directory_submenu = GetSubMenu(file_submenu, n);
			recent_rom_directory_submenu_pos = n;
			RemoveMenu(file_submenu, n, MF_BYPOSITION);
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void RegenerateStateSelectorMenus(void)
{
	{
		InsertMenu
		(
			CPU_submenu,
			state_save_submenu_pos,
			MF_BYPOSITION | MF_POPUP,
			(UINT) state_save_submenu,
			"Save State\tF5"
		);
		InsertMenu
		(
			CPU_submenu,
			state_save_submenu_pos,
			MF_BYPOSITION | MF_POPUP,
			(UINT) state_load_submenu,
			"Load State\tF7"
		);
	}
}

int Separator = 0;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SetupAdvancedMenus(void)
{
	/*~~*/
	int i;
	/*~~*/

	for(i = 0; i < MAX_RECENT_ROM_DIR; i++)
	{
		ModifyMenuText(recent_rom_directory_menus[i].id, recent_rom_directory_lists[i]);
		if(strcmp(recent_rom_directory_lists[i], "Empty Rom Folder Slot" ) == 0)
		{
			EnableMenuItem(gui.hMenu1964main, recent_rom_directory_menus[i].id,MF_GRAYED);
			DeleteMenu(gui.hMenu1964main, recent_rom_directory_menus[i].id, MF_BYCOMMAND);
			recent_rom_directory_menus[i].visible=FALSE;
		}
		else
		{
			EnableMenuItem(gui.hMenu1964main, recent_rom_directory_menus[i].id,MF_ENABLED);
			if( i >= 8 )
			{
				DeleteMenu(gui.hMenu1964main, recent_rom_directory_menus[i].id, MF_BYCOMMAND);
				recent_rom_directory_menus[i].visible=FALSE;
			}
		}
	}

	for(i = 0; i < MAX_RECENT_GAME_LIST; i++)
	{
		ModifyMenuText(recent_game_menus[i].id, recent_game_lists[i]);
		if(strcmp(recent_game_lists[i], "Empty Game Slot" ) == 0)
		{
			EnableMenuItem(gui.hMenu1964main, recent_game_menus[i].id,MF_GRAYED);
			DeleteMenu(gui.hMenu1964main, recent_game_menus[i].id, MF_BYCOMMAND);
			recent_game_menus[i].visible=FALSE;
		}
		else
		{
			EnableMenuItem(gui.hMenu1964main, recent_game_menus[i].id,MF_ENABLED);
			if( i >= 8 )
			{
				DeleteMenu(gui.hMenu1964main, recent_game_menus[i].id, MF_BYCOMMAND);
				recent_game_menus[i].visible=FALSE;
			}
		}
	}

	if(guioptions.show_recent_rom_directory_list == FALSE) DeleteRecentRomDirectoryMenus();
	if(guioptions.show_recent_game_list == FALSE) DeleteRecentGameMenus();
	if(!emuoptions.SyncVI) CheckMenuItem(gui.hMenu1964main, ID_CPU_AUDIOSYNC, MF_UNCHECKED);
	if(!emuoptions.AutoFrameSkip) CheckMenuItem(gui.hMenu1964main, ID_EMULATION_AUTOFRAMESKIP, MF_UNCHECKED);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CaptureScreenToFile(void)
{
	if(emustatus.Emu_Is_Running)
	{
		/*~~~~~~~~~~~~~~~~~~~~~~~~~*/
		char	directory[_MAX_PATH];
		/*~~~~~~~~~~~~~~~~~~~~~~~~~*/

		strcpy(directory, directories.main_directory);
		strcat(directory, "Screens\\");
		VIDEO_CaptureScreen(directory);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Exit1964(void)
{

	SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, WindowScreenSaverStatus, 0, 0); 

    __try {    
		WINDOWPLACEMENT placement;
		static BOOL exiting_1964 = FALSE;


		if (exiting_1964) return;
		exiting_1964 = TRUE;

		SetStatusBarText(0, "Exiting 1964");
		KillTimer(gui.hwnd1964main, timer);
		if(emustatus.Emu_Is_Running) 
			Stop();

		placement.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(gui.hwnd1964main, &placement);
		guistatus.WindowIsMaximized = (placement.showCmd == SW_SHOWMAXIMIZED);


		RomListRememberColumnWidth();
		RomListSaveCurrentPosToRegistry();

		Close_iPIF();			/* save mempak and eeprom */
		FreeVirtualMemory();

		//FileIO_Write1964Ini();	/* Save ROM_Properties.ini */

		DeleteAllIniEntries();	/* Release all ini entries */
		ClearRomList();			/* Clean the Rom List */
		FreePlugins();

		WriteConfiguration();

		if (StopEmulatorEvent)			CloseHandle( StopEmulatorEvent );
		if (ResumeEmulatorEvent)		CloseHandle( ResumeEmulatorEvent);
		if (AudioThreadStopEvent)		CloseHandle( AudioThreadStopEvent);
		if (AudioThreadEvent)			CloseHandle( AudioThreadEvent);

		if( AllPluginDLLInfos )  
		{
			VirtualFree(AllPluginDLLInfos, 0, MEM_RELEASE);
			AllPluginDLLInfos = NULL;
		}

		{
			int i;
			for(i=0; i<guioptions.totalLanguages; i++)
			{
				if( languageFileNames[i] )
				{
					VirtualFree(languageFileNames[i],0,MEM_RELEASE);
					languageFileNames[i]=NULL;
				}
			}
		}

		FreeDlgLangMapEntries();

		if( rlstatus.hLargeBitmaps == NULL )
		{
			ImageList_Destroy(rlstatus.hLargeBitmaps);
			rlstatus.hLargeBitmaps = NULL;
		}
		if( rlstatus.hSmallIcons == NULL )
		{
			ImageList_Destroy(rlstatus.hSmallIcons);
			rlstatus.hSmallIcons = NULL;
		}


		exit(0);
	}
    __except(NULL, EXCEPTION_EXECUTE_HANDLER)
    {
        exit(0);
    }
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Set_Ready_Message(void)
{
	if(Rom_Loaded)
	{
		if(strlen(currentromoptions.Alt_Title) < 1 || strcmp((const char*)rominfo.name, currentromoptions.Game_Name) != 0)
			sprintf((char*)generalmessage, "%s - %s - [%s]", TranslateStringByString("Ready"), directories.rom_directory_to_use, rominfo.name);
		else
		{
			sprintf(generalmessage, "%s - %s - [%s]", TranslateStringByString("Ready"),directories.rom_directory_to_use, currentromoptions.Alt_Title);
		}
	}
	else
	{
		sprintf(generalmessage, "%s - %s", TranslateStringByString("Ready"), directories.rom_directory_to_use);
	}

	SetStatusBarText(0, generalmessage);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void DisableDebugMenu(void)
{
	/*~~~~~~~~~~~~*/
	/* Disable the DEBUG menu */
	int		i, k;
	char	str[80];
	/*~~~~~~~~~~~~*/

	i = GetMenuItemCount(gui.hMenu1964main);
	for(k = 0; k < i; k++)
	{
		GetMenuString(gui.hMenu1964main, k, str, 80, MF_BYPOSITION);
		if(strcmp(str, "Debug") == 0)
		{
			DeleteMenu(gui.hMenu1964main, k, MF_BYPOSITION);
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SetupDebuger(void)
{
#ifdef _DEBUG
	CheckMenuItem(gui.hMenu1964main, ID_DEBUG_CONTROLLER, debugoptions.debug_si_controller ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(gui.hMenu1964main, ID_DEBUGSPTASK, debugoptions.debug_sp_task ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(gui.hMenu1964main, ID_DEBUGSITASK, debugoptions.debug_si_task ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(gui.hMenu1964main, ID_DEBUGSPDMA, debugoptions.debug_sp_dma ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(gui.hMenu1964main, ID_DEBUGSIDMA, debugoptions.debug_si_dma ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(gui.hMenu1964main, ID_DEBUGPIDMA, debugoptions.debug_pi_dma ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(gui.hMenu1964main, ID_DEBUGMEMPAK, debugoptions.debug_si_mempak ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(gui.hMenu1964main, ID_DEBUGTLB, debugoptions.debug_tlb ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(gui.hMenu1964main, ID_DEBUGEEPROM, debugoptions.debug_si_eeprom ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(gui.hMenu1964main, ID_DEBUG_SRAM, debugoptions.debug_sram ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(gui.hMenu1964main, ID_DEBUG_EXCEPTION_SERVICES, debugoptions.debug_exception_services ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(gui.hMenu1964main, ID_DEBUGFRAMEBUFFERREAD, debugoptions.debug_framebuffer_rw ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(gui.hMenu1964main, ID_DEBUGDYNA, debugoptions.debug_dyna_compiler ? MF_CHECKED : MF_UNCHECKED);

	DebuggerBreakPointActive = FALSE;
	OpCount = 0;
	NextClearCode = 250;
	BreakAddress = -1;
	DebuggerActive = FALSE;
	OpenDebugger();
#else
	DisableDebugMenu();
#endif
}

long OnNotifyStatusBar(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if( ((LPNMHDR) lParam)->code == NM_DBLCLK )
	{
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		int fieldno = ((LPNMLISTVIEW) lParam)->iItem;
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		
		switch(fieldno)
		{
		case 4:						/* CPU core */
			/* Switch CPU core */
			if(emustatus.Emu_Is_Running)
			{
				SendMessage
					(
					gui.hwnd1964main,
					WM_COMMAND,
					emustatus.cpucore == DYNACOMPILER ? ID_INTERPRETER : ID_DYNAMICCOMPILER,
					0
					);
			}
			else
			{
				SendMessage
					(
					gui.hwnd1964main,
					WM_COMMAND,
					defaultoptions.Emulator == DYNACOMPILER ? ID_INTERPRETER : ID_DYNAMICCOMPILER,
					0
					);
			}
			break;
		}
	}

	return 0l;
}


extern BOOL WINAPI InitListViewItems(HWND hwndLV, int Sorting);
extern void RemoveImageListMap(int index);
long OnPopupMenuCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
	case ID_POPUP_LOADPLAY:
		if( Rom_Loaded )
		{
			Play(emuoptions.auto_full_screen);
		}
		else
		{
			RomListOpenRom(RomListGetSelectedIndex(), TRUE);
		}
		break;
	case ID_POPUP_LOADPLAYINFULLSCREEN:
		RomListOpenRom(RomListGetSelectedIndex(), FALSE);
		Play(TRUE);
		break;
	case ID_POPUP_LOADPLAYINWINDOWMODE:
		RomListOpenRom(RomListGetSelectedIndex(), FALSE);
		Play(FALSE);
		break;
	case ID_POPUP_ROM_SETTING:
		RomListRomOptions(rlstatus.selected_rom_index);
		break;
	case ID_POPUP_CHEATCODE:
		if(emustatus.Emu_Is_Running)
		{
			PauseEmulator();
			CreateOptionsDialog(PSH_CHEAT_CODES);
			ResumeEmulator(DO_NOTHING_AFTER_PAUSE);
		}
		else
			CreateOptionsDialog(PSH_CHEAT_CODES);
		break;
	case ID_HEADERPOPUP_SHOW_INTERNAL_NAME:
		rlstatus.romlistNameToDisplay = ROMLIST_DISPLAY_INTERNAL_NAME;
		SendMessage(gui.hwnd1964main, WM_COMMAND, ID_FILE_FRESHROMLIST, 0);
		break;
	case ID_HEADERPOPUP_SHOWALTERNATEROMNAME:
		rlstatus.romlistNameToDisplay = ROMLIST_DISPLAY_ALTER_NAME;
		SendMessage(gui.hwnd1964main, WM_COMMAND, ID_FILE_FRESHROMLIST, 0);
		break;
	case ID_HEADERPOPUP_SHOWROMFILENAME:
		rlstatus.romlistNameToDisplay = ROMLIST_DISPLAY_FILENAME;
		SendMessage(gui.hwnd1964main, WM_COMMAND, ID_FILE_FRESHROMLIST, 0);
		break;
	case ID_HEADERPOPUP_1_SORT_ASCENDING:
		//set sort method by column and set ascending.
         rlstatus.romlist_sort_method = rlstatus.romListHeaderClickedColumn;
		NewRomList_Sort();
        InitListViewItems(gui.hwndRomList, 1);
		break;
	case ID_HEADERPOPUP_1_SORT_DESCENDING:
        rlstatus.romlist_sort_method = rlstatus.romListHeaderClickedColumn;
        rlstatus.romlist_sort_method += NUM_SORT_METHODS;
		NewRomList_Sort();
        InitListViewItems(gui.hwndRomList, 1);
		break;
	case ID_HEADERPOPUP_2_SORT_ASCENDING:
		rlstatus.romlist_sort_method = rlstatus.romListHeaderClickedColumn;
		NewRomList_Sort();
        InitListViewItems(gui.hwndRomList, 1);
		break;
	case ID_HEADERPOPUP_2_SORT_DESCENDING:
        rlstatus.romlist_sort_method = rlstatus.romListHeaderClickedColumn+NUM_SORT_METHODS;
		NewRomList_Sort();
        InitListViewItems(gui.hwndRomList, 1);
		break;
	case ID_HEADERPOPUP_1_SELECTING:
	case ID_HEADERPOPUP_2_SELECTING:
		CreateOptionsDialog(PSH_ROM_BROWSER);
		break;
	case ID_POPUPMENU_PLUGINSETTING:
		guioptions.displayDefaultPlugins = FALSE;
		CreateOptionsDialog(PSH_PLUGINS);
		break;
	case ID_POPUPMENU_ROMINFORMATION:
		CreateOptionsDialog(PSH_ROM_INFORMATION);
		break;
	case ID_HEADERPOPUP3_SHOWROMLIST:
		if( guioptions.display_boxart )
		{
			guioptions.display_boxart = FALSE;
			REGISTRY_WriteDWORD( "DisplayBoxart", guioptions.display_boxart);
			OnFreshRomList(TRUE);
		}
		break;
	case ID_HEADERPOPUP3_SHOWBOXART:
		if( !guioptions.display_boxart )
		{
			guioptions.display_boxart = TRUE;
			REGISTRY_WriteDWORD( "DisplayBoxart", guioptions.display_boxart);
			OnFreshRomList(TRUE);
		}
		break;
	case ID_BOXARTIMAGE_ZOOM_IN:
		if( guioptions.boxart_image_width < 1024 && guioptions.boxart_image_height < 720 )
		{
			guioptions.boxart_image_width = min(guioptions.boxart_image_width*2, 1024);
			guioptions.boxart_image_height = min(guioptions.boxart_image_height*2, 720);
			REGISTRY_WriteDWORD( "BoxartImageWidth", guioptions.boxart_image_width);
			REGISTRY_WriteDWORD( "BoxartImageHeight", guioptions.boxart_image_height);
			OnFreshRomList(TRUE);
		}
		break;
	case ID_BOXARTIMAGE_ZOOM_OUT:
		if( guioptions.boxart_image_width > 16 && guioptions.boxart_image_height > 12 )
		{
			guioptions.boxart_image_width = max(guioptions.boxart_image_width/2, 16);
			guioptions.boxart_image_height = max(guioptions.boxart_image_height/2, 12);
			REGISTRY_WriteDWORD( "BoxartImageWidth", guioptions.boxart_image_width);
			REGISTRY_WriteDWORD( "BoxartImageHeight", guioptions.boxart_image_height);
			OnFreshRomList(TRUE);
		}
		break;
	case ID_BOXARTIMAGE_DECREASEXSPACING:
		if( guioptions.boxart_image_x_spacing > 4 )
		{
			guioptions.boxart_image_x_spacing = max(guioptions.boxart_image_x_spacing/2, 4);
			REGISTRY_WriteDWORD( "BoxartImageXSpacing", guioptions.boxart_image_x_spacing);
			OnFreshRomList(TRUE);
		}
		break;
	case ID_BOXARTIMAGE_INCREASEXSPACING:
		if( guioptions.boxart_image_x_spacing < 300 )
		{
			guioptions.boxart_image_x_spacing = min(guioptions.boxart_image_x_spacing*2, 300);
			REGISTRY_WriteDWORD( "BoxartImageXSpacing", guioptions.boxart_image_x_spacing);
			OnFreshRomList(TRUE);
		}
		break;
	case ID_BOXARTIMAGE_DECREASEYSPACING:
		if( guioptions.boxart_image_y_spacing > 4 )
		{
			guioptions.boxart_image_y_spacing = max(guioptions.boxart_image_y_spacing/2, 4);
			REGISTRY_WriteDWORD( "BoxartImageYSpacing", guioptions.boxart_image_y_spacing);
			OnFreshRomList(TRUE);
		}
		break;
	case ID_BOXARTIMAGE_INCREASEYSPACING:
		if( guioptions.boxart_image_y_spacing < 300 )
		{
			guioptions.boxart_image_y_spacing = min(guioptions.boxart_image_y_spacing*2, 300);
			REGISTRY_WriteDWORD( "BoxartImageYSpacing", guioptions.boxart_image_y_spacing);
			OnFreshRomList(TRUE);
		}
		break;
	case ID_POPUPMENU_ASSIGNBOXARTIMAGE:
		if( LinkBoxArtImageByDialog() )
		{
			RemoveImageListMap(rlstatus.selected_rom_index);
			OnFreshRomList(TRUE);
		}
		break;
	case ID_POPUPMENU_REMOVEBOXARTIMAGELINK:
		romlist[rlstatus.selected_rom_index]->pinientry->iconFilename[0] = 0;
		FileIO_Write1964Ini();
		RemoveImageListMap(rlstatus.selected_rom_index);
		OnFreshRomList(TRUE);
		break;
	case ID_HEADERPOPUP3_REFRESH:
		OnFreshRomList(TRUE);
		break;
	}

	return 0l;

}

long OnOpcodeDebuggerCommands(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return 0l;
}

void OnFreshRomList(BOOL reload)
{
	if( !emustatus.Emu_Is_Running )
	{
		RomListSaveCurrentPosToRegistry();
		NewRomList_ListViewChangeWindowRect();
		InitStatusBarParts();
		if( reload )
		{
			ClearRomList();
			NewRomList_ListViewFreshRomList();
			SetStatusBarText(0, TranslateStringByString("Looking for ROM files in the ROM directory and Generating List"));
			RomListReadDirectory(directories.rom_directory_to_use,FALSE);
		}
		NewRomList_ListViewFreshRomList();
		RomListLoadCurrentPosFromRegistry();

		Set_Ready_Message();
	}
}

void IncreaseSpeedLimit()
{
	if( emustatus.Emu_Is_Running )
	{
		switch( currentromoptions.Max_FPS )
		{
		case MAXFPS_NONE:
		case MAXFPS_AUTO_SYNC:
			currentromoptions.Max_FPS = MAXFPS_NTSC_60;
			break;
		case MAXFPS_PAL_50:
			currentromoptions.Max_FPS = MAXFPS_NTSC_60;
			break;
		case MAXFPS_NTSC_60:
			currentromoptions.Max_FPS = MAXFPS_70;
			break;
		case MAXFPS_40:
			currentromoptions.Max_FPS = MAXFPS_PAL_50;
		default:
			if( currentromoptions.Max_FPS < MAXFPS_210 )
			{
				currentromoptions.Max_FPS++;
			}
			break;
		}
	}
}

void DecreaseSpeedLimit()
{
	if( emustatus.Emu_Is_Running )
	{
		switch( currentromoptions.Max_FPS )
		{
		case MAXFPS_NONE:
		case MAXFPS_AUTO_SYNC:
			currentromoptions.Max_FPS = MAXFPS_PAL_50;
			break;
		case MAXFPS_PAL_50:
			currentromoptions.Max_FPS = MAXFPS_40;
			break;
		case MAXFPS_NTSC_60:
			currentromoptions.Max_FPS = MAXFPS_PAL_50;
			break;
		case MAXFPS_70:
			currentromoptions.Max_FPS = MAXFPS_NTSC_60;
		default:
			if( currentromoptions.Max_FPS > MAXFPS_10 )
			{
				currentromoptions.Max_FPS--;
			}
			break;
		}
	}
}

void ResetSpeedLimit()
{
	if( emustatus.Emu_Is_Running )
	{
		currentromoptions.Max_FPS = MAXFPS_AUTO_SYNC;
	}
}

