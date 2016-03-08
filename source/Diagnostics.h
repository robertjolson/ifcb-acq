//-----------------------------------------------------------------------------
//  IFCB Project
//	Diagnostics.h
//	Martin Cooper
//-----------------------------------------------------------------------------
#pragma once

#include "Tab.h"


class CDiagnosticsTab : public CTab {

	DECLARE_DYNAMIC(CDiagnosticsTab)

public:
	CDiagnosticsTab(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDiagnosticsTab();

	enum { IDD = IDD_DIAGNOSTICS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
