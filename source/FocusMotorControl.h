//-----------------------------------------------------------------------------
//  IFCB Project
//	FocusMotorControl.h
//	Martin Cooper
//-----------------------------------------------------------------------------
#pragma once

#include "resource.h"
#include "afxwin.h"

//-----------------------------------------------------------------------------
class CFocusMotorControl : public CDialog {
	DECLARE_DYNAMIC(CFocusMotorControl)

public:
	typedef enum MOTOR_TYPE {UNKNOWN, FOCUS, LASER};
	CFocusMotorControl(CWnd* pParent = NULL, MOTOR_TYPE type = UNKNOWN);   // standard constructor
	virtual ~CFocusMotorControl();

	enum { IDD = IDD_FOCUS_MIRROR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedLargeR();
	afx_msg void OnBnClickedSmallR();
	afx_msg void OnBnClickedSmallL();
	afx_msg void OnBnClickedLargeL();
	afx_msg void OnBnClickedLargeSet();
	afx_msg void OnBnClickedSmallSet();
	afx_msg void OnBnClickedClear();

	void	UpdatePosition(int change);
	void	MoveMotor(int step);
	CEdit	LargeStepSize;
	CEdit	SmallStepSize;
	CEdit	Position;
	int		position;
	MOTOR_TYPE	motorType;
	int		largeStep;
	int		smallStep;
};
