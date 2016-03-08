#pragma once
#include "resource.h"
#include "afxwin.h"
#include "atlimage.h"
#include "UniControl.h"
#include "UniTransform.h"
#include <vector>
#include <map>
#include "afxcmn.h"
#include "ZoomDlg.h"
#include "Tab.h"

#define WM_UNI_API WM_USER+1 // Message for the Api 
#define CALL_UCC_CHECKED(Foo,Expected,Result,Text){if(Expected != Foo){CString txt(Text _T(" Reason:")); char ar[256];UCC_GetErrorInfo(Foo,ar,256); txt +=ar;MessageBox(txt);return Result;}}

struct CAMERA_INFO {
    ID_TYPE         m_CamId;            //!< camera id
    UINT32_TYPE     m_ColorCode;        //!< current iidc color coding
    UINT32_TYPE     m_IsInterlaced;     //!< interlaced camera
    UINT32_TYPE     m_Mode;             //!< current iidc mode
    UINT32_TYPE     m_Format;           //!< current iidc format
    UINT32_TYPE     m_BayerPattern;     //!< color filter id
};

class CZoomDlg;
class CTab;
class CCameraTab : public CTab {

	DECLARE_DYNAMIC(CCameraTab)

protected:
    enum LocalEvents{E_NODELIST_CHANGED, E_FRAME_DROPPED, E_FRAME_READY};
    // node list change events
    S_UNI_NOTIFICATION      m_NodeListChangedNotification;
    UNI_NOTIFICATION_HANDLE m_NodeListChangedHandle;

    // frame ready events
    S_UNI_NOTIFICATION      m_FrameReadyNotification;
    UNI_NOTIFICATION_HANDLE m_FrameReadyHandle;

    // frame dropped events
//    S_UNI_NOTIFICATION      m_FrameDroppedNotification;
//    UNI_NOTIFICATION_HANDLE m_FrameDroppedHandle;

	CSliderCtrl		ShutterSlider;
	CStatic			PicView;
	CButton			LoadButton;
	CButton			SaveButton;
	CButton			BlobAnalysis;
	CButton			StainSample;
	CButton			StainAuto;
	CButton			StainRinseMixingChamber;
	CButton			LockFilterSlider;
	CComboBox		ModeSelect;
	CButton			ViewImages;
	CButton			SaveAllImages;
	CZoomDlg		*ZoomDlg;

	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    CAMERA_INFO     CameraInfo;
    virtual BOOL	OnInitDialog();
	void			GetImage();
    afx_msg void	OnPaint();
	afx_msg void	OnClose();

	DECLARE_MESSAGE_MAP()

    void            CloseCamera();
    bool            GrabImage(UINT32_TYPE CamId);
    bool            GetCamera();

    static VOID_TYPE NOTIFICATION_CALLING_CONVENTION OnFrameReadyCallback ( S_UNI_CALLBACK_ARGUMENT arg );
    UINT32_TYPE     m_FramesDropped;
    unsigned		m_FrameCount;
    typedef struct {
        UINT32_TYPE nMode;
        UINT32_TYPE nColorCode;
    } S_MODE_ENTRY;
    std::map<DWORD_PTR, S_MODE_ENTRY> m_mapListedModes;
    afx_msg LRESULT OnNotification      (WPARAM wPrm, LPARAM lPrm);
    afx_msg void    OnModeSelectChanged();
    afx_msg void    OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

public:
					CCameraTab(CWnd* pParent = NULL);   // standard constructor
	virtual			~CCameraTab();
	virtual bool	Run(bool run);
	unsigned		GetFrameCount(void);
	void			ZeroFrameCount(void);
	bool			zoomed;

	afx_msg void	OnBnClickedLoadButton();
	afx_msg void	OnBnClickedSaveButton();
//	afx_msg void	OnBnClickedBlobCheck();
	afx_msg void	OnCbnSelchangeModeSelect();
	afx_msg void	OnBnClickedViewImages();
	afx_msg void	OnStnDblclickPicview();
	afx_msg void	OnBnClickedZoomButt();
	virtual void	CloseDriver();
	virtual BOOL	OpenDriver();

	enum { IDD = IDD_CAMERA };
    BYTE			*Buffer;
    bool			SetupImage();
    CImage          Image;            // display image
	afx_msg void OnNMCustomdrawExposureSlider(NMHDR *pNMHDR, LRESULT *pResult);
};
