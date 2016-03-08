//-----------------------------------------------------------------------------
//  IFCB Project
//	FocusMotorControl.cpp : implementation file
//	Martin Cooper
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "FocusMotorControl.h"
#include "Daq.h"
#include "FileIo.h"

#include "FluidsWHOI.h"
#include "Fluids.h"

int			smallFocusStep;
int			largeFocusStep;
int			smallLaserStep;
int			largeLaserStep;

IMPLEMENT_DYNAMIC(CFocusMotorControl, CDialog)

//-----------------------------------------------------------------------------
CFocusMotorControl::CFocusMotorControl(CWnd* pParent, MOTOR_TYPE type)	: CDialog(CFocusMotorControl::IDD, pParent) {

	motorType = type;
}

//-----------------------------------------------------------------------------
CFocusMotorControl::~CFocusMotorControl() {

}

//-----------------------------------------------------------------------------
void CFocusMotorControl::DoDataExchange(CDataExchange* pDX) {

	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LARGE_STEP, LargeStepSize);
	DDX_Control(pDX, IDC_SMALL_STEP, SmallStepSize);
	DDX_Control(pDX, IDC_POSITION, Position);
}


BEGIN_MESSAGE_MAP(CFocusMotorControl, CDialog)
	ON_BN_CLICKED(IDC_LARGE_R, &CFocusMotorControl::OnBnClickedLargeR)
	ON_BN_CLICKED(IDC_SMALL_R, &CFocusMotorControl::OnBnClickedSmallR)
	ON_BN_CLICKED(IDC_SMALL_L, &CFocusMotorControl::OnBnClickedSmallL)
	ON_BN_CLICKED(IDC_LARGE_L, &CFocusMotorControl::OnBnClickedLargeL)
	ON_BN_CLICKED(IDC_LARGE_SET, &CFocusMotorControl::OnBnClickedLargeSet)
	ON_BN_CLICKED(IDC_SMALL_SET, &CFocusMotorControl::OnBnClickedSmallSet)
	ON_BN_CLICKED(IDC_CLEAR, &CFocusMotorControl::OnBnClickedClear)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
BOOL CFocusMotorControl::OnInitDialog() {

    CDialog::OnInitDialog();

	CString valstr;

	switch (motorType) {
		case FOCUS:
			largeStep = largeFocusStep;
			smallStep = smallFocusStep;

			SetWindowText(_T("Focus Motor Control"));
			valstr.Format(_T("%d"), largeStep);
			LargeStepSize.SetSel(0, -1);
			LargeStepSize.ReplaceSel(valstr, false);

			valstr.Format(_T("%d"), smallStep);
			SmallStepSize.SetSel(0, -1);
			SmallStepSize.ReplaceSel(valstr, false);
			break;

		case LASER:
			largeStep = largeLaserStep;
			smallStep = smallLaserStep;

			SetWindowText(_T("Laser Motor Control"));
			valstr.Format(_T("%d"), largeStep);
			LargeStepSize.SetSel(0, -1);
			LargeStepSize.ReplaceSel(valstr, false);

			valstr.Format(_T("%d"), smallStep);
			SmallStepSize.SetSel(0, -1);
			SmallStepSize.ReplaceSel(valstr, false);
			break;

		default:
			SetWindowText(_T("Unknown Motor Control"));
			break;
	}
	
	position = 0;
	UpdatePosition(0);

	return TRUE;
}

//-----------------------------------------------------------------------------
void CFocusMotorControl::MoveMotor(int step) {
	
	// just in case...
	step = min(step, 10000);
	step = max(step, -10000);
	switch (motorType) {
		case FOCUS:
			FluidicsRoutine(FLUIDICS_ADJUST_FOCUS, step);
			break;
		case LASER:
			FluidicsRoutine(FLUIDICS_ADJUST_LASER, step);
			break;
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
void CFocusMotorControl::UpdatePosition(int change) {

	CString valstr;

	position += change;
	valstr.Format(_T("%d"), position);
	Position.SetSel(0, -1);
	Position.ReplaceSel(valstr, false);
}

//-----------------------------------------------------------------------------
void CFocusMotorControl::OnBnClickedLargeR() {

	MoveMotor(largeStep);
	UpdatePosition(largeStep);
}

//-----------------------------------------------------------------------------
void CFocusMotorControl::OnBnClickedSmallR() {

	MoveMotor(smallStep);
	UpdatePosition(smallStep);
}

//-----------------------------------------------------------------------------
void CFocusMotorControl::OnBnClickedSmallL() {

	MoveMotor(-smallStep);
	UpdatePosition(-smallStep);
}

//-----------------------------------------------------------------------------
void CFocusMotorControl::OnBnClickedLargeL() {

	MoveMotor(-largeStep);
	UpdatePosition(-largeStep);
}

//-----------------------------------------------------------------------------
void CFocusMotorControl::OnBnClickedLargeSet() {

	CString valstr;
	char cval[20];

	LargeStepSize.GetWindowTextW(valstr);
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %d", &largeStep);
	switch (motorType) {
		case FOCUS:
			largeFocusStep = largeStep;
			smallFocusStep = smallStep;
			break;

		case LASER:
			largeLaserStep = largeStep;
			smallLaserStep = smallStep;
			break;

		default:
			break;
	}
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CFocusMotorControl::OnBnClickedSmallSet() {

	CString valstr;
	char cval[20];

	SmallStepSize.GetWindowTextW(valstr);
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %d", &smallStep);
	switch (motorType) {
		case FOCUS:
			largeFocusStep = largeStep;
			smallFocusStep = smallStep;
			break;

		case LASER:
			largeLaserStep = largeStep;
			smallLaserStep = smallStep;
			break;

		default:
			break;
	}
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CFocusMotorControl::OnBnClickedClear() {

	position = 0;
	UpdatePosition(0);
}
