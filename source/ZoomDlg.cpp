//-----------------------------------------------------------------------------
//  IFCB Project
//	ZoomDlg.cpp
//	Martin Cooper
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "ZoomDlg.h"
//#include "IfcbDlg.h"			// debug only

IMPLEMENT_DYNAMIC(CZoomDlg, CDialog)

//-----------------------------------------------------------------------------
CZoomDlg::CZoomDlg(CWnd* pParent) : CDialog(CZoomDlg::IDD, pParent)
{
	parent = (CCameraTab *)pParent;
	initialised = false;
	parent->zoomed = true;
}

//-----------------------------------------------------------------------------
CZoomDlg::~CZoomDlg() {

	parent->zoomed = false;
}

//-----------------------------------------------------------------------------
void CZoomDlg::DoDataExchange(CDataExchange* pDX) {

	CDialog::DoDataExchange(pDX);
}

//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CZoomDlg, CDialog)
	ON_WM_SIZE()
    ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
BOOL CZoomDlg::OnInitDialog() {

    CDialog::OnInitDialog();

	imageRect.left = imageRect.top = 0;
	imageRect.right = parent->Image.GetWidth();
	imageRect.bottom = parent->Image.GetHeight();
	hScrollPos = vScrollPos = 0;
    GetClientRect(&dlgRect);
	SetScroll(dlgRect.right - dlgRect.left, dlgRect.bottom - dlgRect.top);
	initialised = true;

	return true;
}

//-----------------------------------------------------------------------------
void CZoomDlg::OnPaint() {

    CPaintDC dc(this);

	parent->Image.Draw(dc.m_hDC, -hScrollPos, -vScrollPos);

	CDialog::OnPaint();
 }

//-----------------------------------------------------------------------------
void CZoomDlg::SetScroll(int cx, int cy) {

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);

	cx = min(cx, imageRect.Width());
	cy = min(cy, imageRect.Height());

	// vertical scroll
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = (cy < imageRect.Height()) ? imageRect.Height() - cy : 0;
	si.nPage = BIG_SCROLL;
	si.nPos = vScrollPos;
    SetScrollInfo(SB_VERT, &si, FALSE); 

	// horizontal scroll
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = (cx < imageRect.Width()) ? imageRect.Width() - cx : 0;
	si.nPage = BIG_SCROLL;
	si.nPos = hScrollPos;
    SetScrollInfo(SB_HORZ, &si, FALSE); 

/*	CString debugStr;
    GetClientRect(&dlgRect);
	debugStr.Format(_T("h = %d, W = %d, P = %d, range %d - %d, lim = %d\r\n"), 
		hScrollPos, dlgRect.right - dlgRect.left, GetScrollPos(SB_HORZ), si.nMin, si.nMax, GetScrollLimit(SB_HORZ));
	DEBUG_MESSAGE_EXT(debugStr);*/
}

//-----------------------------------------------------------------------------
void CZoomDlg::OnSize(UINT nType, int cx, int cy)  {

	CDialog::OnSize(nType, cx, cy);

	if (imageRect.Width() == 0)
		return;

	SetScroll(cx, cy);

	// redraw
    GetWindowRect(&dlgRect);				// dlgRect is in screen coords
    ScreenToClient(&dlgRect);				// converts screen coords to dlg coords. dlgRect is now in dlg coords
    InvalidateRect(dlgRect, FALSE);
}

//-----------------------------------------------------------------------------
void CZoomDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {

	int minPos;
	int maxPos;
	
	GetScrollRange(SB_VERT, &minPos, &maxPos); 
	maxPos = GetScrollLimit(SB_VERT);

	// Get the current position of scroll box.
	int curPos = GetScrollPos(SB_VERT);

	switch (nSBCode) {
		case SB_LINEDOWN:
			curPos = min(maxPos, curPos + SMALL_SCROLL);
			break;

		case SB_LINEUP:
			curPos = max(minPos, curPos - SMALL_SCROLL);
			break;

		case SB_PAGEDOWN:
			curPos = min(maxPos, curPos + BIG_SCROLL);
			break;

		case SB_PAGEUP:
			curPos = max(minPos, curPos - BIG_SCROLL);
			break;

		case SB_THUMBPOSITION:
			curPos = nPos;
			break;

		default:
			return;
	}

	SetScrollPos(SB_VERT, curPos, TRUE);
	ScrollWindow(curPos - vScrollPos, 0);
	vScrollPos = curPos;

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);

    GetWindowRect(&dlgRect);
    ScreenToClient(&dlgRect);
	InvalidateRect(dlgRect, FALSE);
}

//-----------------------------------------------------------------------------
void CZoomDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {

	int minPos;
	int maxPos;
	
	GetScrollRange(SB_HORZ, &minPos, &maxPos); 
	maxPos = GetScrollLimit(SB_HORZ);

	// Get the current position of scroll box.
	int curPos = GetScrollPos(SB_HORZ);

	switch (nSBCode) {
		case SB_LINERIGHT:
			curPos = min(maxPos, curPos + SMALL_SCROLL);
			break;

		case SB_LINELEFT:
			curPos = max(minPos, curPos - SMALL_SCROLL);
			break;

		case SB_PAGERIGHT:
			curPos = min(maxPos, curPos + BIG_SCROLL);
			break;

		case SB_PAGELEFT:
/*			SCROLLINFO   info;
			GetScrollInfo(SB_HORZ, &info, SIF_ALL);						// Get the page size. 
			curPos = max(minPos, curPos - (int)info.nPage);*/
			curPos = max(minPos, curPos - BIG_SCROLL);
			break;

		case SB_THUMBPOSITION:
			curPos = nPos;
			break;

		default:
			return;
	}

	SetScrollPos(SB_HORZ, curPos, TRUE);
	ScrollWindow(curPos - hScrollPos, 0);
	hScrollPos = curPos;

/*	CString debugStr;
    GetClientRect(&dlgRect);
	debugStr.Format(_T("h = %d, W = %d, P = %d, range %d - %d, lim = %d\r\n"), 
		hScrollPos, dlgRect.right - dlgRect.left, GetScrollPos(SB_HORZ), minPos, maxPos, GetScrollLimit(SB_HORZ));
	DEBUG_MESSAGE_EXT(debugStr);*/

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);

    GetWindowRect(&dlgRect);
    ScreenToClient(&dlgRect);
	InvalidateRect(dlgRect, FALSE);
}
