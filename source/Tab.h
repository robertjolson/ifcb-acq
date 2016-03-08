//-----------------------------------------------------------------------------
//  IFCB Project
//	Tab.h
//	Martin Cooper
//-----------------------------------------------------------------------------
#pragma once
#include "afxwin.h"
#include "resource.h"

class CTab : public CDialog {

public:
	enum { IDD = IDD_TAB };

	CTab(CWnd* pParent = NULL);
	CTab(UINT nIDTemplate, CWnd* pParent = NULL);

	virtual				~CTab(void);
	virtual bool		Run(bool run) {	return true; };
	virtual	void		UpdateGraph() {};
	virtual void		OnShowTab() {};
	virtual unsigned	GetFrameCount() { return 0; };
	virtual void		ZeroFrameCount() {};
	virtual void		OnLeaveTab() {};
	virtual void		CloseDriver() {};
	virtual BOOL		OpenDriver() { return TRUE; };

};
