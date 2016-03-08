//-----------------------------------------------------------------------------
//  IFCB Project
//	GraphTab.h
//	Martin Cooper
//-----------------------------------------------------------------------------
#pragma once
#include "resource.h"
#include "afxwin.h"
#include "Tab.h"
#include "CPlot Sources/Chart.h"

// Graph data
typedef struct {
	// one per data set
	double	*data[4];					// the actual data
	int		dataID[4];

	// common to all data sets
	int		nextIn;						// where to put the next incoming data point
	int		dimArray[2];
	int		nDims;
	int		nPoints;
	bool	running;
	enum	{	GRAPH_NONE,					// no graph
				GRAPH_ROI_ADC,				// ROI y val vs ADCs 
				GRAPH_ROI_PEAK,				// ROI y val vs ADCs 
				GRAPH_ADCB_ADCA,			// PMTBint vs PMTAint
				GRAPH_ADCC_ADCA,			// PMTCint vs PMTAint
				GRAPH_ADCC_ADCB,			// PMTCint vs PMTBint
				GRAPH_PEAKB_PEAKA,			// PMTBpeak vs PMTApeak
				GRAPH_PEAKC_PEAKA,			// PMTCpeak vs PMTApeak
				GRAPH_PEAKC_PEAKB,			// PMTCpeak vs PMTBpeak
				GRAPH_INT_T,				// oscilloscope plot of integrator channels
				GRAPH_PEAK_T				// oscilloscope plot of peak channels
			} GraphType;
} GRAPH_DATA;
extern GRAPH_DATA GraphData;

//-----------------------------------------------------------------------------
class CGraphTab : public CTab {

	DECLARE_DYNAMIC(CGraphTab)

public:
					CGraphTab(CWnd* pParent = NULL);
	virtual			~CGraphTab();
	void			UpdateGraph();
	enum { IDD = IDD_GRAPH };
	CXYChart		Chart;
//	afx_msg void OnEnChangeGraphPoints();
	CEdit			GraphYmax;
	CEdit			GraphXmax;
	afx_msg void	OnBnClickedSetYmax();
	afx_msg void	OnBnClickedSetXmax();

protected:
	virtual void	DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()
	afx_msg void	OnPaint();
	afx_msg void	OnBnClickedGraphRun();
	afx_msg void	OnBnClickedSetPoints();
	afx_msg void	OnCbnSelchangeGraphCombo();
	void			ShowAxMax(bool show);

	BOOL			OnInitDialog();
	void			InitData();
	void			Resize(int npoints);
	void			DrawAxes();
	double			axisMax;

	CRect			updateRect;
	CStatic			GraphPic;
	CButton			GraphRun;
	CEdit			GraphPoints;
	CComboBox		GraphCombo;
	CStatic			AxMaxLabel;
	CStatic			AxMaxBox;
	CStatic			AxMaxLabel2;
	CStatic			AxMaxBox2;
public:
	CButton AxMaxButton;
	CButton AxMaxButton2;
	afx_msg void OnStnClickedAxmaxLabel2();
};
