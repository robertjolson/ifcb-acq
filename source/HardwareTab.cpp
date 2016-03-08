
//-----------------------------------------------------------------------------
//  IFCB Project
//	HardwareTab.cpp : implementation file
//	Martin Cooper
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "HardwareTab.h"
#include "config.h"
#include "FileIo.h"
#include "Daq.h"
#include "IfcbDlg.h"
#include "GraphTab.h"
#include "Process.h"
#include "config.h"
//#include <stdio.h>      /* printf, fgets */
//#include <stdlib.h>     /* atoi */

#include <math.h>
#include "Fluids.h"
#include "Analysis.h"



IMPLEMENT_DYNAMIC(CHardwareTab, CDialog)

#define HOUSEKEEPING_TIMER_ID	1

//-----------------------------------------------------------------------------
CHardwareTab::CHardwareTab(CWnd* pParent)	: CTab(CHardwareTab::IDD, pParent) {

}

//-----------------------------------------------------------------------------
CHardwareTab::~CHardwareTab() {

}

//-----------------------------------------------------------------------------
void CHardwareTab::DoDataExchange(CDataExchange* pDX) {

	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PMTA_SET, PmtADacState);
	DDX_Control(pDX, IDC_PMTB_SET, PmtBDacState);
	DDX_Control(pDX, IDC_PMTC_SET, PmtCDacState);
	DDX_Control(pDX, IDC_FLASH_SET, FlashDacState);
	DDX_Control(pDX, IDC_FLASH_SET2, FlashDacState);
	DDX_Control(pDX, IDC_PMTA_SP, PmtADacSet);
	DDX_Control(pDX, IDC_PMTB_SP, PmtBDacSet);
	DDX_Control(pDX, IDC_PMTC_SP, PmtCDacSet);
	DDX_Control(pDX, IDC_FLASH_SP, FlashDacSet);
	DDX_Control(pDX, IDC_FLASH_SP2, FlashDacSet2);
	DDX_Control(pDX, IDC_FAN_THRESH, FanThresh);
	DDX_Control(pDX, IDC_EDIT14, HumidThresh);
	DDX_Control(pDX, IDC_HUMID_ADC, HumidAdc);
	DDX_Control(pDX, IDC_TEMP_ADC, TempAdc);
	DDX_Control(pDX, IDC_HUMID_SET, HumidSet);
	DDX_Control(pDX, IDC_LIVE, LiveReadback);
	DDX_Control(pDX, IDC_FILL1microinjector, Fill1microinjector);
	DDX_Control(pDX, IDC_FILL2bypassFilter, Fill2bypassFilter);
	DDX_Control(pDX, IDC_FILL3newSheath, Fill3newSheath);
	DDX_Control(pDX, IDC_ISOL, Isol);
	DDX_Control(pDX, IDC_SPARE, Spare);
	DDX_Control(pDX, IDC_FANS, Fans);
	DDX_Control(pDX, IDC_LASER, Laser);
	DDX_Control(pDX, IDC_CAMERA, Camera);
	DDX_Control(pDX, IDC_STIRRER, Stirrer);
	DDX_Control(pDX, IDC_PUMP1, Pump1);
	DDX_Control(pDX, IDC_PUMP2, Pump2);
	DDX_Control(pDX, IDC_FAN_AUTO, FansAuto);
	DDX_Control(pDX, IDC_HUMID_AUTO, HumidAuto);
	DDX_Control(pDX, IDC_FAN_SET, FanSet);
	DDX_Control(pDX, IDC_CAMERA_LABEL, CameraLabel);
	DDX_Control(pDX, IDC_LASER_LABEL, LaserLabel);
	DDX_Control(pDX, IDC_ISOL_LABEL, IsolLabel);
	DDX_Control(pDX, IDC_SPARE_LABEL, SpareLabel);
	DDX_Control(pDX, IDC_FAN_LABEL, FanLabel);
	DDX_Control(pDX, IDC_FAN_THRESH_LABEL, FanThreshLabel);
	DDX_Control(pDX, IDC_HUMID_THRESH_LABEL, HumidThreshLabel);
	DDX_Control(pDX, IDC_READBACK, Readback);
	DDX_Control(pDX, IDC_DACS_ON, DacsOn);
	DDX_Control(pDX, IDC_RESET_TRIG, ResetTrig);
	DDX_Control(pDX, IDC_FORCE_TRIG_NO_CAMERA, ForceTrigNoCam);
	DDX_Control(pDX, IDC_POLL_ADCS, PollADCs);
	DDX_Control(pDX, IDC_FILL1microinjector_LABEL, Fill1microinjectorLabel);
	DDX_Control(pDX, IDC_FILL2bypassFilter_LABEL, Fill2bypassFilterLabel);
	DDX_Control(pDX, IDC_FILL3newSheath_LABEL, Fill3newSheathLabel);
	DDX_Control(pDX, IDC_VALVE_GROUP, ValveGroup);
	DDX_Control(pDX, IDC_STIRRER_LABEL, StirrerLabel);
	DDX_Control(pDX, IDC_PUMP1_LABEL, Pump1Label);
	DDX_Control(pDX, IDC_PUMP2_LABEL, Pump2Label);
	DDX_Control(pDX, IDC_SWITCH_GROUP, SwitchGroup);
	DDX_Control(pDX, IDC_FOCUS_MIRROR, FocusMotorButt);
	DDX_Control(pDX, IDC_FLASHLAMP, Flashlamp);
#ifdef DAQ_MCC
	DDX_Control(pDX, IDC_TRIG_THRESHA, TrigThreshA);
	DDX_Control(pDX, IDC_TRIG_THRESHB, TrigThreshB);
	DDX_Control(pDX, IDC_TRIG_THRESHC, TrigThreshC);
	DDX_Control(pDX, IDC_TRIG_ENA, TrigEnA);
	DDX_Control(pDX, IDC_TRIG_ENB, TrigEnB);
	DDX_Control(pDX, IDC_TRIG_ENC, TrigEnC);
	DDX_Control(pDX, IDC_PGA_COMBO, PgaCombo);
#endif
	DDX_Control(pDX, IDC_PUMP_VOLTS, PumpVolts);
	DDX_Control(pDX, IDC_PUMP_VOLTS_LABEL, PumpVoltsLabel);
	DDX_Control(pDX, IDC_PUMP_SET, PumpVoltsSet);
	DDX_Control(pDX, IDC_ALT_CONFIG, AltConfig);
	DDX_Control(pDX, IDC_PMTA_SP2, PmtADacSet2);
	DDX_Control(pDX, IDC_PMTB_SP2, PmtBDacSet2);
	DDX_Control(pDX, IDC_PMTC_SP2, PmtCDacSet2);
	DDX_Control(pDX, IDC_TRIG_THRESHA2, TrigThreshA2);
	DDX_Control(pDX, IDC_TRIG_THRESHB2, TrigThreshB2);
	DDX_Control(pDX, IDC_TRIG_THRESHC2, TrigThreshC2);
	DDX_Control(pDX, IDC_TRIG_ENA2, TrigEnA2);
	DDX_Control(pDX, IDC_TRIG_ENB2, TrigEnB2);
	DDX_Control(pDX, IDC_TRIG_ENC2, TrigEnC2);
	DDX_Control(pDX, IDC_PGA_COMBO2, PgaCombo2);
	DDX_Control(pDX, IDC_ALT_LABEL1, PmtA2Label);
	DDX_Control(pDX, IDC_ALT_LABEL2, PmtB2Label);
	DDX_Control(pDX, IDC_ALT_LABEL3, PmtC2Label);
	DDX_Control(pDX, IDC_ALT_LABEL4, Pga2Label);
	DDX_Control(pDX, IDC_PMTA_SET2, PmtA2Set);
	DDX_Control(pDX, IDC_PMTB_SET2, PmtB2Set);
	DDX_Control(pDX, IDC_PMTC_SET2, PmtC2Set);
	DDX_Control(pDX, IDC_ALT_LABEL5, TrigA2Label);
	DDX_Control(pDX, IDC_ALT_LABEL6, TrigB2Label);
	DDX_Control(pDX, IDC_ALT_LABEL7, TrigC2Label);
	DDX_Control(pDX, IDC_TRIG_SETA2, TrigA2Set);
	DDX_Control(pDX, IDC_TRIG_SETB2, TrigB2Set);
	DDX_Control(pDX, IDC_TRIG_SETC2, TrigC2Set);
	DDX_Control(pDX, IDC_ALT_GROUP, AltGroup);
}

//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CHardwareTab, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_PMTA_SET, &CHardwareTab::OnBnClickedPmtASet)
	ON_BN_CLICKED(IDC_PMTB_SET, &CHardwareTab::OnBnClickedPmtBSet)
	ON_BN_CLICKED(IDC_PMTC_SET, &CHardwareTab::OnBnClickedPmtCSet)
	ON_BN_CLICKED(IDC_FLASH_SET, &CHardwareTab::OnBnClickedFlashSet)
	ON_BN_CLICKED(IDC_FLASH_SET2, &CHardwareTab::OnBnClickedFlashSet2)
	ON_BN_CLICKED(IDC_FAN_SET, &CHardwareTab::OnBnClickedFanSet)
	ON_BN_CLICKED(IDC_HUMID_SET, &CHardwareTab::OnBnClickedHumidSet)
	ON_BN_CLICKED(IDC_FILL1microinjector, &CHardwareTab::OnBnClickedFill1microinjector)
	ON_BN_CLICKED(IDC_FILL2bypassFilter, &CHardwareTab::OnBnClickedFill2bypassFilter)
	ON_BN_CLICKED(IDC_FILL3newSheath, &CHardwareTab::OnBnClickedFill3newSheath)
	ON_BN_CLICKED(IDC_ISOL, &CHardwareTab::OnBnClickedIsol)
	ON_BN_CLICKED(IDC_SPARE, &CHardwareTab::OnBnClickedSpare)
	ON_BN_CLICKED(IDC_FANS, &CHardwareTab::OnBnClickedFans)
	ON_BN_CLICKED(IDC_LASER, &CHardwareTab::OnBnClickedLaser)
	ON_BN_CLICKED(IDC_CAMERA, &CHardwareTab::OnBnClickedCamera)
	ON_BN_CLICKED(IDC_STIRRER, &CHardwareTab::OnBnClickedStirrer)
	ON_BN_CLICKED(IDC_PUMP1, &CHardwareTab::OnBnClickedPump1)
	ON_BN_CLICKED(IDC_PUMP2, &CHardwareTab::OnBnClickedPump2)
	ON_BN_CLICKED(IDC_FAN_AUTO, &CHardwareTab::OnBnClickedFanAuto)
	ON_BN_CLICKED(IDC_HUMID_AUTO, &CHardwareTab::OnBnClickedHumidAuto)
	ON_BN_CLICKED(IDC_READBACK, &CHardwareTab::OnBnClickedReadback)
	ON_BN_CLICKED(IDC_LIVE, &CHardwareTab::OnBnClickedLive)
	ON_BN_CLICKED(IDC_DACS_ON, &CHardwareTab::OnBnClickedDacsOn)
	ON_BN_CLICKED(IDC_RESET_TRIG, &CHardwareTab::OnBnClickedResetTrig)
	ON_BN_CLICKED(IDC_FORCE_TRIG_NO_CAMERA, &CHardwareTab::OnBnClickedForceTrigNoCam)
	ON_BN_CLICKED(IDC_POLL_ADCS, &CHardwareTab::OnBnClickedPollADCs)
	ON_BN_CLICKED(IDC_FOCUS_MIRROR, &CHardwareTab::OnBnClickedFocusMotor)
	ON_BN_CLICKED(IDC_FLASHLAMP, &CHardwareTab::OnBnClickedFlashlamp)
#ifdef DAQ_MCC
	ON_BN_CLICKED(IDC_TRIG_SETA, &CHardwareTab::OnBnClickedTrigSeta)
	ON_BN_CLICKED(IDC_TRIG_SETB, &CHardwareTab::OnBnClickedTrigSetb)
	ON_BN_CLICKED(IDC_TRIG_SETC, &CHardwareTab::OnBnClickedTrigSetc)
	ON_BN_CLICKED(IDC_TRIG_ENA, &CHardwareTab::OnBnClickedEna)
	ON_BN_CLICKED(IDC_TRIG_ENB, &CHardwareTab::OnBnClickedEnb)
	ON_BN_CLICKED(IDC_TRIG_ENC, &CHardwareTab::OnBnClickedEnc)
	ON_CBN_SELCHANGE(IDC_PGA_COMBO, &CHardwareTab::OnCbnSelchangePgaCombo)
#endif
	ON_BN_CLICKED(IDC_PUMP_SET, &CHardwareTab::OnBnClickedPumpSet)
	ON_BN_CLICKED(IDC_ALT_CFG_SET, &CHardwareTab::OnBnClickedAltCfgSet)
	ON_BN_CLICKED(IDC_PMTA_SET2, &CHardwareTab::OnBnClickedPmtaSet2)
	ON_BN_CLICKED(IDC_PMTB_SET2, &CHardwareTab::OnBnClickedPmtbSet2)
	ON_BN_CLICKED(IDC_PMTC_SET2, &CHardwareTab::OnBnClickedPmtcSet2)
	ON_BN_CLICKED(IDC_TRIG_SETA2, &CHardwareTab::OnBnClickedTrigSeta2)
	ON_BN_CLICKED(IDC_TRIG_SETB2, &CHardwareTab::OnBnClickedTrigSetb2)
	ON_BN_CLICKED(IDC_TRIG_SETC2, &CHardwareTab::OnBnClickedTrigSetc2)
	ON_BN_CLICKED(IDC_TRIG_ENA2, &CHardwareTab::OnBnClickedTrigEna2)
	ON_BN_CLICKED(IDC_TRIG_ENB2, &CHardwareTab::OnBnClickedTrigEnb2)
	ON_BN_CLICKED(IDC_TRIG_ENC2, &CHardwareTab::OnBnClickedTrigEnc2)
	ON_CBN_SELCHANGE(IDC_PGA_COMBO2, &CHardwareTab::OnCbnSelchangePgaCombo2)
	ON_BN_CLICKED(IDC_LASER_MOTOR, &CHardwareTab::OnBnClickedLaserMotor)
	ON_STN_CLICKED(IDC_FILL3newSheath_LABEL, &CHardwareTab::OnStnClickedFill3newSheathLabel)
	ON_EN_CHANGE(IDC_ALT_CONFIG, &CHardwareTab::OnEnChangeAltConfig)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//	Handles the DACs ON/OFF button
//-----------------------------------------------------------------------------
bool CHardwareTab::Run(bool run) {

	if (IfcbHandle->dacsOn) {
        DacsOn.SetWindowText(_T("All ON"));
		DacsOn.SetCheck(BST_CHECKED);
	} else {
        DacsOn.SetWindowText(_T("All OFF"));
		DacsOn.SetCheck(BST_UNCHECKED);
	}

	return true;
}

//-----------------------------------------------------------------------------
void CHardwareTab::UpdateData() {

	CString valstr;

#ifdef DAQ_MCC
	ReadHousekeeping();	
#endif

	if (altInterval) {
		PmtADacSet2.ShowWindow(true);
		PmtBDacSet2.ShowWindow(true);
		PmtCDacSet2.ShowWindow(true);
		PmtA2Label.ShowWindow(true);
		PmtB2Label.ShowWindow(true);
		PmtC2Label.ShowWindow(true);
		Pga2Label.ShowWindow(true);
		PgaCombo2.ShowWindow(true);
		PmtA2Set.ShowWindow(true);
		PmtB2Set.ShowWindow(true);
		PmtC2Set.ShowWindow(true);
		TrigA2Label.ShowWindow(true);
		TrigB2Label.ShowWindow(true);
		TrigC2Label.ShowWindow(true);
		TrigThreshA2.ShowWindow(true);
		TrigThreshB2.ShowWindow(true);
		TrigThreshC2.ShowWindow(true);
		TrigA2Set.ShowWindow(true);
		TrigB2Set.ShowWindow(true);
		TrigC2Set.ShowWindow(true);
		TrigEnA2.ShowWindow(true);
		TrigEnB2.ShowWindow(true);
		TrigEnC2.ShowWindow(true);
		AltGroup.ShowWindow(true);
	} else {
		PmtADacSet2.ShowWindow(false);
		PmtBDacSet2.ShowWindow(false);
		PmtCDacSet2.ShowWindow(false);
		PmtA2Label.ShowWindow(false);
		PmtB2Label.ShowWindow(false);
		PmtC2Label.ShowWindow(false);
		Pga2Label.ShowWindow(false);
		PgaCombo2.ShowWindow(false);
		PmtA2Set.ShowWindow(false);
		PmtB2Set.ShowWindow(false);
		PmtC2Set.ShowWindow(false);
		TrigA2Label.ShowWindow(false);
		TrigB2Label.ShowWindow(false);
		TrigC2Label.ShowWindow(false);
		TrigThreshA2.ShowWindow(false);
		TrigThreshB2.ShowWindow(false);
		TrigThreshC2.ShowWindow(false);
		TrigA2Set.ShowWindow(false);
		TrigB2Set.ShowWindow(false);
		TrigC2Set.ShowWindow(false);
		TrigEnA2.ShowWindow(false);
		TrigEnB2.ShowWindow(false);
		TrigEnC2.ShowWindow(false);
		AltGroup.ShowWindow(false);
	}

	valstr.Format(_T("%.2f"), highVoltagePMTA);			// PMTA dac
	PmtADacSet.SetSel(0, -1);
	PmtADacSet.ReplaceSel(valstr, false);
	valstr.Format(_T("%.2f"), highVoltagePMTB);			// PMTB dac
	PmtBDacSet.SetSel(0, -1);
	PmtBDacSet.ReplaceSel(valstr, false);
	valstr.Format(_T("%.2f"), highVoltagePMTC);			// PMTC dac
	PmtCDacSet.SetSel(0, -1);
	PmtCDacSet.ReplaceSel(valstr, false);
	if (IfcbHandle->dacsOn) {
        DacsOn.SetWindowText(_T("All ON"));
		DacsOn.SetCheck(BST_CHECKED);
	} else {
        DacsOn.SetWindowText(_T("All OFF"));
		DacsOn.SetCheck(BST_UNCHECKED);
	}
	valstr.Format(_T("%.2f"), highVoltagePMTA2);		// alt PMTA dac
	PmtADacSet2.SetSel(0, -1);
	PmtADacSet2.ReplaceSel(valstr, false);
	valstr.Format(_T("%.2f"), highVoltagePMTB2);		// alt PMTB dac
	PmtBDacSet2.SetSel(0, -1);
	PmtBDacSet2.ReplaceSel(valstr, false);
	valstr.Format(_T("%.2f"), highVoltagePMTC2);		// alt PMTC dac
	PmtCDacSet2.SetSel(0, -1);
	PmtCDacSet2.ReplaceSel(valstr, false);
	valstr.Format(_T("%.2f"), flashVoltage);			// flash control dac
	FlashDacSet.SetSel(0, -1);
	FlashDacSet.ReplaceSel(valstr, false);
	valstr.Format(_T("%.2f"), flashVoltage2);			// alt flash control dac
	FlashDacSet2.SetSel(0, -1);
	FlashDacSet2.ReplaceSel(valstr, false);
#ifdef DAQ_MCC
	valstr.Format(_T("%0.3f"), trigThreshA);			// trigger A thresh dac
	TrigThreshA.SetSel(0, -1);
	TrigThreshA.ReplaceSel(valstr, false);
	valstr.Format(_T("%0.3f"), trigThreshB);			// trigger B thresh dac
	TrigThreshB.SetSel(0, -1);
	TrigThreshB.ReplaceSel(valstr, false);
	valstr.Format(_T("%0.3f"), trigThreshC);			// trigger C thresh dac
	TrigThreshC.SetSel(0, -1);
	TrigThreshC.ReplaceSel(valstr, false);
	if (triggerPulseMask & 1) {							// trigger A enable
		TrigEnA.SetCheck(BST_CHECKED);
        TrigEnA.SetWindowText(_T("ON"));
	} else {
		TrigEnA.SetCheck(BST_UNCHECKED);
        TrigEnA.SetWindowText(_T("OFF"));
	}
	if (triggerPulseMask & 2) {							// trigger B enable
		TrigEnB.SetCheck(BST_CHECKED);
        TrigEnB.SetWindowText(_T("ON"));
	} else {
		TrigEnB.SetCheck(BST_UNCHECKED);
        TrigEnB.SetWindowText(_T("OFF"));
	}
	if (triggerPulseMask & 4) {							// trigger C enable
		TrigEnC.SetCheck(BST_CHECKED);
        TrigEnC.SetWindowText(_T("ON"));
	} else {
		TrigEnC.SetCheck(BST_UNCHECKED);
        TrigEnC.SetWindowText(_T("OFF"));
	}
	valstr.Format(_T("%0.3f"), trigThreshA2);			// alt trigger A thresh dac
	TrigThreshA2.SetSel(0, -1);
	TrigThreshA2.ReplaceSel(valstr, false);
	valstr.Format(_T("%0.3f"), trigThreshB2);			// lat trigger B thresh dac
	TrigThreshB2.SetSel(0, -1);
	TrigThreshB2.ReplaceSel(valstr, false);
	valstr.Format(_T("%0.3f"), trigThreshC2);			// alt trigger C thresh dac
	TrigThreshC2.SetSel(0, -1);
	TrigThreshC2.ReplaceSel(valstr, false);
	if (triggerPulseMask2 & 1) {						// alt trigger A enable
		TrigEnA2.SetCheck(BST_CHECKED);
        TrigEnA2.SetWindowText(_T("ON"));
	} else {
		TrigEnA2.SetCheck(BST_UNCHECKED);
        TrigEnA2.SetWindowText(_T("OFF"));
	}
	if (triggerPulseMask2 & 2) {						// alt trigger B enable
		TrigEnB2.SetCheck(BST_CHECKED);
        TrigEnB2.SetWindowText(_T("ON"));
	} else {
		TrigEnB2.SetCheck(BST_UNCHECKED);
        TrigEnB2.SetWindowText(_T("OFF"));
	}
	if (triggerPulseMask2 & 4) {						// alt trigger C enable
		TrigEnC2.SetCheck(BST_CHECKED);
        TrigEnC2.SetWindowText(_T("ON"));
	} else {
		TrigEnC2.SetCheck(BST_UNCHECKED);
        TrigEnC2.SetWindowText(_T("OFF"));
	}

	valstr.Format(_T("%d"), fanThresh);					// fans/temperature thresh dac
	FanThresh.SetSel(0, -1);
	FanThresh.ReplaceSel(valstr, false);
	valstr.Format(_T("%d"), humidThresh);				// humidity/valves thresh dac
	HumidThresh.SetSel(0, -1);
	HumidThresh.ReplaceSel(valstr, false);
	CString pga;										// PGA
	pga.Format(_T("%d"), adcPga);
	PgaCombo.SetCurSel(PgaCombo.FindStringExact(-1, pga));
	pga.Format(_T("%d"), adcPga2);
	PgaCombo2.SetCurSel(PgaCombo.FindStringExact(-1, pga));

	valstr.Format(_T("%.1f"), Temperature);				// temperature
	TempAdc.SetSel(0, -1, false);
	TempAdc.ReplaceSel(valstr, false);

	valstr.Format(_T("%.1f"), Humidity);				// humidity
	HumidAdc.SetSel(0, -1, false);
	HumidAdc.ReplaceSel(valstr, false);

	if (pumpDrive) {
		valstr.Format(_T("%.1f"), pumpVolts);			// pump drive voltage
		PumpVolts.SetSel(0, -1);
		PumpVolts.ReplaceSel(valstr, false);
	}

	SetDlgItemInt(IDC_ALT_CONFIG, altInterval, FALSE);	// alt interval

#endif

	// update flags values
	Fill1microinjector.SetCheck(hardware[FILL1microinjector].value.i ? BST_CHECKED : BST_UNCHECKED);
	Fill1microinjector.SetWindowText(hardware[FILL1microinjector].value.i ? _T("ON") : _T("OFF"));
	Fill2bypassFilter.SetCheck(hardware[FILL2bypassFilter].value.i ? BST_CHECKED : BST_UNCHECKED);
	Fill2bypassFilter.SetWindowText(hardware[FILL2bypassFilter].value.i ? _T("ON") : _T("OFF"));
	Fill3newSheath.SetCheck(hardware[FILL3newSheath].value.i ? BST_CHECKED : BST_UNCHECKED);
	Fill3newSheath.SetWindowText(hardware[FILL3newSheath].value.i ? _T("ON") : _T("OFF"));
	Stirrer.SetCheck(hardware[STIRRER].value.i ? BST_CHECKED : BST_UNCHECKED);
	Stirrer.SetWindowText(hardware[STIRRER].value.i ? _T("ON") : _T("OFF"));
	Pump1.SetCheck(hardware[PUMP1].value.i ? BST_CHECKED : BST_UNCHECKED);
	Pump1.SetWindowText(hardware[PUMP1].value.i ? _T("ON") : _T("OFF"));
	Pump2.SetCheck(hardware[PUMP2].value.i ? BST_CHECKED : BST_UNCHECKED);
	Pump2.SetWindowText(hardware[PUMP2].value.i ? _T("ON") : _T("OFF"));
#ifdef DAQ_MCC
	FansAuto.SetCheck(hardware[FANS].value.i == AUTO ? BST_CHECKED : BST_UNCHECKED);
	HumidAuto.SetCheck(hardware[HUMID_AUTO].value.i ? BST_CHECKED : BST_UNCHECKED);
	Isol.SetCheck(hardware[ISOL].value.i ? BST_CHECKED : BST_UNCHECKED);
	Isol.SetWindowText(hardware[ISOL].value.i ? _T("ON") : _T("OFF"));
	Spare.SetCheck(hardware[SPARE].value.i ? BST_CHECKED : BST_UNCHECKED);
	Spare.SetWindowText(hardware[SPARE].value.i ? _T("ON") : _T("OFF"));
	Fans.SetCheck(hardware[FANS].value.i ? BST_CHECKED : BST_UNCHECKED);
	Fans.SetWindowText(hardware[FANS].value.i ? _T("ON") : _T("OFF"));
	Laser.SetCheck(hardware[LASER].value.i ? BST_CHECKED : BST_UNCHECKED);
	Laser.SetWindowText(hardware[LASER].value.i ? _T("ON") : _T("OFF"));
	Camera.SetCheck(hardware[CAMERA].value.i ? BST_CHECKED : BST_UNCHECKED);
	Camera.SetWindowText(hardware[CAMERA].value.i ? _T("ON") : _T("OFF"));
#endif
}

//-----------------------------------------------------------------------------
BOOL CHardwareTab::OnInitDialog() {

    CDialog::OnInitDialog();

#ifndef DAQ_MCC
	// DACs Group
	TrigLabel.ShowWindow(false);
	FlashLabel.ShowWindow(false);

	// Switches Group
	SwitchGroup.ShowWindow(false);
	Stirrer.ShowWindow(false);
	StirrerLabel.ShowWindow(false);
	Pump1.ShowWindow(false);
	Pump1Label.ShowWindow(false);
	Pump2.ShowWindow(false);
	Pump2Label.ShowWindow(false);
	PumpVolts.ShowWindow(false);
	PumpVoltsLabel.ShowWindow(false);
	PumpVoltsSet.ShowWindow(false);
	Laser.ShowWindow(false);
	LaserLabel.ShowWindow(false);
	Camera.ShowWindow(false);
	CameraLabel.ShowWindow(false);
	Flashlamp.ShowWindow(false);
#endif

	// Valve Group - comment out these lines to restore the buttons
	//ValveGroup.ShowWindow(false);
	//Fill1microinjector.ShowWindow(false);
	//Fill1microinjectorLabel.ShowWindow(false);
	Fill2bypassFilter.ShowWindow(false);
	Fill2bypassFilterLabel.ShowWindow(false);
	//Fill3newSheath.ShowWindow(false);
	//Fill3newSheathLabel.ShowWindow(false);
	Isol.ShowWindow(false);
	IsolLabel.ShowWindow(false);
	Spare.ShowWindow(false);
	SpareLabel.ShowWindow(false);
	Fans.ShowWindow(false);
	FanLabel.ShowWindow(false);

	// Environment Group - comment out these lines to restore the buttons
	//FanThresh.ShowWindow(false);
	//FansAuto.ShowWindow(false);
	//FanSet.ShowWindow(false);
	//FanThreshLabel.ShowWindow(false);
	//HumidThresh.ShowWindow(false);
	//HumidAuto.ShowWindow(false);
	//HumidSet.ShowWindow(false);
	//HumidThreshLabel.ShowWindow(false);

	// miscellaneous
	//FocusMotorButt.ShowWindow(false);
#ifdef DAQ_MCC
	ResetTrig.ShowWindow(false);
	ForceTrigNoCam.ShowWindow(false);
	PollADCs.ShowWindow(false);
	Flashlamp.SetCheck(BST_CHECKED);
	SetTimer(HOUSEKEEPING_TIMER_ID, 300, NULL); 
	if (!pumpDrive) {
		PumpVolts.ShowWindow(false);
		PumpVoltsLabel.ShowWindow(false);
		PumpVoltsSet.ShowWindow(false);
	}
#endif

	return TRUE;
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnShowTab() {

	CTab::OnShowTab();
	UpdateData();
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnTimer(UINT_PTR nIDEvent) {

	if (nIDEvent != HOUSEKEEPING_TIMER_ID )
		return;

	if (LiveReadback.GetCheck() == BST_CHECKED)
		UpdateData();
	
	if ((GraphData.GraphType == GraphData.GRAPH_ROI_ADC) && false)	{								// graph test stuff
		GraphData.data[0][GraphData.nextIn] = 1.0 + 1.5 * (double)rand() / RAND_MAX;		// x data from ReadDAQIntegrated()
		GraphData.data[1][GraphData.nextIn] = 3.0 + 1.5 * (double)rand() / RAND_MAX;		// x data from ReadDAQIntegrated()
		GraphData.data[2][GraphData.nextIn] = 5.0 + 1.5 * (double)rand() / RAND_MAX;		// x data from ReadDAQIntegrated()
		GraphData.data[3][GraphData.nextIn] = 8.0 + 1.5 * (double)rand() / RAND_MAX;		// x data from ReadDAQIntegrated()

		GraphData.data[0][GraphData.nextIn + GraphData.nPoints] = 800.0 + 150.0 * (double)rand() / RAND_MAX;		// y data from Analysis()
		GraphData.data[1][GraphData.nextIn + GraphData.nPoints] = 300.0 + 150.0 * (double)rand() / RAND_MAX;		// y data from Analysis()
		GraphData.data[2][GraphData.nextIn + GraphData.nPoints] = 500.0 + 150.0 * (double)rand() / RAND_MAX;		// y data from Analysis()
		GraphData.data[3][GraphData.nextIn + GraphData.nPoints] = 400.0 + 150.0 * (double)rand() / RAND_MAX;		// y data from Analysis()

		IfcbHandle->TabContainer.tabPages[graphTab]->UpdateGraph();
	}
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedPmtASet() {

	CString valstr;

	PmtADacSet.GetWindowTextW(valstr);						// get the voltage string

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &highVoltagePMTA);
	WriteCfgFile();
	if (IfcbHandle->dacsOn)
		ChangeHWParam(PMTA, highVoltagePMTA);				// change the value
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedPmtaSet2() {

	CString valstr;

	PmtADacSet2.GetWindowTextW(valstr);						// get the voltage string

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &highVoltagePMTA2);
	WriteCfgFile();
			
		if (IfcbHandle->dacsOn) //Rob add these 2 lines 6Dec2013
		ChangeHWParam(PMTA, highVoltagePMTA2);				// change the value

}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedPmtBSet() {

	CString valstr;

	PmtBDacSet.GetWindowTextW(valstr);						// get the voltage string

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &highVoltagePMTB);
	WriteCfgFile();
	if (IfcbHandle->dacsOn)
		ChangeHWParam(PMTB, highVoltagePMTB);				// change the value
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedPmtbSet2() {

	CString valstr;

	PmtBDacSet2.GetWindowTextW(valstr);						// get the voltage string

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &highVoltagePMTB2);
	WriteCfgFile();
		if (IfcbHandle->dacsOn) //Rob add these 2 lines 6Dec2013
		ChangeHWParam(PMTB, highVoltagePMTB2);				// change the value

}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedPmtCSet() {

	CString valstr;

	PmtCDacSet.GetWindowTextW(valstr);						// get the voltage string

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &highVoltagePMTC);
	WriteCfgFile();
	if (IfcbHandle->dacsOn)
		ChangeHWParam(PMTC, highVoltagePMTC);				// change the value
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedPmtcSet2() {

	CString valstr;

	PmtCDacSet2.GetWindowTextW(valstr);						// get the voltage string

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &highVoltagePMTC2);
	WriteCfgFile();

		if (IfcbHandle->dacsOn) //Rob add these 2 lines 6Dec2013
		ChangeHWParam(PMTC, highVoltagePMTC2);				// change the value

}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedFlashSet() {

	CString valstr;

	FlashDacSet.GetWindowTextW(valstr);						// get the voltage string

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &flashVoltage);
	WriteCfgFile();
	ChangeHWParam(FLASH_DAC, flashVoltage);
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedFlashSet2() {

	CString valstr;

	FlashDacSet2.GetWindowTextW(valstr);						// get the voltage string

	char cval[20];
	CStringToChar(cval, &valstr);
	if ((StainAuto && runType == ALT) | stainSample)	{
		sscanf_s(cval, " %lf", &flashVoltage2);
		WriteCfgFile();
		ChangeHWParam(FLASH_DAC, flashVoltage2);
	}
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedFanSet() {

#ifdef DAQ_MCC
/*	CString valstr;
	char cval[20];
	FanThresh.GetWindowTextW(valstr);
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %d", &PZTfrequency);
	CString args, str, args2;		
	args.Format(_T(" --nogui --on --hz %d --volts %d"), PZTfrequency, PZTvolts);
	DEBUG_MESSAGE_EXT(args); 
	DEBUG_MESSAGE_EXT(_T("\r\n"));
	str = sigGenPath; 
	DEBUG_MESSAGE_EXT(str); 
	DEBUG_MESSAGE_EXT(_T("\r\n"));
	ShellExecute(NULL, _T("open"), str, args, NULL, SW_HIDE);			// run sig-gen with the appropriate arguments	*/

	if (AcousticFocusing) {
	}
#endif
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedHumidSet() {

#ifdef DAQ_MCC
	CString valstr;

	HumidThresh.GetWindowTextW(valstr);

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %d", &humidThresh);
	WriteCfgFile();
	ChangeHWParam(HUMID_THRESH, humidThresh);
#endif
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedFill1microinjector() { //now used for Dye to Loop -- 24V to solenoids 1,2

	if (Fill1microinjector.GetCheck() == BST_CHECKED) {
        Fill1microinjector.SetWindowText(_T("ON"));
		ChangeHWParam(FILL1microinjector, ON);
		Sleep(100);
        Fill1microinjector.SetWindowText(_T("OFF"));
		ChangeHWParam(FILL1microinjector, OFF);
	} else {
		Fill1microinjector.SetWindowText( _T("OFF") );
		ChangeHWParam(FILL1microinjector, OFF);
	}
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedFill2bypassFilter() { // solenoid valve (N.O.) to bypass cartridge filter

	if (Fill2bypassFilter.GetCheck() == BST_CHECKED) {
        Fill2bypassFilter.SetWindowText(_T("ON"));
		ChangeHWParam(FILL2bypassFilter, ON);
		fill2bypassFilterState |= 1;
	} else {
		Fill2bypassFilter.SetWindowText( _T("OFF") );
		ChangeHWParam(FILL2bypassFilter, OFF);
		fill2bypassFilterState &= ~1;
	}
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedFill3newSheath() {// solenoid valve (N.C.) to let in new SW for sheath

	if (Fill3newSheath.GetCheck() == BST_CHECKED) {
        Fill3newSheath.SetWindowText(_T("ON"));
		ChangeHWParam(FILL3newSheath, ON); // solenoid valve (N.C.) to let in new SW for sheath
		fill3newSheathState |= 1;
		ChangeHWParam(FILL2bypassFilter, ON); // solenoid valve (N.O.) to bypass cartridge filter after flow cell
		fill2bypassFilterState |= 1;

	} else {
		Fill3newSheath.SetWindowText( _T("OFF") );
		ChangeHWParam(FILL3newSheath, OFF); // solenoid valve (N.C.) to let in new SW for sheath
		fill3newSheathState &= ~1;
		ChangeHWParam(FILL2bypassFilter, OFF); // solenoid valve (N.O.) to bypass cartridge filter after flow cell
		fill2bypassFilterState &= ~1;
	}
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedIsol() {

	HumidAuto.SetCheck(BST_UNCHECKED);					// flip to manual

	if (Isol.GetCheck() == BST_CHECKED) {
        Isol.SetWindowText(_T("ON"));
		ChangeHWParam(ISOL, ON);
	} else {
		Isol.SetWindowText( _T("OFF") );
		ChangeHWParam(ISOL, OFF);
	}
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedSpare() {

	HumidAuto.SetCheck(BST_UNCHECKED);					// flip to manual
	ChangeHWParam(HUMID_AUTO, OFF);

	if (Spare.GetCheck() == BST_CHECKED) {
        Spare.SetWindowText(_T("ON"));
		ChangeHWParam(SPARE, ON);
	} else {
		Spare.SetWindowText( _T("OFF") );
		ChangeHWParam(SPARE, OFF);
	}
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedFans() {

	FansAuto.SetCheck(BST_UNCHECKED);					// flip to manual

	if (Fans.GetCheck() == BST_CHECKED) {
        Fans.SetWindowText(_T("ON"));
		ChangeHWParam(FANS, ON);
	} else {
		Fans.SetWindowText( _T("OFF") );
		ChangeHWParam(FANS, OFF);
	}
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedLaser() {

	if (Laser.GetCheck() == BST_CHECKED) {
        Laser.SetWindowText(_T("ON"));
		ChangeHWParam(LASER, ON);
		laserState |= 1;
	} else {
		Laser.SetWindowText( _T("OFF") );
		ChangeHWParam(LASER, OFF);
		laserState &= ~1;
	}
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedCamera() {

	if (Camera.GetCheck() == BST_CHECKED) {
        Camera.SetWindowText(_T("ON"));
		ChangeHWParam(CAMERA, ON);
	} else {
		Camera.SetWindowText( _T("OFF") );
		ChangeHWParam(CAMERA, OFF);
	}
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedStirrer() {

	if (Stirrer.GetCheck() == BST_CHECKED) {
        Stirrer.SetWindowText(_T("ON"));
		ChangeHWParam(STIRRER, ON);
	} else {
		Stirrer.SetWindowText( _T("OFF") );
		ChangeHWParam(STIRRER, OFF);
	}
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedPump1() {

	if (Pump1.GetCheck() == BST_CHECKED) {
        Pump1.SetWindowText(_T("ON"));
		ChangeHWParam(PUMP1, ON);
		pumpState |= 1;
	} else {
		Pump1.SetWindowText( _T("OFF") );
		ChangeHWParam(PUMP1, OFF);
		pumpState &= ~1;
	}
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedPump2() {

	if (Pump2.GetCheck() == BST_CHECKED) {
        Pump2.SetWindowText(_T("ON"));
		ChangeHWParam(PUMP2, ON);
		pumpState |= 2;
	} else {
		Pump2.SetWindowText( _T("OFF") );
		ChangeHWParam(PUMP2, OFF);
		pumpState &= ~2;
	}
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedFanAuto() {

	ChangeHWParam(FANS, FansAuto.GetCheck() == BST_CHECKED ? (uint16)2 : (uint16)0);
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedHumidAuto() {

	ChangeHWParam(ISOL, HumidAuto.GetCheck() == BST_CHECKED ? (uint16)2 : (uint16)0);
	ChangeHWParam(SPARE, HumidAuto.GetCheck() == BST_CHECKED ? (uint16)2 : (uint16)0);
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedReadback() {

	UpdateData();
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedLive() {

	if (LiveReadback.GetCheck() == BST_CHECKED)
		Readback.SetCheck(BST_CHECKED);
	else
		Readback.SetCheck(BST_UNCHECKED);
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedDacsOn() {

	if (DacsOn.GetCheck() == BST_CHECKED) {
        DacsOn.SetWindowText(_T("All ON"));
		ChangeHWParam(PMTA, highVoltagePMTA);
		ChangeHWParam(PMTB, highVoltagePMTB);
		ChangeHWParam(PMTC, highVoltagePMTC);
		IfcbHandle->dacsOn = true;
	} else {
		DacsOn.SetWindowText(_T("All OFF"));
		ChangeHWParam(PMTA, 0.0);
		ChangeHWParam(PMTB, 0.0);
		ChangeHWParam(PMTC, 0.0);
		IfcbHandle->dacsOn = false;
	}
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedResetTrig() {

	DaqArmTrigger();
}

//-----------------------------------------------------------------------------
// This sends a hardware trigger signal to fire the camera, lamps, etc, no camera
//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedForceTrigNoCam() {

	GenerateTrigger();
}

//-----------------------------------------------------------------------------
// a debug for polling ADCs directly
//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedPollADCs() {

}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedFocusMotor() {

	FocusMotorControl = new CFocusMotorControl(this, CFocusMotorControl::FOCUS);
	FocusMotorControl->Create(IDD_FOCUS_MIRROR, this);
	FocusMotorControl->ShowWindow(SW_SHOW);
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedLaserMotor() {

	LaserMotorControl = new CFocusMotorControl(this, CFocusMotorControl::LASER);
	LaserMotorControl->Create(IDD_FOCUS_MIRROR, this);
	LaserMotorControl->ShowWindow(SW_SHOW);

}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedFlashlamp() {

	if (Flashlamp.GetCheck() == BST_CHECKED) {
        Flashlamp.SetWindowText(_T("ON"));
		ChangeHWParam(FLASH, ON);
	} else {
		Flashlamp.SetWindowText( _T("OFF") );
		ChangeHWParam(FLASH, OFF);
	}
}

#ifdef DAQ_MCC
//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedTrigSeta() {

	CString valstr;

	TrigThreshA.GetWindowTextW(valstr);						// get the voltage string

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &trigThreshA);
	ChangeHWParam(TRIGA, trigThreshA);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedTrigSeta2() {

	CString valstr;

	TrigThreshA2.GetWindowTextW(valstr);						// get the voltage string

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &trigThreshA2);
		ChangeHWParam(TRIGA, trigThreshA2);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedTrigSetb() {

	CString valstr;

	TrigThreshB.GetWindowTextW(valstr);						// get the voltage string

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &trigThreshB);
	ChangeHWParam(TRIGB, trigThreshB);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedTrigSetb2() {

	CString valstr;

	TrigThreshB2.GetWindowTextW(valstr);						// get the voltage string

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &trigThreshB2);
		ChangeHWParam(TRIGB, trigThreshB2);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedTrigSetc() {

	CString valstr;

	TrigThreshC.GetWindowTextW(valstr);						// get the voltage string

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &trigThreshC);
	ChangeHWParam(TRIGC, trigThreshC);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedTrigSetc2() {

	CString valstr;

	TrigThreshC2.GetWindowTextW(valstr);						// get the voltage string

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &trigThreshC2);
		ChangeHWParam(TRIGC, trigThreshC2);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedEna() {

	if (TrigEnA.GetCheck() == BST_CHECKED) {		// enable
        TrigEnA.SetWindowText(_T("ON"));
		triggerPulseMask |= 1;
	} else {										// disable
		TrigEnA.SetWindowText( _T("OFF") );
		triggerPulseMask &= ~1;
	}
	WriteCfgFile();
	if (runType == ALT)
		ChangeHWParam(TRIG_MASK, triggerPulseMask2);
	else
		ChangeHWParam(TRIG_MASK, triggerPulseMask);
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedTrigEna2() {

	if (TrigEnA2.GetCheck() == BST_CHECKED) {		// enable
        TrigEnA2.SetWindowText(_T("ON"));
		triggerPulseMask2 |= 1;
	} else {										// disable
		TrigEnA2.SetWindowText( _T("OFF") );
		triggerPulseMask2 &= ~1;
	}
	WriteCfgFile();
	if (runType == ALT)
		ChangeHWParam(TRIG_MASK, triggerPulseMask2);
	else
		ChangeHWParam(TRIG_MASK, triggerPulseMask);
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedEnb() {

	if (TrigEnB.GetCheck() == BST_CHECKED) {		// enable
        TrigEnB.SetWindowText(_T("ON"));
		triggerPulseMask |= 2;
	} else {										// disable
		TrigEnB.SetWindowText( _T("OFF") );
		triggerPulseMask &= ~2;
	}
	WriteCfgFile();
	if (runType == ALT)
		ChangeHWParam(TRIG_MASK, triggerPulseMask2);
	else
		ChangeHWParam(TRIG_MASK, triggerPulseMask);
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedTrigEnb2() {

	if (TrigEnB2.GetCheck() == BST_CHECKED) {		// enable
        TrigEnB2.SetWindowText(_T("ON"));
		triggerPulseMask2 |= 2;
	} else {										// disable
		TrigEnB2.SetWindowText( _T("OFF") );
		triggerPulseMask2 &= ~2;
	}
	WriteCfgFile();
	if (runType == ALT)
		ChangeHWParam(TRIG_MASK, triggerPulseMask2);
	else
		ChangeHWParam(TRIG_MASK, triggerPulseMask);
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedEnc() {

	if (TrigEnC.GetCheck() == BST_CHECKED) {		// enable
        TrigEnC.SetWindowText(_T("ON"));
		triggerPulseMask |= 4;
	} else {										// disable
		TrigEnC.SetWindowText( _T("OFF") );
		triggerPulseMask &= ~4;
	}
	WriteCfgFile();
	if (runType == ALT)
		ChangeHWParam(TRIG_MASK, triggerPulseMask2);
	else
		ChangeHWParam(TRIG_MASK, triggerPulseMask);
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedTrigEnc2() {

	if (TrigEnC2.GetCheck() == BST_CHECKED) {		// enable
        TrigEnC2.SetWindowText(_T("ON"));
		triggerPulseMask2 |= 4;
	} else {										// disable
		TrigEnC2.SetWindowText( _T("OFF") );
		triggerPulseMask2 &= ~4;
	}
	WriteCfgFile();
	if (runType == ALT)
		ChangeHWParam(TRIG_MASK, triggerPulseMask2);
	else
		ChangeHWParam(TRIG_MASK, triggerPulseMask);
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnCbnSelchangePgaCombo() {

	adcPga = 1 << PgaCombo.GetCurSel();
	WriteCfgFile();
	if (runType == ALT)
		ChangeHWParam(PGA, adcPga2);
	else
		ChangeHWParam(PGA, adcPga);
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnCbnSelchangePgaCombo2() {

	adcPga2 = 1 << PgaCombo2.GetCurSel();
	WriteCfgFile();
	if (runType == ALT)
		ChangeHWParam(PGA, adcPga2);
	else
		ChangeHWParam(PGA, adcPga);
}

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedPumpSet() {

	CString valstr;

	PumpVolts.GetWindowTextW(valstr);				// get the voltage string

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &pumpVolts);
	if ((pumpVolts < 4.5) || (pumpVolts > 24.0)) {
		if (pumpVolts < 4.5)
			pumpVolts = 4.5;
		if (pumpVolts > 24.0)
			pumpVolts = 24.0;
		valstr.Format(_T("%.1f"), pumpVolts);				// pump drive voltage
		PumpVolts.SetSel(0, -1);
		PumpVolts.ReplaceSel(valstr, false);
	}
	WriteCfgFile();
	ChangeHWParam(PUMP_DAC, pumpVolts);
}

#endif

//-----------------------------------------------------------------------------
void CHardwareTab::OnBnClickedAltCfgSet() {

	CString valstr;

	AltConfig.GetWindowTextW(valstr);						// get the number

	char cval[20];
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %d", &altInterval);
	AltConfig.SetSel(0, -1);
	AltConfig.ReplaceSel(valstr, false);

	UpdateData();
	WriteCfgFile();
}



void CHardwareTab::OnStnClickedFill3newSheathLabel()
{
	// TODO: Add your control notification handler code here
}

void CHardwareTab::OnEnChangeAltConfig()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CTab::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
