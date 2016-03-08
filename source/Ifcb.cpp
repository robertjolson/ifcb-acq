//-----------------------------------------------------------------------------
//  IFCB Project
//	Ifcb.cpp
//	Martin Cooper
//
//	12 August, 2010		Intial release
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Ifcb.h"
#include "IfcbDlg.h"
#include "FileIo.h"
#include "config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CIfcbApp

BEGIN_MESSAGE_MAP(CIfcbApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CIfcbApp construction

CIfcbApp::CIfcbApp()
{
	ReadCfgFile();								// read config parameters [--- but (Rob) it never seems to get to this line...??]
}	

// The one and only CIfcbApp object

CIfcbApp theApp;


// CIfcbApp initialization

BOOL CIfcbApp::InitInstance()
{
	CWinApp::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("WHOI Imaging FlowCytobot"));

	CIfcbDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	ReadCfgFile();	//Rob 2 Feb 2016 to get correct StartFreq into freq.txt...

	if (DoFreqScan){
	WriteFreqFile(); //Rob 2 Feb 2016 to get correct StartFreq into freq.txt......
	Sleep(100000);
	ReadFreqFile();	//initialize freq.txt before starting; putting up higher causes crash at first DEBUG (because the Debug window was not established yet?).
	PZTfrequency = (int)StartFreq;	
	stepCount = 1;
	WriteFreqFile();
	}


	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
