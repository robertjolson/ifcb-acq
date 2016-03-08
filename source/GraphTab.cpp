//-----------------------------------------------------------------------------
//  IFCB Project
//	GraphTab.cpp : implementation file
//	Martin Cooper
//-----------------------------------------------------------------------------
#include <math.h>
#include "stdafx.h"
#include "GraphTab.h"
#include "FileIo.h"
#include "config.h"
#include "IfcbDlg.h"

GRAPH_DATA GraphData;						// graph data set selector

IMPLEMENT_DYNAMIC(CGraphTab, CDialog)

//-----------------------------------------------------------------------------
CGraphTab::CGraphTab(CWnd* pParent) : CTab(CGraphTab::IDD, pParent) {

}

//-----------------------------------------------------------------------------
CGraphTab::~CGraphTab() {

}

//-----------------------------------------------------------------------------
void CGraphTab::DoDataExchange(CDataExchange* pDX) {

	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRAPH, GraphPic);
	DDX_Control(pDX, IDC_GRAPH_RUN, GraphRun);
	DDX_Control(pDX, IDC_GRAPH_POINTS, GraphPoints);
	DDX_Control(pDX, IDC_GRAPH_COMBO, GraphCombo);
	DDX_Control(pDX, IDC_GRAPH_YMAX, GraphYmax);
	DDX_Control(pDX, IDC_AXMAX_LABEL, AxMaxLabel);
	DDX_Control(pDX, IDC_AXMAX_BOX, AxMaxBox);
	DDX_Control(pDX, IDC_SET_YMAX, AxMaxButton);
}

//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CGraphTab, CDialog)
    ON_WM_PAINT()
	ON_BN_CLICKED(IDC_GRAPH_RUN, &CGraphTab::OnBnClickedGraphRun)
	ON_BN_CLICKED(IDC_SET_POINTS, &CGraphTab::OnBnClickedSetPoints)
	ON_CBN_SELCHANGE(IDC_GRAPH_COMBO, &CGraphTab::OnCbnSelchangeGraphCombo)
//	ON_EN_CHANGE(IDC_GRAPH_POINTS, &CGraphTab::OnEnChangeGraphPoints)
	ON_BN_CLICKED(IDC_SET_YMAX, &CGraphTab::OnBnClickedSetYmax)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
void CGraphTab::UpdateGraph() {

	for (int i = 0; i < 4; i++)
		Chart.SetData(GraphData.dataID[i], GraphData.data[i], GraphData.nDims, GraphData.dimArray);
	GraphData.nextIn = (++GraphData.nextIn) % GraphData.nPoints;	

	GraphPic.InvalidateRect(updateRect, FALSE);
}

//-----------------------------------------------------------------------------
void CGraphTab::OnPaint() {

	CDialog::OnPaint();

    CPaintDC	dc(&GraphPic);
	CRect		clientRect;

	GraphPic.GetClientRect(&clientRect);
	dc.SetMapMode(MM_LOMETRIC);
	dc.DPtoLP((LPPOINT)&clientRect, 2);
	Chart.OnDraw(&dc, clientRect, clientRect);
}

//-----------------------------------------------------------------------------
void CGraphTab::InitData() {

//	Chart.DeleteAllData();

	// initialise data - shift all the points to the origin
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < GraphData.nPoints; i++) {
			GraphData.data[j][i] = 0.0;							// x data
			GraphData.data[j][i + GraphData.nPoints] = 0.0;
		}
		if (GraphData.dataID[j] == -1)
			GraphData.dataID[j] = Chart.AddData(GraphData.data[j], GraphData.nDims, GraphData.dimArray);
		else
			Chart.SetData(GraphData.dataID[j], GraphData.data[j], GraphData.nDims, GraphData.dimArray);
	}
	GraphPic.InvalidateRect(updateRect, FALSE);
}

//-----------------------------------------------------------------------------
void CGraphTab::Resize(int npoints) {

	// initialise the graph data
	GraphData.nPoints = npoints;
	GraphData.dimArray[1] = GraphData.nPoints;
	for (int i = 0; i < 4; i++)
		GraphData.data[i] = (double *)realloc(GraphData.data[i], 2 * sizeof(double) * GraphData.nPoints);
	GraphData.nextIn = 0;

	// update the nPoints box
	CString valstr;
	valstr.Format(_T("%d"), GraphData.nPoints);
	GraphPoints.SetSel(0, -1);
	GraphPoints.ReplaceSel(valstr, false);

	GraphPic.InvalidateRect(updateRect, FALSE);
}

//-----------------------------------------------------------------------------
// N.B. the scaling routines only use the first x axis and the first y and ignore any others
//-----------------------------------------------------------------------------
void CGraphTab::DrawAxes() {

	CAxis	*axis;
//	Chart.SetTitle(_T("A parabola"));

	switch (GraphData.GraphType) {
		case GraphData.GRAPH_ROI_ADC:			
		case GraphData.GRAPH_ROI_PEAK:
			axis = Chart.AddAxis(kLocationLeft);			// left axis
			axis->SetRange(1023.0, 0.0);
			axis->SetTitle(_T("ROI Y position"));
			axis = Chart.AddAxis(kLocationBottom);			// bottom axis
			axis->SetRange(0.0, axisMax);
			if (GraphData.GraphType == GraphData.GRAPH_ROI_PEAK)
				axis->SetTitle(_T("PMT peak (A=red, B=blue)"));
			else
				axis->SetTitle(_T("PMT integral (A=red, B=blue)"));
			break;

		case GraphData.GRAPH_ADCB_ADCA:						// PMTBint vs PMTAint
			axis = Chart.AddAxis(kLocationLeft);			// left axis
			axis->SetRange(0.0, axisMax);
			axis->SetTitle(_T("PMT B"));
			axis = Chart.AddAxis(kLocationBottom);			// bottom axis
			axis->SetRange(0.0, axisMax);
			axis->SetTitle(_T("PMT A"));
			break;
		case GraphData.GRAPH_ADCC_ADCA:						// PMTCint vs PMTAint
			axis = Chart.AddAxis(kLocationLeft);			// left axis
			axis->SetRange(0.0, axisMax);
			axis->SetTitle(_T("PMT C"));
			axis = Chart.AddAxis(kLocationBottom);			// bottom axis
			axis->SetRange(0.0, axisMax);
			axis->SetTitle(_T("PMT A"));
			break;
		case GraphData.GRAPH_ADCC_ADCB:						// PMTCint vs PMTBint
			axis = Chart.AddAxis(kLocationLeft);			// left axis
			axis->SetRange(0.0, axisMax);
			axis->SetTitle(_T("PMT C"));
			axis = Chart.AddAxis(kLocationBottom);			// bottom axis
			axis->SetRange(0.0, axisMax);
			axis->SetTitle(_T("PMT B"));
			break;

		case GraphData.GRAPH_PEAKB_PEAKA:						// PMTCint vs PMTBint
			axis = Chart.AddAxis(kLocationLeft);			// left axis
			axis->SetRange(0.0, axisMax);
			axis->SetTitle(_T("Peak B"));
			axis = Chart.AddAxis(kLocationBottom);			// bottom axis
			axis->SetRange(0.0, axisMax);
			axis->SetTitle(_T("Peak A"));
			break;

		case GraphData.GRAPH_PEAKC_PEAKA:						// PMTCint vs PMTBint
			axis = Chart.AddAxis(kLocationLeft);			// left axis
			axis->SetRange(0.0, axisMax);
			axis->SetTitle(_T("Peak C"));
			axis = Chart.AddAxis(kLocationBottom);			// bottom axis
			axis->SetRange(0.0, axisMax);
			axis->SetTitle(_T("Peak A"));
			break;

		case GraphData.GRAPH_PEAKC_PEAKB:						// PMTCint vs PMTBint
			axis = Chart.AddAxis(kLocationLeft);			// left axis
			axis->SetRange(0.0, axisMax);
			axis->SetTitle(_T("Peak C"));
			axis = Chart.AddAxis(kLocationBottom);			// bottom axis
			axis->SetRange(0.0, axisMax);
			axis->SetTitle(_T("Peak B"));
			break;

		case GraphData.GRAPH_INT_T:							// oscilloscope plot of integrator channels
			axis = Chart.AddAxis(kLocationLeft);			// left axis
//			axis->SetRange(FALSE, 0.0, 5.0);				// autorange
			axis->SetRange(0.0, axisMax);
			axis->SetTitle(_T("PMT INT (A=red, B=blue)"));
			axis = Chart.AddAxis(kLocationBottom);			// bottom axis
			axis->SetRange(FALSE, 0.0, GraphData.nPoints);	// autorange
//			axis->SetRange(0.0, 100.0);
			axis->SetTitle(_T("Trigger #"));
			break;

		case GraphData.GRAPH_PEAK_T:						// oscilloscope plot of peak channels
			axis = Chart.AddAxis(kLocationLeft);			// left axis
//			axis->SetRange(FALSE, 0.0, 5.0);				// autorange
			axis->SetRange(0.0, axisMax);
			axis->SetTitle(_T("Peak Volts (A=red, B=blue)"));
			axis = Chart.AddAxis(kLocationBottom);			// bottom axis
			axis->SetRange(FALSE, 0.0, GraphData.nPoints);	// autorange
			axis->SetTitle(_T("Trigger #"));
			break;

		case GraphData.GRAPH_NONE:
		default:		
			break;
	}
	
	GraphPic.InvalidateRect(updateRect, FALSE);
}

//-----------------------------------------------------------------------------
BOOL CGraphTab::OnInitDialog() {

	BOOL ret = CDialog::OnInitDialog();

	// initialise the graph data
	GraphData.GraphType = GraphData.GRAPH_NONE;
	GraphData.nDims = 2;
	GraphData.dimArray[0] = 2;
	for (int i = 0; i < 4; i++)
		GraphData.dataID[i] = -1;

	GraphCombo.SetCurSel(0);
	Resize(30);
	InitData();

	DrawAxes();						// axes etc
	
	Chart.SetMarkerType(GraphData.dataID[0], kXYMarkerSquare);
	Chart.SetMarkerType(GraphData.dataID[1], kXYMarkerCircle);
	Chart.SetMarkerType(GraphData.dataID[2], kXYMarkerTriangle);
	Chart.SetMarkerType(GraphData.dataID[3], kXYMarkerX);

	Chart.SetDataColor(GraphData.dataID[0], RGB(0xFF, 0, 0));
	Chart.SetDataColor(GraphData.dataID[1], RGB(0, 0, 0xFF));
	Chart.SetDataColor(GraphData.dataID[2], RGB(0, 0xFF, 0));
	Chart.SetDataColor(GraphData.dataID[3], RGB(0, 0xFF, 0xFF));

	for (int i = 0; i < 4; i++) {
		Chart.SetMarkerFrequency(GraphData.dataID[i], 1);
		Chart.SetChartType(GraphData.dataID[i], kXYChartPlain);
	}

	Chart.m_UseMajorHorizontalGrids = TRUE;
	Chart.m_UseMajorVerticalGrids = TRUE;

	GraphPic.GetWindowRect(&updateRect);
	GraphPic.ScreenToClient(&updateRect);

	return ret;
}

//-----------------------------------------------------------------------------
void CGraphTab::OnBnClickedGraphRun() {

    if (GraphRun.GetCheck() == BST_CHECKED) {				// Run
        GraphRun.SetWindowText(_T("Stop Graph"));
		GraphData.running = true;
		GraphCombo.EnableWindow(FALSE);
	} else {												// Stop
		GraphRun.SetWindowText( _T("Start Graph") );
		GraphData.running = false;
		GraphCombo.EnableWindow(TRUE);
	}
}

//-----------------------------------------------------------------------------
void CGraphTab::OnBnClickedSetPoints() {

	CString valstr;

	GraphPoints.GetWindowTextW(valstr);

	char cval[20];
	int n;
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %d", &n);

	Resize(n);
	OnPaint();
}

//-----------------------------------------------------------------------------
void CGraphTab::OnBnClickedSetYmax() {

	CString valstr;

	GraphYmax.GetWindowTextW(valstr);

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &axisMax);

	DrawAxes();
}

//-----------------------------------------------------------------------------
void CGraphTab::ShowAxMax(bool show) {

	AxMaxBox.ShowWindow(show);
	AxMaxLabel.ShowWindow(show);
	GraphYmax.ShowWindow(show);
	AxMaxButton.ShowWindow(show);
}

//-----------------------------------------------------------------------------
void CGraphTab::OnCbnSelchangeGraphCombo() {

	switch (GraphCombo.GetCurSel()) {
		case 1:									// PMT vs ROI Y
			GraphData.GraphType = GraphData.GRAPH_ROI_ADC;		
			//DEBUG_MESSAGE_EXT(_T("Red -\tchannel A low\r\nGreen -\tchannel A high\r\nBlue -\tchannel C low\r\nCyan -\tchannel C high\r\n"));
			DEBUG_MESSAGE_EXT(_T("Red -\tchannel A \r\nBlue -\tchannel B \r\n"));
			axisMax = 0.5;						// default xmax
			ShowAxMax(true);
			AxMaxLabel.SetWindowTextW(_T("X max"));
			AxMaxLabel.SetWindowTextW(_T("Y max"));
			break;
		case 2:									// PEAK vs ROI Y
			GraphData.GraphType = GraphData.GRAPH_ROI_PEAK;		
			DEBUG_MESSAGE_EXT(_T("Red -\tchannel A \r\nBlue -\tchannel B \r\nGreen -\tchannel C \r\n"));
			axisMax = 0.5;						// default xmax
			ShowAxMax(true);
			AxMaxLabel.SetWindowTextW(_T("X max"));
			break;
		case 3:									// PMTB vs PMTA
			GraphData.GraphType = GraphData.GRAPH_ADCB_ADCA;
			axisMax = 0.5;						// default xmax
			//axisMax = 5.0;						// default ymax
			ShowAxMax(true);
			AxMaxLabel.SetWindowTextW(_T("X max"));
			break;
		case 4:									// PMTC vs PMTA
			GraphData.GraphType = GraphData.GRAPH_ADCC_ADCA;
			axisMax = 0.5;						// default xmax
			//axisMax = 5.0;						// default ymax
			ShowAxMax(true);			break;
			AxMaxLabel.SetWindowTextW(_T("X max"));
		case 5:									// PMTD vs PMTA
			GraphData.GraphType = GraphData.GRAPH_ADCC_ADCB;
			axisMax = 0.5;						// default xmax
			//axisMax = 5.0;						// default ymax
			ShowAxMax(true);			break;
			AxMaxLabel.SetWindowTextW(_T("X max"));
		case 6:									// PMTs vs time
			GraphData.GraphType = GraphData.GRAPH_INT_T;
			axisMax = 0.5;						// default ymax
			ShowAxMax(true);
			AxMaxLabel.SetWindowTextW(_T("Y max"));
			break;
		case 7:									// PEAK vs time
			GraphData.GraphType = GraphData.GRAPH_PEAK_T;
			axisMax = 0.5;						// default ymax
			ShowAxMax(true);
			AxMaxLabel.SetWindowTextW(_T("Y max"));
			break;
		case 8:									// PMTD vs PMTA
			GraphData.GraphType = GraphData.GRAPH_PEAKB_PEAKA;
			axisMax = 0.5;						// default xmax
			//axisMax = 5.0;						// default ymax
			ShowAxMax(true);			break;
			AxMaxLabel.SetWindowTextW(_T("X max"));
		case 9:									// PMTD vs PMTA
			GraphData.GraphType = GraphData.GRAPH_PEAKC_PEAKA;
			axisMax = 0.5;						// default xmax
			//axisMax = 5.0;						// default ymax
			ShowAxMax(true);			break;
			AxMaxLabel.SetWindowTextW(_T("X max"));
		case 10:									// PMTD vs PMTA
			GraphData.GraphType = GraphData.GRAPH_PEAKC_PEAKB;
			axisMax = .5;						// default xmax
			//axisMax = 5.0;						// default ymax
			ShowAxMax(true);			break;
			AxMaxLabel.SetWindowTextW(_T("X max"));
		case 0:									// none
		default:		
			GraphData.GraphType = GraphData.GRAPH_NONE;
			ShowAxMax(false);
			break;
	}
	// update axisMax box
	CString str;
	str.Format(_T("%.2f"), axisMax);
	GraphYmax.SetSel(0, -1);
	GraphYmax.ReplaceSel(str, false);

	// 'delete' all points
	InitData();
/*	for (int i = 0; i < 4; i++)
		GraphData.data[0][GraphData.nextIn] = GraphData.data[0][GraphData.nextIn + GraphData.nPoints] = -1.0;*/

	DrawAxes();
}

//-----------------------------------------------------------------------------
/*void CGraphTab::OnEnChangeGraphPoints() {

	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CTab::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}*/

