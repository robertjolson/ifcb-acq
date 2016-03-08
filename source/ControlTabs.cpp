//-----------------------------------------------------------------------------
//  IFCB Project
//	ControlTabs.cpp
//	Martin Cooper
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "ControlTabs.h"
#include "CameraTab.h"
#include "FluidsTab.h"
#include "HardwareTab.h"
#include "GraphTab.h"
#include "Diagnostics.h"
#include "ExpertTab.h"

int cameraTab, fluidsTab, hardwareTab, graphTab, expertTab, diagnosticsTab;

//-----------------------------------------------------------------------------
CControlTabs::CControlTabs() {

	nTabs = 0;

	if (hasCamera) {
		cameraTab = nTabs;
		tabPages[nTabs++] = new CCameraTab;
	} else
		cameraTab = -1;

	fluidsTab = nTabs;
	tabPages[nTabs++] = new CFluidsTab;
	hardwareTab = nTabs;
	tabPages[nTabs++] = new CHardwareTab;
	graphTab = nTabs;
	tabPages[nTabs++] = new CGraphTab;

	if (hasExpertTab) {
		expertTab = nTabs;
		tabPages[nTabs++] = new CExpertTab;
	} else
		expertTab = -1;

#ifdef IFCB8_BENCHTOP
	tabPages[nTabs++] = new CDiagnosticsTab;
#else
	diagnosticsTab = -1;
#endif
}

//-----------------------------------------------------------------------------
CControlTabs::~CControlTabs() {

	for(int n = 0; n < nTabs; n++)
		delete tabPages[n];
}

//-----------------------------------------------------------------------------
void CControlTabs::Init() {

	int i = 0;

	if (hasCamera)
		tabPages[i++]->Create(IDD_CAMERA, this);
	tabPages[i++]->Create(IDD_FLUIDS, this);
	tabPages[i++]->Create(IDD_HARDWARE, this);
	tabPages[i++]->Create(IDD_GRAPH, this);
	if (hasExpertTab)
		tabPages[i++]->Create(IDD_EXPERT, this);

#ifdef IFCB8_BENCHTOP
	tabPages[diagnosticsTab]->Create(IDD_DIAGNOSTICS, this);
	tabPages[diagnosticsTab]->ShowWindow(SW_HIDE);
#endif

	CRect tabRect, itemRect;
	int nX, nY, nXc, nYc;

	GetClientRect(&tabRect);
	GetItemRect(0, &itemRect);

	nX = itemRect.left;
	nY = itemRect.bottom + 1;
	nXc = tabRect.right - itemRect.left - 1;
	nYc = tabRect.bottom - nY - 1;

	// make only the first tab visible
	tabPages[0]->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_SHOWWINDOW);
	for(int n = 1; n < nTabs; n++) {
		tabPages[n]->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_HIDEWINDOW);
	}
	currentTab = 0;
	tabPages[currentTab]->OnShowTab();
}

//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CControlTabs, CTabCtrl)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
void CControlTabs::OnLButtonDown(UINT nFlags, CPoint point)  {

	CTabCtrl::OnLButtonDown(nFlags, point);

	if (currentTab != GetCurFocus()){
		tabPages[currentTab]->ShowWindow(SW_HIDE);
		tabPages[currentTab]->OnLeaveTab();
		currentTab = GetCurFocus();
		tabPages[currentTab]->ShowWindow(SW_SHOW);
		tabPages[currentTab]->SetFocus();
		tabPages[currentTab]->OnShowTab();
	}
}


