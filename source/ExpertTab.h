//-----------------------------------------------------------------------------
//  IFCB Project
//	ExpertTab.h
//	Martin Cooper
//-----------------------------------------------------------------------------
#pragma once
#include "resource.h"
#include "afxwin.h"
#include "config.h"
#include "Tab.h"

class CExpertTab : public CTab {

	DECLARE_DYNAMIC(CExpertTab)

public:
	CExpertTab(CWnd* pParent = NULL);   // standard constructor
	virtual ~CExpertTab();

	enum { IDD = IDD_EXPERT };
	DECLARE_MESSAGE_MAP()

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL			OnInitDialog();
	virtual void	OnShowTab();
	virtual void	OnLeaveTab();
	virtual bool	Run(bool run);
	void			UpdateData();
	uint16			ExtractInt(CEdit *edit, double scale);
	double			ExtractDouble(CEdit *edit, double scale);

	afx_msg void OnBnClickedButton1();
	CEdit FluidsPort;
	CEdit DaqPort;
	CEdit DataPath;
	CEdit FileComment;
	CEdit IntSettleTime;
	CEdit PulseStretch;
	CEdit MaxPulseLength;
	CComboBox AdcCombo;
	CComboBox PrimPortValve;
	CComboBox SecPortValve;
	CEdit FlashDelay;
	CEdit FlashPulseLength;
	CEdit CameraPulseLength;
	CEdit MinBlobArea;
	CEdit BinarizeThresh;
	CEdit SyringeSize;
	CEdit HumidityThreshold;
	CEdit AcqStartTimeout;
};

