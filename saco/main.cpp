//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: main.cpp,v 1.60 2006/05/21 11:20:49 kyeman Exp $
//
//----------------------------------------------------------

#include "main.h"
#include "game/util.h"
#include "anticheat.h"
#include <aclapi.h>

extern CGame			*pGame;

CHAR					szArtworkProxy[MAX_PATH+1];
CHAR					szConfigFile[MAX_PATH+1];
CHAR					szChatLogFile[MAX_PATH+1];
CHAR					szWorkingPath[MAX_PATH+1];
CHAR					szCachePath[MAX_PATH+1];

int						iGtaVersion=0;

GAME_SETTINGS			tSettings;
CConfig					*pConfig=0;
CChatWindow				*pChatWindow=0;
CCmdWindow				*pCmdWindow=0;
CDeathWindow			*pDeathWindow=0;
CSpawnScreen			*pSpawnScreen=0;
CNetGame				*pNetGame=0;
CFontRender				*pDefaultFont=0;

BOOL					bGameInited=FALSE;
BOOL					bNetworkInited=FALSE;

BOOL					bGameModded = FALSE;
BOOL					bQuitGame=FALSE;
DWORD					dwStartQuitTick=0;

IDirect3D9				*pD3D;
IDirect3DDevice9		*pD3DDevice	= NULL;
//D3DMATRIX				matView;

HANDLE					hInstance=0;
CNewPlayerTags			*pNewPlayerTags=NULL;
CScoreBoard				*pScoreBoard=NULL;
CLabel					*pLabel=NULL;
CNetStats				*pNetStats=NULL;
CSvrNetStats			*pSvrNetStats=NULL;
CHelpDialog				*pHelpDialog=NULL;

bool					bShowDebugLabels = false;

CGame					*pGame=0;
DWORD					dwGameLoop=0;
DWORD					dwGraphicsLoop=0;
DWORD					dwUIMode=0;				// 0 = old mode, 1 = new MMOG mode, 2 = DXUT perhaps?
												// Have this settable from the server.. on Init.
CFileSystem				*pFileSystem=NULL;
CAntiCheat				*pAntiCheat = NULL;

CDXUTDialogResourceManager	*pDialogResourceManager=NULL;
CDXUTDialog					*pGameUI=NULL;

// forwards

BOOL SubclassGameWindow();
void SetupCommands();
//void TheGameLoop();
void TheGraphicsLoop();
void MarkAsModdedGame();
void GameDebugDrawDebugScreens();
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
LONG WINAPI exc_handler(_EXCEPTION_POINTERS* exc_inf);
void d3d9DestroyDeviceObjects();
void d3d9RestoreDeviceObjects();

UINT uiCounter=0;

DWORD dwOrgRwSetState=0;
DWORD dwSetStateCaller=0;
DWORD dwSetStateOption=0;
DWORD dwSetStateParam=0;
char dbgstr[256];

// backwards
//----------------------------------------------------

extern void InstallGameAndGraphicsLoopHooks();
extern void InitScripting();
extern void ApplyDebugLevelPatches();

// polls the game until it's able to run.
void LaunchMonitor(PVOID v)
{
	if(pGame) {
		pGame->InitGame();
	}

	while(1) {
		if(*(PDWORD)ADDR_ENTRY == 7) {
			pGame->StartGame();
            break;
		}
		else {
			Sleep(5);
		}
	}

	ExitThread(0);
}

//----------------------------------------------------

#define ARCHIVE_FILE	"samp.saa"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(DLL_PROCESS_ATTACH==fdwReason)
	{
		OutputDebugString("SA-MP Process Attached\n");

		hInstance = hinstDLL;
		InitSettings();

		if(tSettings.bDebug || tSettings.bPlayOnline)
		{
			SetProcessAffinityMask(GetCurrentProcess(),1);

			SetUnhandledExceptionFilter(exc_handler);
			//dwGameLoop = (DWORD)TheGameLoop;
			dwGraphicsLoop = (DWORD)TheGraphicsLoop;

			OutputDebugString("Loading Archive");

			CHAR szArchiveFile[MAX_PATH];
			GetModuleFileNameA((HMODULE)hInstance, szArchiveFile, MAX_PATH);
			DWORD dwFileNameLen = strlen(szArchiveFile);
			while(szArchiveFile[dwFileNameLen] != '\\')
				dwFileNameLen--;
			strcpy(szArchiveFile+dwFileNameLen+1, ARCHIVE_FILE);

			pFileSystem = new CArchiveFS();
			if(!pFileSystem->Load(szArchiveFile)) _asm int 3

			OutputDebugString("Installing filesystem hooks.");

			InstallFileSystemHooks();
			InstallShowCursorHook();

			pGame = new CGame();

			_beginthread(LaunchMonitor,0,NULL);	
			OutputDebugString("SA:MP Inited\n");			
		}
	}
	else if(DLL_PROCESS_DETACH==fdwReason)
	{
		if(tSettings.bDebug || tSettings.bPlayOnline) {
			OutputDebugString("Process Detached\n");
			/*if (pAntiCheat)	{
				pAntiCheat->Disable();
				delete pAntiCheat;
			}*/
			UninstallFileSystemHooks();
		}
	}

	return TRUE;
}

//----------------------------------------------------

PCHAR GetWorkingPath()
{
	return szWorkingPath;
}

//----------------------------------------------------

PCHAR GetCachePath()
{
	return szCachePath;
}

//----------------------------------------------------

void CreateCacheDirectory()
{
	memset(szCachePath,0,sizeof(szCachePath));

	CHAR szBuffer[MAX_PATH+1];
	memset(szBuffer,0,sizeof(szBuffer));

	DWORD dwBufLen = MAX_PATH;
	HKEY hKey;
	DWORD dwType;
	
	sprintf(szCachePath,"%s\\cache",szWorkingPath);

	if( !RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\SAMP", 0, KEY_READ, &hKey) &&
		!RegQueryValueEx(hKey, "model_cache", NULL, &dwType, (LPBYTE)szBuffer, &dwBufLen) )
	{
		strncpy(szCachePath,szBuffer,MAX_PATH+1);
	}

	if(!IsFileOrDirectoryExist(szCachePath))
		CreateDirectory(szCachePath,NULL);

	CHAR szLocalPath[MAX_PATH+1];
	sprintf(szLocalPath,"%s\\local",szCachePath);
	if(!IsFileOrDirectoryExist(szLocalPath))
		CreateDirectory(szLocalPath,NULL);
}

//----------------------------------------------------

PCHAR GetArtworkProxy()
{
	memset(szArtworkProxy,0,sizeof(szArtworkProxy));

	HKEY hKey;
	DWORD dwType;
	DWORD dwBufLen = MAX_PATH;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\SAMP", 0, KEY_READ, &hKey))
		return NULL;

	if (RegQueryValueEx(hKey, "artwork_proxy", NULL, &dwType, (LPBYTE)szArtworkProxy, &dwBufLen) == ERROR_SUCCESS)
		return szArtworkProxy;

	return NULL;
}

//----------------------------------------------------

void CreateWorkingDirectory()
{
	memset(szWorkingPath,0,sizeof(szWorkingPath));

	if(!strlen((char*)0xC92368))
	{
		GetCurrentDirectory(MAX_PATH+1,szWorkingPath);
	}
	else
	{
		sprintf(szWorkingPath,"%s\\SAMP",(char*)0xC92368);
		if(!IsFileOrDirectoryExist(szWorkingPath))
			CreateDirectory(szWorkingPath,NULL);

		CHAR szScreensPath[MAX_PATH+1];
		sprintf(szScreensPath,"%s\\screens",szWorkingPath);
		if(!IsFileOrDirectoryExist(szScreensPath))
			CreateDirectory(szScreensPath,NULL);

		CreateCacheDirectory();
	}
}

//----------------------------------------------------

DWORD dwFogEnabled = 0;
DWORD dwFogColor = 0x00FF00FF;
BOOL gDisableAllFog = FALSE;

void SetupD3DFog(BOOL bEnable)
{
	float fFogStart = 500.0f;
	float fFogEnd = 700.0f;

	if(gDisableAllFog) bEnable = FALSE;

	if(pD3DDevice) {
		pD3DDevice->SetRenderState(D3DRS_FOGENABLE, bEnable);
		//pD3DDevice->SetRenderState(D3DRS_FOGCOLOR, dwFogColor);
		pD3DDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
		pD3DDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
		//pD3DDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&fFogStart));
		//pD3DDevice->SetRenderState(D3DRS_FOGEND, *(DWORD*)(&fFogEnd));
	}
}

//----------------------------------------------------

void _declspec(naked) RwRenderStateSetHook()
{
	_asm mov eax, [esp]
	_asm mov dwSetStateCaller, eax
	_asm mov eax, [esp+4]
	_asm mov dwSetStateOption, eax
	_asm mov eax, [esp+8]
	_asm mov dwSetStateParam, eax

	if(dwSetStateOption == 14) {
		if(dwSetStateParam) {
			SetupD3DFog(TRUE);
			dwFogEnabled = 1;
		} else {
			SetupD3DFog(FALSE);
			dwFogEnabled = 0;
		}
		_asm mov [esp+8], 0 ; no fog
	}

	_asm mov eax, dwOrgRwSetState
	_asm jmp eax
}

//----------------------------------------------------

void HookRwRenderStateSet()
{
	DWORD dwNewRwSetState = (DWORD)RwRenderStateSetHook;

	_asm mov ebx, 0xC97B24
	_asm mov eax, [ebx]
	_asm mov edx, [eax+32]
	_asm mov dwOrgRwSetState, edx
	_asm mov edx, dwNewRwSetState
	_asm mov [eax+32], edx

#ifdef _DEBUG
	sprintf(dbgstr,"HookRwRenderStateSet(0x%X)",dwOrgRwSetState);
	OutputDebugString(dbgstr);
#endif

}

//----------------------------------------------------

void CallRwRenderStateSet(int state, int option)
{
	_asm push option
	_asm push state
	_asm mov ebx, 0xC97B24
	_asm mov eax, [ebx]
	_asm call dword ptr [eax+32]
	_asm add esp, 8
}

//----------------------------------------------------

void SetupDialogResourceManager()
{
	pDialogResourceManager = new CDXUTDialogResourceManager();
	pDialogResourceManager->OnCreateDevice(pD3DDevice);
	pDialogResourceManager->OnResetDevice();
}

//----------------------------------------------------

void SetupGameUI()
{
	if(pGameUI) SAFE_DELETE(pGameUI);

	pGameUI = new CDXUTDialog();
    pGameUI->Init(pDialogResourceManager);
	pGameUI->SetCallback( OnGUIEvent );
	pGameUI->SetLocation(0,0);
	pGameUI->SetSize(pGame->GetScreenWidth(),pGame->GetScreenHeight());

	//pGameUI->AddButton(2,"Test Button",30,390,80,35,0);
	//pGameUI->AddEditBox(3,"",20,pGame->GetScreenHeight()-40,pGame->GetScreenWidth()-40,36,true);

	pGameUI->EnableMouseInput(false);
	pGameUI->EnableKeyboardInput(true);

	if(pChatWindow) pChatWindow->ResetDialogControls(pGameUI);
	if(pCmdWindow) pCmdWindow->ResetDialogControls(pGameUI);

}		

//----------------------------------------------------

extern void CheckDuplicateD3D9Dlls();

void DoInitStuff()
{
	// GAME INIT
	if(!bGameInited)
	{	
		OutputDebugString("Start of DoInitStuff()");
		
		CreateWorkingDirectory();

		sprintf(szConfigFile,"%s\\sa-mp.cfg",szWorkingPath);

		pConfig = new CConfig(szConfigFile);
		
		timeBeginPeriod(1); // increases the accuracy of Sleep()
		SubclassGameWindow();

		CheckDuplicateD3D9Dlls(); // Do this before any hooks are installed.

		// Grab the real IDirect3D9 * from the game.
		pD3D = (IDirect3D9 *)pGame->GetD3D();

		// Grab the real IDirect3DDevice9 * from the game.
		pD3DDevice = (IDirect3DDevice9 *)pGame->GetD3DDevice();

		*(IDirect3DDevice9Hook**)ADDR_ID3D9DEVICE = new IDirect3DDevice9Hook();

		pD3DDevice->ShowCursor(FALSE);

		OutputDebugString("Font and chat window creating..");

		// Create instances of the chat and input classes.
		pDefaultFont = new CFontRender(pD3DDevice);
		pChatWindow = new CChatWindow(pD3DDevice,pDefaultFont->m_pD3DFont);
		pCmdWindow = new CCmdWindow(pD3DDevice);

		// DXUT GUI INITIALISATION
		OutputDebugString("DXUTGUI creating..");

		SetupDialogResourceManager();
		
		OutputDebugString("SetupGameUI() creating..");

		SetupGameUI();	

		if(tSettings.bPlayOnline) {
			pDeathWindow = new CDeathWindow(pD3DDevice);
			pSpawnScreen = new CSpawnScreen(pD3DDevice);
			//pPlayerTags = new CPlayerTags(pD3DDevice);
			pNewPlayerTags = new CNewPlayerTags(pD3DDevice);
			pScoreBoard = new CScoreBoard(pD3DDevice, FALSE);
			pNetStats = new CNetStats(pD3DDevice);
			pSvrNetStats = new CSvrNetStats(pD3DDevice);
			pHelpDialog = new CHelpDialog(pD3DDevice);
		}
		
		OutputDebugString("Labels creating..");
		pLabel = new CLabel(pD3DDevice, "Verdana", false);

		// Setting up the commands.
		OutputDebugString("Setting up commands..");
		SetupCommands();

		OutputDebugString("Hooking RwSetState..");
		HookRwRenderStateSet();

		if(tSettings.bDebug) {
			CCamera *pGameCamera = pGame->GetCamera();
			pGameCamera->Restore();
			pGameCamera->SetBehindPlayer();
			pGame->DisplayHud(TRUE);
		}

		bGameInited = TRUE;
		OutputDebugString("End of DoInitStuff()");

		return;		
	}

	// NET GAME INIT
	if(!bNetworkInited && tSettings.bPlayOnline) {

		pNetGame = new CNetGame(tSettings.szConnectHost,atoi(tSettings.szConnectPort),
				tSettings.szNickName,tSettings.szConnectPass);

		bNetworkInited = TRUE;
		return;
	}
}

//----------------------------------------------------

void MarkAsModdedGame()
{
	bGameModded = TRUE;
}

//----------------------------------------------------

void TheGraphicsLoop()
{	
	_asm pushad // because we're called from a hook

	DoInitStuff();
	
	SetupD3DFog(TRUE);

	// Process the netgame if it's active.
	if(pNetGame) {
		//Sleep(0); // This hands the context over to raknet
		pNetGame->Process();
	}

	if(bQuitGame) {
		if((GetTickCount() - dwStartQuitTick) > 1000) {
			if (pNetGame)
			{
				delete pNetGame;
				pNetGame = NULL;
			}
			ExitProcess(0);
		}
		_asm popad
		return;
	}

	//pGame->UpdateFarClippingPlane();
	pGame->ProcessInputDisabling();
	if (bGameModded == TRUE)
	{
		FORCE_EXIT(0x4);
	}

	// We have to call the real Render2DStuff
	// because we overwrote its call to get here.
	_asm popad
	_asm mov edx, ADDR_RENDER2DSTUFF
	_asm call edx
}

//----------------------------------------------------

/*void TheGameLoop()
{	
	_asm pushad

	//SetCursor(LoadCursor(NULL,IDC_ARROW));
	//ShowCursor(TRUE);
	//OutputDebugString("---- New Frame ----");

	_asm popad
}*/

//----------------------------------------------------

void QuitGame()
{
	if(pNetGame && pNetGame->GetGameState() == GAMESTATE_CONNECTED) {
		pNetGame->GetRakClient()->Disconnect(500);
	}	
	bQuitGame = TRUE;
	dwStartQuitTick = GetTickCount();
}

//----------------------------------------------------

int GetDialogFontSize()
{
	return 2 * pConfig->GetIntConfigEntry("fontsize") + 20;
}

//----------------------------------------------------

int GetFontWeight()
{
	int iFontWeight = pConfig->GetIntConfigEntry("fontweight");
	switch (iFontWeight) {
		case 0: return FW_BOLD;
		case 1: return FW_NORMAL;
		default: return FW_BOLD;
	}
}

//----------------------------------------------------

PCHAR GetFontFace()
{
	if (pConfig && pConfig->GetStringConfigEntry("fontface"))
	{
		return pConfig->GetStringConfigEntry("fontface");
	}
	return "Arial";
}

//----------------------------------------------------

void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	switch( nControlID )
    {
        case IDC_CMDEDIT:
		{
			if(nEvent == EVENT_EDITBOX_STRING) {
				if(pCmdWindow) pCmdWindow->ProcessInput();
			}
			break;
		}
	}
	return;
}

//----------------------------------------------------

void InitSettings()
{
	PCHAR szCmdLine = GetCommandLineA();

	OutputDebugString(szCmdLine);
	OutputDebugString("\n");

	memset(&tSettings,0,sizeof(GAME_SETTINGS));

	while(*szCmdLine) {

		if(*szCmdLine == '-' || *szCmdLine == '/') {
			szCmdLine++;
			switch(*szCmdLine) {
				case 'd':
					tSettings.bDebug = TRUE;
					tSettings.bPlayOnline = FALSE;
					break;
				case 'c':
					tSettings.bPlayOnline = TRUE;
					tSettings.bDebug = FALSE;
					break;
				case 'z':
					szCmdLine++;
					SetStringFromCommandLine(szCmdLine,tSettings.szConnectPass);
					break;
				/*
				// We'll do this using ALT+ENTER
				case 'w':
					tSettings.bWindowedMode = TRUE;
					break;
				*/
				case 'h':
					szCmdLine++;
					SetStringFromCommandLine(szCmdLine,tSettings.szConnectHost);
					break;
				case 'p':
					szCmdLine++;
					SetStringFromCommandLine(szCmdLine,tSettings.szConnectPort);
					break;
				case 'n':
					szCmdLine++;
					SetStringFromCommandLine(szCmdLine,tSettings.szNickName);
					break;
			}
		}

		szCmdLine++;
	}
}

//----------------------------------------------------

void SetStringFromCommandLine(char *szCmdLine, char *szString)
{
	while(*szCmdLine == ' ') szCmdLine++;
	while(*szCmdLine &&
		  *szCmdLine != ' ' &&
		  *szCmdLine != '-' &&
		  *szCmdLine != '/') 
	{
		*szString = *szCmdLine;
		szString++; szCmdLine++;
	}
	*szString = '\0';
}

//----------------------------------------------------

void d3d9DestroyDeviceObjects()
{
	//if (pPlayerTags)
	//	pPlayerTags->DeleteDeviceObjects();

	pDialogResourceManager->OnDestroyDevice();

	if (pNewPlayerTags)
		pNewPlayerTags->DeleteDeviceObjects();

	if (pScoreBoard)
		pScoreBoard->DeleteDeviceObjects();

	if (pLabel)
		pLabel->DeleteDeviceObjects();

	if (pDefaultFont)
		pDefaultFont->DeleteDeviceObjects();

	if(pSpawnScreen)
		pSpawnScreen->DeleteDeviceObjects();

	if(pDeathWindow && pDeathWindow->m_pD3DFont) pDeathWindow->m_pD3DFont->OnLostDevice();
	if(pDeathWindow && pDeathWindow->m_pWeaponFont) pDeathWindow->m_pWeaponFont->OnLostDevice();
	if(pDeathWindow && pDeathWindow->m_pSprite) pDeathWindow->m_pSprite->OnLostDevice();
	
	if(pChatWindow && pChatWindow->m_pChatTextSprite) pChatWindow->m_pChatTextSprite->OnLostDevice();

	pDialogResourceManager->OnLostDevice();

}

void d3d9RestoreDeviceObjects()
{
	if(pDialogResourceManager) pDialogResourceManager->OnResetDevice();

	//if (pPlayerTags)
	//	pPlayerTags->RestoreDeviceObjects();

	if (pNewPlayerTags)
		pNewPlayerTags->RestoreDeviceObjects();

	if (pScoreBoard)
		pScoreBoard->RestoreDeviceObjects();

	if (pLabel)
		pLabel->RestoreDeviceObjects();

	if (pDefaultFont)
		pDefaultFont->RestoreDeviceObjects();

	if(pSpawnScreen) 
		pSpawnScreen->RestoreDeviceObjects();

	if(pDeathWindow && pDeathWindow->m_pD3DFont) pDeathWindow->m_pD3DFont->OnResetDevice();
	if(pDeathWindow && pDeathWindow->m_pWeaponFont) pDeathWindow->m_pWeaponFont->OnResetDevice();
	if(pDeathWindow && pDeathWindow->m_pSprite) pDeathWindow->m_pSprite->OnResetDevice();

	if(pChatWindow && pChatWindow->m_pChatTextSprite) pChatWindow->m_pChatTextSprite->OnResetDevice();

	pDialogResourceManager->OnCreateDevice(pD3DDevice);
	pDialogResourceManager->OnResetDevice();
	//SetupGameUI();
}

//----------------------------------------------------