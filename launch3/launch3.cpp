
// launch3.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "launch3.h"
#include "launch3Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLaunch3App

BEGIN_MESSAGE_MAP(CLaunch3App, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CLaunch3App construction

CLaunch3App::CLaunch3App()
{
}


// The one and only CLaunch3App object

CLaunch3App theApp;


// CLaunch3App initialization

BOOL CLaunch3App::InitInstance()
{
	// Standard initialization

#ifdef _AFXDLL
	//Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	//Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CLaunch3Dlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

