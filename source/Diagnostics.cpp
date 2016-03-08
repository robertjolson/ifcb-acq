//-----------------------------------------------------------------------------
//  IFCB Project
//	Diagnostics.h
//	Martin Cooper
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "Diagnostics.h"


IMPLEMENT_DYNAMIC(CDiagnosticsTab, CDialog)

//-----------------------------------------------------------------------------
CDiagnosticsTab::CDiagnosticsTab(CWnd* pParent) : CTab(CDiagnosticsTab::IDD, pParent) {

}

//-----------------------------------------------------------------------------
CDiagnosticsTab::~CDiagnosticsTab() {

}

//-----------------------------------------------------------------------------
void CDiagnosticsTab::DoDataExchange(CDataExchange* pDX) {

	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDiagnosticsTab, CDialog)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
