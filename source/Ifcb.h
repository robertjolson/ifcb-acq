//-----------------------------------------------------------------------------
//  IFCB Project
//	Ifcb.h
//	Martin Cooper
//-----------------------------------------------------------------------------
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CIfcbApp:
// See Ifcb.cpp for the implementation of this class
//

class CIfcbApp : public CWinApp
{
public:
	CIfcbApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CIfcbApp theApp;