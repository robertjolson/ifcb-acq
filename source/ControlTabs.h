//-----------------------------------------------------------------------------
//  IFCB Project
//	ControlTabs.h
//	Martin Cooper
//-----------------------------------------------------------------------------
#pragma once
#include "resource.h"
#include "Tab.h"

// ugly globals to allow access to individual tabs
extern int cameraTab, fluidsTab, hardwareTab, graphTab, expertTab, diagnosticsTab;
#define MAX_TABS	6

class CControlTabs : public CTabCtrl {

public:
	CControlTabs();
	virtual			~CControlTabs();
	CTab			*tabPages[MAX_TABS];
	int				currentTab;
	int				nTabs;
	void			Init();

protected:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
