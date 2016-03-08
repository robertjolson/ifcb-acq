//-----------------------------------------------------------------------------
//  IFCB Project
//	ZoomDlg.h
//	Martin Cooper
//-----------------------------------------------------------------------------
#pragma once

#include "resource.h"
#include "afxwin.h"
#include "CameraTab.h"

#define SMALL_SCROLL	10
#define BIG_SCROLL		100

class CCameraTab;
class CZoomDlg : public CDialog {

	DECLARE_DYNAMIC(CZoomDlg)

public:
	CZoomDlg(CWnd* pParent = NULL);
	virtual ~CZoomDlg();

	enum { IDD = IDD_ZOOM };
	bool initialised;

private:
	CCameraTab		*parent;

protected:
	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void	OnSize(UINT nType, int cx, int cy);
    afx_msg void	OnPaint();
    virtual BOOL	OnInitDialog();
	void			SetScroll(int cx, int cy);

	int				vScrollPos;
	int				hScrollPos;
	CRect			imageRect;
	CRect			dlgRect;
	void			OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	void			OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	DECLARE_MESSAGE_MAP()
};
