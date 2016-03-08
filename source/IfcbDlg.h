//-----------------------------------------------------------------------------
//  IFCB Project
//	IfcbDlg.h
//	Martin Cooper
//-----------------------------------------------------------------------------

#pragma once
#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "ControlTabs.h"

//	handy macros for error reporting
//	ERROR_MESSAGE(text) prints the CString text to the debug window
//	DEBUG_MESSAGE(text) prints the CString text to the debug window if the Debug checkbox is checked
//	CALL_CHECKED executes a command and prints an error message if the response is not as expected
//	CALL_CHECKED_RETVOID does CALL_CHECKED and then executes return if the result was not as expected
//	CALL_CHECKED_RETFALSE does CALL_CHECKED and then executes return false if the result was not as expected
//
//	the _EXT versions are for use in functions that are not part of the CIfcbDlg class
#define ERROR_MESSAGE(text) {DebugWindow.ReplaceSel(text, false); }
#define ERROR_MESSAGE_EXT(text) {IfcbHandle->DebugWindow.ReplaceSel(text, false);}
#define DEBUG_MESSAGE(text) {if (appDebug) ERROR_MESSAGE(text);}
#define DEBUG_MESSAGE_EXT(text) {if (appDebug) ERROR_MESSAGE_EXT(text);}
#define CALL_CHECKED(Foo,Expected,Text){if(Foo != Expected){ ERROR_MESSAGE(Text);}}
#define CALL_CHECKED_EXT(Foo,Expected,Text){if(Foo != Expected){ ERROR_MESSAGE_EXT(Text);}}
#define CALL_CHECKED_RETVOID(Foo,Expected,Text){if(Foo != Expected){ ERROR_MESSAGE(Text); return;}}
#define CALL_CHECKED_RETVOID_EXT(Foo,Expected,Text){if(Foo != Expected){ ERROR_MESSAGE_EXT(Text); return;}}
#define CALL_CHECKED_RETFALSE(Foo,Expected,Text){if(Foo != Expected){ ERROR_MESSAGE(Text); return false;}}
#define CALL_CHECKED_RETFALSE_EXT(Foo,Expected,Text){if(Foo != Expected){ ERROR_MESSAGE_EXT(Text); return false;}}

#define SYRINGE_TIMER_ID		2
#define TRIGGER_TIMER_ID		3
#define ACQ_START_TIMEOUT_ID	4
#define FRAMERATE_TIMER_ID		6
#define FRAMECOUNT_TIMER_ID		7
#define AUTOSTART_TIMER_ID		8
#define TCP_TIMER_ID			9
#define CAPTURE_TIMER_ID		10
#define PZT_TIMER_ID		11
//#define PZT_TIMER_NOINIT_ID		12
#define PZT_SCAN_TIMER_ID		13


#define AUTOSTART_DELAY			60			// autostart delay (s)

// CIfcbDlg dialog
class CIfcbDlg : public CDialog {

public:
    CIfcbDlg(CWnd* pParent = NULL);    // standard constructor
    ~CIfcbDlg();

	// Dialog Data
    enum { IDD = IDD_IFCB_DIALOG };

	void			OnTimer(UINT_PTR nIDEvent);

	CEdit			DebugWindow;
	CButton			RunButton;
	CControlTabs	TabContainer;

	afx_msg void	OnBnClickedRun();
	afx_msg void	OnBnClickedTrigCont();
	void			CreateSyringeNumString(CString *string);
	bool			grabbing;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedCheck6();
	afx_msg void OnBnClickedCheck7();
	afx_msg void OnBnClickedButton13();
	afx_msg void OnBnClickedTrigger();
    afx_msg HCURSOR OnQueryDragIcon();

	HICON m_hIcon;

    virtual BOOL OnInitDialog();
	BOOL DestroyWindow();

	CButton OutputFiles;
	CButton DebugCheckBox;
	CButton ClearDebug;
	CButton TriggerButton;
	CButton TriggerContinuous;
public:
	CButton AutoshutdownCheckBox;
	afx_msg void OnBnClickedCheck10();
	void ShutdownWindows();
	CEdit nSyringesEditBox;
	CSpinButtonCtrl nSyringesSpinner;
	afx_msg void OnEnChangeEdit2();
	void	Run();
	void	Stop();
	void	WriteSyringeNum();
	bool	dacsOn;								// the DACs ON/OFF button on the hardware tab
protected:
	CButton BlobAnalysis;
	afx_msg void OnBnClickedCheck11();
	bool	rqStop;
public:
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
};
extern CIfcbDlg	*IfcbHandle;					// handle to the main dialog - crude but effective
