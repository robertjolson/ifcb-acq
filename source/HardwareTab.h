//-----------------------------------------------------------------------------
//  IFCB Project
//	HardwareTab.h
//	Martin Cooper
//-----------------------------------------------------------------------------
#pragma once
#include "resource.h"
#include "afxwin.h"
#include "config.h"
#include "Tab.h"
#include "FocusMotorControl.h"

class CHardwareTab : public CTab
{
	DECLARE_DYNAMIC(CHardwareTab)

public:
	CHardwareTab(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHardwareTab();

	enum { IDD = IDD_HARDWARE };
	virtual bool	Run(bool run);

private:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedReadback();
	afx_msg void OnBnClickedLive();
	afx_msg void OnBnClickedDacsOn();
	afx_msg void OnBnClickedForceTrigNoCam();
	afx_msg void OnBnClickedResetTrig();
	afx_msg void OnBnClickedPollADCs();
	afx_msg void OnBnClickedFlashOn();
	afx_msg void OnBnClickedFanSet();
	afx_msg void OnBnClickedHumidSet();
	afx_msg void OnBnClickedPmtASet();
	afx_msg void OnBnClickedPmtBSet();
	afx_msg void OnBnClickedPmtCSet();
	afx_msg void OnBnClickedFlashSet();
	afx_msg void OnBnClickedFlashSet2();
	afx_msg void OnBnClickedFill1microinjector();
	afx_msg void OnBnClickedFill2bypassFilter();
	afx_msg void OnBnClickedFill3newSheath();
	afx_msg void OnBnClickedIsol();
	afx_msg void OnBnClickedSpare();
	afx_msg void OnBnClickedFans();
	afx_msg void OnBnClickedLaser();
	afx_msg void OnBnClickedCamera();
	afx_msg void OnBnClickedStirrer();
	afx_msg void OnBnClickedPump1();
	afx_msg void OnBnClickedPump2();
	afx_msg void OnBnClickedFanAuto();
	afx_msg void OnBnClickedHumidAuto();
	afx_msg void OnBnClickedFocusMotor();
	afx_msg void OnEnChangeHumidAdc();
	afx_msg void OnEnChangeTempAdc();

	void UpdateData();
	void OnTimer(UINT_PTR nIDEvent);
	void OnShowTab();

	CButton PmtADacState;
	CButton PmtBDacState;
	CButton PmtCDacState;
	CButton TrigDacState;
	CButton FlashDacState;
	CEdit PmtADacSet;
	CEdit PmtBDacSet;
	CEdit PmtCDacSet;
	CEdit FlashDacSet;
	CEdit FlashDacSet2;
	CEdit FanThresh;
	CEdit HumidThresh;
	CEdit HumidAdc;
	CEdit TempAdc;
	CButton HumidSet;
	CButton LiveReadback;
	CButton Fill1microinjector;
	CButton Fill2bypassFilter;
	CButton Fill3newSheath;
	CButton Isol;
	CButton Spare;
	CButton Fans;
	CButton Laser;
	CButton Camera;
	CButton Stirrer;
	CButton Pump1;
	CButton Pump2;
	CButton FansAuto;
	CButton HumidAuto;
	CButton FanSet;
	CStatic CameraLabel;
	CStatic LaserLabel;
	CStatic TrigLabel;
	CStatic FlashLabel;
	CStatic IsolLabel;
	CStatic SpareLabel;
	CStatic FanLabel;
	CStatic FanThreshLabel;
	CStatic HumidThreshLabel;
	CButton Readback;
	CButton ResetTrig;
	CButton ForceTrigNoCam;
	CButton PollADCs;
	CButton DacsOn;
	CStatic Fill1microinjectorLabel;
	CStatic Fill2bypassFilterLabel;
	CStatic Fill3newSheathLabel;
	CStatic ValveGroup;
	CStatic StirrerLabel;
	CStatic Pump1Label;
	CStatic Pump2Label;
	CStatic SwitchGroup;
	CFocusMotorControl *FocusMotorControl;
	CFocusMotorControl *LaserMotorControl;
	CButton FocusMotorButt;
	CButton Flashlamp;
	afx_msg void OnBnClickedFlashlamp();
#ifdef DAQ_MCC
	CEdit TrigThreshA;
	CEdit TrigThreshB;
	CEdit TrigThreshC;
	CButton TrigEnA;
	CButton TrigEnB;
	CButton TrigEnC;
	afx_msg void OnBnClickedTrigSeta();
	afx_msg void OnBnClickedTrigSetb();
	afx_msg void OnBnClickedTrigSetc();
	afx_msg void OnBnClickedTrigSetd();
	afx_msg void OnBnClickedEna();
	afx_msg void OnBnClickedEnb();
	afx_msg void OnBnClickedEnc();
	afx_msg void OnBnClickedEnd();
	CComboBox PgaCombo;
	CComboBox PgaCombo2;
public:
	afx_msg void OnCbnSelchangePgaCombo();
#endif
	CEdit PumpVolts;
	CStatic PumpVoltsLabel;
	CButton PumpVoltsSet;
	afx_msg void OnBnClickedPumpSet();
	CEdit AltConfig;
	afx_msg void OnBnClickedAltCfgSet();
	CEdit PmtADacSet2;
	CEdit PmtBDacSet2;
	CEdit PmtCDacSet2;
	CEdit TrigThreshA2;
	CEdit TrigThreshB2;
	CEdit TrigThreshC2;
	CButton TrigEnA2;
	CButton TrigEnB2;
	CButton TrigEnC2;
	afx_msg void OnBnClickedPmtaSet2();
	afx_msg void OnBnClickedPmtbSet2();
	afx_msg void OnBnClickedPmtcSet2();
	afx_msg void OnBnClickedTrigSeta2();
	afx_msg void OnBnClickedTrigSetb2();
	afx_msg void OnBnClickedTrigSetc2();
	afx_msg void OnBnClickedTrigEna2();
	afx_msg void OnBnClickedTrigEnb2();
	afx_msg void OnBnClickedTrigEnc2();
	afx_msg void OnCbnSelchangePgaCombo2();
	CStatic PmtA2Label;
	CStatic PmtB2Label;
	CStatic PmtC2Label;
	CStatic Pga2Label;
	CButton PmtA2Set;
	CButton PmtB2Set;
	CButton PmtC2Set;
	CStatic TrigA2Label;
	CStatic TrigB2Label;
	CStatic TrigC2Label;
	CButton TrigA2Set;
	CButton TrigB2Set;
	CButton TrigC2Set;
	CStatic AltGroup;
	afx_msg void OnBnClickedLaserMotor();
	afx_msg void OnStnClickedFill3newSheathLabel();
	afx_msg void OnEnChangeAltConfig();
};
