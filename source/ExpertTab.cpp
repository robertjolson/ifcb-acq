//-----------------------------------------------------------------------------
//  IFCB Project
//	ExpertTab.cpp : implementation file
//	Martin Cooper
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "ExpertTab.h"
#include "config.h"
#include "IfcbDlg.h"
#include "FileIo.h"
#include "Daq.h"

IMPLEMENT_DYNAMIC(CExpertTab, CDialog)

//-----------------------------------------------------------------------------
CExpertTab::CExpertTab(CWnd* pParent)	: CTab(CExpertTab::IDD, pParent) {

}

//-----------------------------------------------------------------------------
CExpertTab::~CExpertTab() {

}

//-----------------------------------------------------------------------------
void CExpertTab::DoDataExchange(CDataExchange* pDX) {

	DDX_Control(pDX, IDC_EDIT4, DataPath);
	DDX_Control(pDX, IDC_EDIT17, FileComment);
	DDX_Control(pDX, IDC_EDIT5, IntSettleTime);
	DDX_Control(pDX, IDC_EDIT6, PulseStretch);
	DDX_Control(pDX, IDC_EDIT7, MaxPulseLength);
	DDX_Control(pDX, IDC_ADC_COMBO, AdcCombo);
	DDX_Control(pDX, IDC_PRIMPORT_VALVE, PrimPortValve);
	DDX_Control(pDX, IDC_SECPORT_VALVE, SecPortValve);
	DDX_Control(pDX, IDC_EDIT8, FlashDelay);
	DDX_Control(pDX, IDC_EDIT9, FlashPulseLength);
	//DDX_Control(pDX, IDC_EDIT10, CameraPulseLength);
	DDX_Control(pDX, IDC_EDIT10, MinBlobArea);
	DDX_Control(pDX, IDC_EDIT15, BinarizeThresh);
	DDX_Control(pDX, IDC_EDIT16, SyringeSize);
	DDX_Control(pDX, IDC_EDIT11, HumidityThreshold);
	DDX_Control(pDX, IDC_EDIT13, CameraPulseLength);
	DDX_Control(pDX, IDC_ACQSTARTTIMEOUT, AcqStartTimeout);
}

BEGIN_MESSAGE_MAP(CExpertTab, CTab)
ON_BN_CLICKED(IDC_BUTTON1, &CExpertTab::OnBnClickedButton1)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
BOOL CExpertTab::OnInitDialog() {

	return CDialog::OnInitDialog();
}

//-----------------------------------------------------------------------------
void CExpertTab::UpdateData() {

	CString valstr;

	valstr = dataPath;										// 3) data path
	DataPath.SetSel(0, -1);
	DataPath.ReplaceSel(valstr, false);

	valstr = fileComment;										// 17) fiel comment
	FileComment.SetSel(0, -1);
	FileComment.ReplaceSel(valstr, false);

	valstr.Format(_T("%.1f"), pulseStretchTime * 2.170);	// 44) pulse stretch
	PulseStretch.SetSel(0, -1);
	PulseStretch.ReplaceSel(valstr, false);

	valstr.Format(_T("%.3f"), maxPulseStretch * 2.170e-3);	// 63) max pulse length
	MaxPulseLength.SetSel(0, -1);
	MaxPulseLength.ReplaceSel(valstr, false);

	valstr.Format(_T("%.1f"), integratorSettleTime * 2.170);// 17) integrator settle time
	IntSettleTime.SetSel(0, -1);
	IntSettleTime.ReplaceSel(valstr, false);

	valstr.Format(_T("%d"), adcDrate);						// 22) ADC data rate
	AdcCombo.SetCurSel(AdcCombo.FindStringExact(-1, valstr));
	
	valstr.Format(_T("%d"), primValvePort);						// Primary intake port valve
	PrimPortValve.SetCurSel(PrimPortValve.FindStringExact(-1, valstr));

	valstr.Format(_T("%d"), secValvePort);						// Secondary intake port valve
	SecPortValve.SetCurSel(SecPortValve.FindStringExact(-1, valstr));

	valstr.Format(_T("%.1f"), flashDelay * 0.434);			// 29) flash delay
	FlashDelay.SetSel(0, -1);
	FlashDelay.ReplaceSel(valstr, false);

	valstr.Format(_T("%.1f"), flashPulseLength * 0.434);	// 30) flash pulse length
	FlashPulseLength.SetSel(0, -1);
	FlashPulseLength.ReplaceSel(valstr, false);

	valstr.Format(_T("%.1f"), cameraPulseLength * 0.434);	// 31) camera pulse length
	CameraPulseLength.SetSel(0, -1);
	CameraPulseLength.ReplaceSel(valstr, false);
	
	valstr.Format(_T("%.1f"), humidityThreshold * 1.0);		// 66) humidity alarm threshold
	HumidityThreshold.SetSel(0, -1);
	HumidityThreshold.ReplaceSel(valstr, false);

	valstr.Format(_T("%d"), minimumBlobArea);				// 16) minimum blob area
	MinBlobArea.SetSel(0, -1);
	MinBlobArea.ReplaceSel(valstr, false);

	valstr.Format(_T("%d"), binarizeThresh);				// 13) binarize threshold
	BinarizeThresh.SetSel(0, -1);
	BinarizeThresh.ReplaceSel(valstr, false);

	valstr.Format(_T("%d"), syringeSize);				// 91) syringe size (ml)
	SyringeSize.SetSel(0, -1);
	SyringeSize.ReplaceSel(valstr, false);

	valstr.Format(_T("%u"), acqStartTimeout / 1000);		// 70) timeout for first frame
	AcqStartTimeout.SetSel(0, -1);
	AcqStartTimeout.ReplaceSel(valstr, false);
}

//-----------------------------------------------------------------------------
void CExpertTab::OnShowTab() {

	CTab::OnShowTab();
	UpdateData();
}

//-----------------------------------------------------------------------------
void CExpertTab::OnLeaveTab() {

	OnBnClickedButton1();
}

//-----------------------------------------------------------------------------
bool CExpertTab::Run(bool run) {

	if (run) {												// Run
		DataPath.EnableWindow(FALSE);
		return true;
	} else {												// Stop 
		DataPath.EnableWindow(TRUE);
		return false;
	}
}

//-----------------------------------------------------------------------------
uint16 CExpertTab::ExtractInt(CEdit *edit, double scale) {

	CString valstr;
	char cval[20];
	double val;

	edit->GetWindowTextW(valstr);
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &val);

	return (uint16)(val / scale);
}

//-----------------------------------------------------------------------------
double CExpertTab::ExtractDouble(CEdit *edit, double scale) {

	CString valstr;
	char cval[20];
	double val;

	edit->GetWindowTextW(valstr);
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &val);

	return val / scale;
}

//-----------------------------------------------------------------------------
void CExpertTab::OnBnClickedButton1() {

	CString valstr;

	DataPath.GetWindowTextW(valstr);
	CStringToChar(dataPath, &valstr);

	FileComment.GetWindowTextW(valstr);
	CStringToChar(fileComment, &valstr);

	pulseStretchTime = ExtractInt(&PulseStretch, 2.170);
	ChangeHWParam(PULSE_STRETCH, pulseStretchTime);

	maxPulseStretch = ExtractInt(&MaxPulseLength, 2.170e-3);
	ChangeHWParam(MAX_PULSE_STRETCH, maxPulseStretch);

	integratorSettleTime = ExtractInt(&IntSettleTime, 2.170);
	ChangeHWParam(INTEGRATOR_SETTLE, integratorSettleTime);

	flashDelay = ExtractInt(&FlashDelay, 0.434);
	ChangeHWParam(FLASH_DELAY, flashDelay);

	flashPulseLength = ExtractInt(&FlashPulseLength, 0.434);
	ChangeHWParam(FLASH_LENGTH, flashPulseLength);

	cameraPulseLength = ExtractInt(&CameraPulseLength, 0.434);
	ChangeHWParam(CAMERA_LENGTH, cameraPulseLength);

	minimumBlobArea = ExtractInt(&MinBlobArea, 1.0);
	binarizeThresh = ExtractInt(&BinarizeThresh, 1.0);
	syringeSize = ExtractInt(&SyringeSize, 1.0);
	humidityThreshold = ExtractDouble(&HumidityThreshold, 1.0);
	acqStartTimeout = (unsigned long)ExtractDouble(&AcqStartTimeout, 0.001);

	char cval[20];
	AdcCombo.GetLBText(AdcCombo.GetCurSel(), valstr);
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %hd", &adcDrate);
	ChangeHWParam(ADC_RATE, adcDrate);

	PrimPortValve.GetLBText(PrimPortValve.GetCurSel(), valstr);
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %hd", &primValvePort);
	//ChangeHWParam(ADC_RATE, adcDrate);

	SecPortValve.GetLBText(SecPortValve.GetCurSel(), valstr);
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %hd", &secValvePort);
	//ChangeHWParam(ADC_RATE, adcDrate);

	WriteCfgFile();
	UpdateData();
}

