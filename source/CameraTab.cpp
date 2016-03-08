//-----------------------------------------------------------------------------
//  IFCB Project
//	CameraTab.cpp : implementation file
//	Martin Cooper
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "CameraTab.h"
#include "Analysis.h"
#include "FileIo.h"
#include "config.h"
#include "IfcbDlg.h"
#include "Daq.h"
#include "Process.h"
#include "GraphTab.h"
#include "Fluids.h"

IMPLEMENT_DYNAMIC(CCameraTab, CDialog)

//-----------------------------------------------------------------------------
// convert IIDC color code to human readable string.
//-----------------------------------------------------------------------------
CString ColorCodeToString(UINT32_TYPE cc) {

    switch(cc) {
		case E_CC_MONO8:    return CString(_T("Mono8") );
		case E_CC_MONO12:   return CString(_T("Mono12") );
		case E_CC_MONO16:   return CString(_T("Mono16") );
		case E_CC_SMONO12:  return CString(_T("SMono12") );
		case E_CC_SMONO16:  return CString(_T("SMono16") );
		case E_CC_RAW8:     return CString(_T("Raw8") );
		case E_CC_RAW12:    return CString(_T("Raw12") );
		case E_CC_RAW16:    return CString(_T("Raw16") );
		case E_CC_RGB8:     return CString(_T("RGB8") );
		case E_CC_RGB12:    return CString(_T("RGB12") );
		case E_CC_RGB16:    return CString(_T("RGB16") );
		case E_CC_SRGB12:   return CString(_T("SRGB12") );
		case E_CC_SRGB16:   return CString(_T("SRGB16") );
		case E_CC_YUV411:   return CString(_T("YUV411") );
		case E_CC_YUV422:   return CString(_T("YUV422") );
		case E_CC_YUV444:   return CString(_T("YUV444") );
		default: return CString(_T("Unknown"));
    }
}

//-----------------------------------------------------------------------------
CCameraTab::CCameraTab(CWnd* pParent) : CTab(CCameraTab::IDD, pParent), m_FrameCount(0), m_FramesDropped(0) {

}

//-----------------------------------------------------------------------------
CCameraTab::~CCameraTab() {

}

//-----------------------------------------------------------------------------
void CCameraTab::DoDataExchange(CDataExchange* pDX) {

	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LOAD_BUTTON, LoadButton);
	DDX_Control(pDX, IDC_SAVE_BUTTON, SaveButton);
//	DDX_Control(pDX, IDC_BLOB_CHECK, BlobAnalysis);
	DDX_Control(pDX, IDC_MODE_SELECT, ModeSelect);
	DDX_Control(pDX, IDC_VIEW_IMAGES, ViewImages);
	DDX_Control(pDX, IDC_EXPOSURE_SLIDER, ShutterSlider);
	DDX_Control(pDX, IDC_PICVIEW, PicView);
	DDX_Control(pDX, IDC_SAVE_ALL, SaveAllImages);
}

//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CCameraTab, CDialog)
    ON_WM_PAINT()
    ON_MESSAGE(WM_UNI_API,OnNotification)
    ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_SAVE_BUTTON, &CCameraTab::OnBnClickedSaveButton)
	ON_BN_CLICKED(IDC_LOAD_BUTTON, &CCameraTab::OnBnClickedLoadButton)
//	ON_BN_CLICKED(IDC_BLOB_CHECK, &CCameraTab::OnBnClickedBlobCheck)
	ON_CBN_SELCHANGE(IDC_MODE_SELECT, &CCameraTab::OnCbnSelchangeModeSelect)
	ON_BN_CLICKED(IDC_VIEW_IMAGES, &CCameraTab::OnBnClickedViewImages)
	ON_STN_DBLCLK(IDC_PICVIEW, &CCameraTab::OnStnDblclickPicview)
	ON_BN_CLICKED(IDC_ZOOM_BUTT, &CCameraTab::OnBnClickedZoomButt)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_EXPOSURE_SLIDER, &CCameraTab::OnNMCustomdrawExposureSlider)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
void CCameraTab::CloseDriver() {

	// stop grabbing
	UCC_GrabStop(CameraInfo.m_CamId, 1);

	// free resources before program termination
    CloseCamera();                                        
    UCC_UnRegisterNotification(m_NodeListChangedHandle);
//    UCC_UnRegisterNotification(m_FrameDroppedHandle);
    UCC_UnRegisterNotification(m_FrameReadyHandle);
    UCC_Release(); // deinitializes API
}

//-----------------------------------------------------------------------------
BOOL CCameraTab::OpenDriver() {

	CALL_UCC_CHECKED( UCC_Init(),S_OK,FALSE,_T("Could not Init the UCC API") );		// Initialize UniAPI
	DEBUG_MESSAGE_EXT(_T("Initialised UCC API\r\n"));

    /* Register notification for device list changes */
    m_NodeListChangedNotification.m_CamId            = UNI_ALL_CAMERAS;
    m_NodeListChangedNotification.m_NotificationType = E_UNI_NOTIFICATION_MESSAGE;
    m_NodeListChangedNotification.m_NotificationWindowsMessage.m_hWnd    = this->m_hWnd;
    m_NodeListChangedNotification.m_NotificationWindowsMessage.m_lParam  = 0;
    m_NodeListChangedNotification.m_NotificationWindowsMessage.m_wParam  = E_NODELIST_CHANGED;
    m_NodeListChangedNotification.m_NotificationWindowsMessage.m_Msg     = WM_UNI_API;
    CALL_UCC_CHECKED( UCC_RegisterNodeListChangedNotification(&m_NodeListChangedHandle,&m_NodeListChangedNotification), S_OK,FALSE,_T("Could not Register Notification") );

	// Register notification for incoming image frames - callback method
    m_FrameReadyNotification.m_CamId            = UNI_ALL_CAMERAS;
    m_FrameReadyNotification.m_NotificationType = E_UNI_NOTIFICATION_CALLBACK;
    m_FrameReadyNotification.m_NotificationCallback.m_Arg.m_Parameter    = this;
    m_FrameReadyNotification.m_NotificationCallback.m_Callback           = OnFrameReadyCallback;
    CALL_UCC_CHECKED( UCC_RegisterFrameReadyNotification(&m_FrameReadyHandle,&m_FrameReadyNotification), S_OK,FALSE,_T("Could not Register Notification") );
    
	/* Register notification for dropped frames */
/*    m_FrameDroppedNotification.m_CamId            = UNI_ALL_CAMERAS;
    m_FrameDroppedNotification.m_NotificationType = E_UNI_NOTIFICATION_MESSAGE;
    m_FrameDroppedNotification.m_NotificationWindowsMessage.m_hWnd    = this->m_hWnd;
    m_FrameDroppedNotification.m_NotificationWindowsMessage.m_lParam  = 0;
    m_FrameDroppedNotification.m_NotificationWindowsMessage.m_wParam  = E_FRAME_DROPPED;
    m_FrameDroppedNotification.m_NotificationWindowsMessage.m_Msg     = WM_UNI_API;
    CALL_UCC_CHECKED( UCC_RegisterFrameDroppedNotification(&m_FrameDroppedHandle,&m_FrameDroppedNotification), S_OK,FALSE,_T("Could not Register Notification") );
*/
    CameraInfo.m_CamId = -1;

	    return TRUE;
}

//-----------------------------------------------------------------------------
void CCameraTab::OnClose() {

	CloseDriver();
}

//-----------------------------------------------------------------------------
BOOL CCameraTab::OnInitDialog() {

    CDialog::OnInitDialog();

	if (!OpenDriver())
		return FALSE;

	// configuration check boxes
	if (viewImages)
		ViewImages.SetCheck(BST_CHECKED);
/*	if (blobAnalysis)
		BlobAnalysis.SetCheck(BST_CHECKED);*/

	Buffer = NULL;
	zoomed = false;

//	DEBUG_MESSAGE_EXT(_T("CCameraTab::OnInitDialog()\r\n"));

	ShutterSlider.ShowWindow(FALSE);

    return TRUE;
}

//-----------------------------------------------------------------------------
void CCameraTab::ZeroFrameCount() {

	m_FrameCount = 0;
	SetDlgItemInt(IDC_FRAMECOUNT, m_FrameCount, FALSE);
}

//-----------------------------------------------------------------------------
void CCameraTab::OnPaint() {

	CDialog::OnPaint();
        
    CPaintDC dc(&PicView);
    CRect    PaintRect;
    PicView.GetClientRect(&PaintRect);
    dc.SetStretchBltMode(HALFTONE);

    if (!Image.IsNull())
		Image.StretchBlt(dc.m_hDC, PaintRect);

	SetDlgItemInt(IDC_FRAMECOUNT, m_FrameCount, FALSE);
}

//-----------------------------------------------------------------------------
/* Callback routine for 'Frame Ready' events.
 * The received image is fetched, converted to Windows BGR, and a window repaint is triggered.
 * For optimal performance, this routine should return as fast as possible.*/
//-----------------------------------------------------------------------------
VOID_TYPE NOTIFICATION_CALLING_CONVENTION CCameraTab::OnFrameReadyCallback(S_UNI_CALLBACK_ARGUMENT arg) {

	IfcbHandle->KillTimer(ACQ_START_TIMEOUT_ID);

	if (!IfcbHandle->grabbing)								// acquisition has been stopped
		return;
		
	triggerTickCount = GetTickCount();

    UNI_RETURN_TYPE Result;

    // This is a static function, so we need to retrieve the object pointer first
    CCameraTab   *This = reinterpret_cast<CCameraTab*>(arg.m_Parameter);
    if (This == NULL) {
		ERROR_MESSAGE_EXT(_T("Error in FrameReadyCallback\r\n"));
		return;												// return on failure
	}
    static CAMERA_INFO cam_info;
	cam_info = This->CameraInfo;
    static S_UNI_TRANSPORT_FORMAT_INFO FormatInfo;

    // Get image in native transport format
	Result = UCC_GetNativeImageEx(cam_info.m_CamId,         // camera 
                                  This->Buffer,				// begin of target data buffer
                                  &FormatInfo,              // Format info to be filled
                                  NULL,
                                  0);						// timeout
    if (Result != S_OK) {
		ERROR_MESSAGE_EXT(_T("Error in FrameReadyCallback\r\n"));
		return;												// return on failure
	}

	This->m_FrameCount++;
	This->SetDlgItemInt(IDC_FRAMECOUNT, This->m_FrameCount, FALSE);

	// stuff needed for image display
	CRect rect;
	if (viewImages) {
		This->PicView.GetWindowRect(&rect);
		This->ScreenToClient(&rect);

		// copy Buffer to Image
		// display raw image
		BYTE *d;
		int pitch = This->Image.GetPitch();

		for (int y = 0, i = 0; y < This->Image.GetHeight(); y++) {
			d = (BYTE *)This->Image.GetBits() + y * pitch;
			for (int x = 0; x < This->Image.GetWidth(); x++, i++) {
				*d++ = This->Buffer[i];							// blue
				*d++ = This->Buffer[i];							// green
				*d++ = This->Buffer[i];							// red
			}
		}
	}

	unsigned int roisFound = 0;
	if (blobAnalysis)
		roisFound = Analyze(This, &rect, false);				// roi analysis

	if (viewImages)	{											// draw image
		This->InvalidateRect(rect, FALSE);
		if (This->zoomed)
			This->ZoomDlg->Invalidate();
		if (This->SaveAllImages.GetCheck() == BST_CHECKED) {	// save the whole image
			CString pathName, suffix;
			pathName = GetOutputFileName();
			suffix.Format(_T("_%d.tif"), This->m_FrameCount);
			pathName += suffix;
			This->Image.Save(pathName, Gdiplus::ImageFormatTIFF);
		}
	}

	ProcessTrigger(roisFound);
	SetHWParams();												// chnage any parameters that need it
	DaqArmTrigger();						// re-arm the trigger

	// alignment graph plot - takes about 0.5 ms
	if (GraphData.GraphType != GraphData.GRAPH_NONE) {
		IfcbHandle->TabContainer.tabPages[graphTab]->UpdateGraph();
		IfcbHandle->TabContainer.tabPages[graphTab]->PostMessage(WM_PAINT, 0, 0);
	}
}

//-----------------------------------------------------------------------------
//	Set up used image buffer to match camera config.
//	Buffer holds the raw image, Image is a fancy object that it gets copied to
//-----------------------------------------------------------------------------
bool CCameraTab::SetupImage() {

    UINT32_TYPE w,h,depth;
    /* collect information about current image format */
    CALL_UCC_CHECKED(
        UCC_GetCurrentImageFormat(CameraInfo.m_CamId,			// camera to query for format
								  &w,							// width in pixel
                                  &h,							// height in pixel
                                  &CameraInfo.m_ColorCode,	// iidc color coding
                                  &depth),                      // bit per pixel
                                  S_OK, false, _T("Could not get image format"));

    UINT32_TYPE Dummy;
    UINT32_TYPE BitsPerPixel;
    UCC_GetCameraInfo           ( CameraInfo.m_CamId,E_CAMINFO_INTERLACED   , &CameraInfo.m_IsInterlaced);
    UCC_GetCameraInfo           ( CameraInfo.m_CamId,E_CAMINFO_BAYERPATTERN , &CameraInfo.m_BayerPattern);
    UCC_GetCurrentFixedFormat   ( CameraInfo.m_CamId, &CameraInfo.m_Format, &CameraInfo.m_Mode,&Dummy);
    UIT_GetBitsPerPixel         ( CameraInfo.m_ColorCode, &BitsPerPixel);
    
	if (BitsPerPixel != 8) {
		ERROR_MESSAGE_EXT(_T("wrong bits per pixel\r\n"));
		return false;
	}
	if (Buffer)
		free(Buffer);

	if (!(Buffer = (BYTE *)malloc(w * h))) {
		ERROR_MESSAGE_EXT(_T("malloc error \r\n"));
		return false;
	}

    // create Image object to be same as camera image
	if (!Image.IsNull())
		Image.Destroy();
	Image.Create(w, -static_cast<INT32_TYPE>(h), 24);			// RGB

    return true;
}

//-----------------------------------------------------------------------------
// Close the current camera, if opened.
//-----------------------------------------------------------------------------
void CCameraTab::CloseCamera() {

	if (CameraInfo.m_CamId == -1)
		return;

	// stop grabbing
	UCC_GrabStop(CameraInfo.m_CamId, 1);
//	if (UCC_GrabStop(CameraInfo.m_CamId, 1) != S_OK)
//		ERROR_MESSAGE_EXT(_T("GrabStop failed\r\n"));

	UCC_CloseCamera(CameraInfo.m_CamId);
    CameraInfo.m_CamId = -1;
}

//-----------------------------------------------------------------------------
//	Loads the first camera it finds
//-----------------------------------------------------------------------------
bool CCameraTab::GetCamera(void) {

	UINT32_TYPE          cameraCount;						// initialized with max number of cameras supported
    UNI_RETURN_TYPE     Result;

	CloseCamera();											// close any open cameras

	int attempts = 5;
	do {													// loop until 1 camera found or timeout
		// get first available camera
		cameraCount = 1;									// max # of cameras to get
		Result = UCC_GetCameras(&cameraCount, &CameraInfo.m_CamId);
		if (Result == UNI_RESULT_PARAMETER_INVALID_1)
			DEBUG_MESSAGE_EXT(_T("pnSize is NULL or *pnSize==0 or *pnSize>max\r\n"));
		if (Result == UNI_RESULT_PARAMETER_INVALID_2)
			DEBUG_MESSAGE_EXT(_T("full range inside vecIDs not writeable\r\n"));
		if (Result == UNI_RESULT_MORE_DATA)
			DEBUG_MESSAGE_EXT(_T("More data provided than for given array\r\n"));
		Sleep(2000);
		if (--attempts == 0)
			break;
	} while ((Result != S_OK) || (cameraCount != 1));

	if (attempts == 0) {									// timeout
		ShutterSlider.EnableWindow(FALSE);
		ModeSelect.EnableWindow(FALSE);
		CameraInfo.m_CamId = -1;
		DEBUG_MESSAGE_EXT(_T("Get Cameras failed in CIfcbDlg::GetCamera\r\n"));
		if (cameraCount != 1)
			DEBUG_MESSAGE_EXT(_T("Not exactly 1 camera found\r\n"));
		if (cameraCount == 0)
			DEBUG_MESSAGE_EXT(_T("No cameras found\r\n"));

		return false;
	}

	// we have a camera
	m_mapListedModes.clear();
    
    /* open selected camera */
    UCC_OpenCamera(CameraInfo.m_CamId);
    UINT32_TYPE pos(0), mode, color_code, x_max, y_max;
    CString txt;
    DWORD_PTR nIndex = 0;

    /* fill mode selection control with supported 'free' modes */
    while (pos != -1) {
        if (UCC_EnumFreeModes(CameraInfo.m_CamId, &pos, &mode, &color_code, &x_max, &y_max) == S_OK) {
            S_MODE_ENTRY entry;
            entry.nColorCode = color_code;
            entry.nMode = mode;
            if(mode < 0x01000000) {
                /* 'real' Format 7 IIDC mode */
                txt.Format( _T("%s %ux%u (Mode %u)"), ColorCodeToString(color_code), x_max, y_max, mode);
            } else {
                /* virtual mode */
                if ((mode & 0xff000000) == 0x01000000 && (mode & 0xffffff) != 0x001001) {
                    /* Most significant byte set to 1 indicating binning and binning factors are not both '1'. */
                    txt.Format( _T("%s %ux%u (Binning %uH %uV)"), ColorCodeToString(color_code), x_max, y_max, (mode&0xfff000)>>12, mode&0xfff);
                } else {
                    /* Binning factors are both '1' (so no binning is applied), or unknown mode (Highest byte != 0) */
                    txt.Format( _T("%s %ux%u"), ColorCodeToString(color_code), x_max, y_max);
                }
            }
            int nComboPos =ModeSelect.AddString(txt);
            ModeSelect.SetItemData(nComboPos , nIndex);
            m_mapListedModes[nIndex] = entry;
            nIndex++;
        } else {
            pos = -1;
        }
    }

    /* synchronize shutter slider with camera */
    UINT32_TYPE shutter_min(0),shutter_max(0),shutter_val(0);
    UCC_GetFeatureMin   (CameraInfo.m_CamId, E_FEAT_SHUTTER, &shutter_min);
    UCC_GetFeatureMax   (CameraInfo.m_CamId, E_FEAT_SHUTTER, &shutter_max);
    UCC_GetFeatureValue (CameraInfo.m_CamId, E_FEAT_SHUTTER, &shutter_val);
    SetDlgItemInt       (IDC_SHUTTER_MIN, shutter_min, FALSE);
    SetDlgItemInt       (IDC_SHUTTER_MAX, shutter_max, FALSE);
    ShutterSlider.SetRange(shutter_min, shutter_max);
    ShutterSlider.SetPos(shutter_val);
    ShutterSlider.EnableWindow(TRUE);

	CCameraTab::SetDlgItemInt(IDC_FRAMECOUNT, 0, FALSE);

    /* select first mode */
    if (ModeSelect.GetCount() != 0) {
        ModeSelect.SetCurSel(0);
        ModeSelect.EnableWindow(TRUE);
    }
    OnModeSelectChanged();

#ifdef IFCB8_BENCHTOP
/*
	// Camera configuration
	// would be smart to issue a camera reset then change only those params you need to
	if (S_OK == UCC_LoadSettings (CameraInfo.m_CamId,"C:\\IFCB8_code\\GC1380H_28Oct2010.xml", E_SSM_USE_GUID) )
	{	// not sure why I need to bracket this to compile...
		DEBUG_MESSAGE_EXT(_T("Loaded camera XML settings file\r\n"));
	}
	else
		DEBUG_MESSAGE_EXT(_T("Not able to load camera XML settings file\r\n"));
//	UNI_RETURN_TYPE ret;
//	ret = UCC_InputPin_Set(CameraInfo.m_CamId, 0, E_IPINMODE_TRIGGER, 1);		// SyncIn1, rising edge
//	ret = UCC_Trigger_Set(CameraInfo.m_CamId, E_TRIGGERMODE_HW_FIXED_SHUTTER, 1, 1);	// active, rising edge
//	UCC_SetFeatureValue  ( CameraInfo.m_CamId, E_FEAT_GAIN, E_FEATSTATE_AUTO);	// set a nonzero gain
*/
#endif

	// start grabbing
	CALL_UCC_CHECKED(UCC_GrabStart(CameraInfo.m_CamId,1000),S_OK, false, _T("GrabStart failed"));

	DEBUG_MESSAGE_EXT(_T("Camera found & successfully set up\r\n"));

	return true;
}

//-----------------------------------------------------------------------------
unsigned CCameraTab::GetFrameCount(void) {

	return m_FrameCount;
}

//-----------------------------------------------------------------------------
// handle UniAPI notifications received as window message
//-----------------------------------------------------------------------------
LRESULT CCameraTab::OnNotification(WPARAM wPrm, LPARAM lPrm) {

    switch (wPrm) {
	    case E_NODELIST_CHANGED:        // cameras have been added or removed
			m_FrameCount++;
            break;
		case E_FRAME_DROPPED:           // a frame has been dropped by the receiver
			SetDlgItemInt(IDC_FRAMEDROPPEDCOUNT, ++m_FramesDropped, FALSE);
            break;
#ifdef FRAME_MESSAGE
		case E_FRAME_READY:

			GetImage();
			break;
#endif
		default:
			break;
	}

    return S_OK;
}

//-----------------------------------------------------------------------------
// handle mode control selection events
//-----------------------------------------------------------------------------
void CCameraTab::OnModeSelectChanged() {

    int pos = ModeSelect.GetCurSel();
    if (pos != CB_ERR) {
        CString selectedItem;
        ModeSelect.GetLBText(pos, selectedItem);

		if (selectedItem.Left (7) == _T("Unknown")) {
			IfcbHandle->RunButton.EnableWindow(FALSE);
		} else {
            /* collect information about selected mode */
            UINT32_TYPE nMode;
            UINT32_TYPE nColorCode;
            UINT32_TYPE nWidth;
            UINT32_TYPE nHeight;
            UINT32_TYPE nBufferCount = 3;
            DWORD_PTR nIndex = ModeSelect.GetItemData(pos);

            nMode = m_mapListedModes[nIndex].nMode;
            nColorCode = m_mapListedModes[nIndex].nColorCode;
            UCC_GetFreeModeInfo(CameraInfo.m_CamId, nMode, E_FREE_MODE_WIDTH, &nWidth);
            UCC_GetFreeModeInfo(CameraInfo.m_CamId, nMode, E_FREE_MODE_HEIGHT, &nHeight);

            /* call UCC_PrepareFreeGrab for selected mode */
            if(UCC_PrepareFreeGrab(CameraInfo.m_CamId, &nMode, &nColorCode, &nWidth, &nHeight, &nBufferCount,NULL,NULL,NULL) != S_OK) {
                MessageBox(_T("could not set new mode"),_T("error"));
            } else {
				IfcbHandle->RunButton.EnableWindow(TRUE);
            }
        }
    }
}

//-----------------------------------------------------------------------------
//	returns run state - if it fails, it sorts out all cameraTab stuff here
//-----------------------------------------------------------------------------
bool CCameraTab::Run(bool run) {

	if (run) {												// Run
		if (!GetCamera())
			return false;

        SetupImage();
		LoadButton.EnableWindow(FALSE);
        ModeSelect.EnableWindow(FALSE);
		m_FrameCount = 0;
		SetDlgItemText(IDC_FRAMEDROPPEDCOUNT, _T(""));

		return true;
	} else {												// Stop 
		LoadButton.EnableWindow(TRUE);
        ModeSelect.EnableWindow(TRUE);

		return false;
	}
}

//-----------------------------------------------------------------------------
void CCameraTab::OnBnClickedSaveButton() {

	TCHAR szFilters[]= _T("bmp (*.bmp)|*.bmp|All Files (*.*)|*.*||");

	// Create an Open dialog
	CFileDialog fileDlg(false, _T(".bmp"), _T(""), OFN_OVERWRITEPROMPT, szFilters);

	if(fileDlg.DoModal() == IDOK) {
		CString pathName = fileDlg.GetPathName();
		CALL_CHECKED_EXT(Image.Save(pathName, Gdiplus::ImageFormatBMP),	S_OK, _T("Could not save file\r\n"));
	}
}

//-----------------------------------------------------------------------------
void CCameraTab::OnBnClickedLoadButton() {

	TCHAR szFilters[]= _T("tiff (*.tif)|*.tif|bmp (*.bmp)|*.bmp|All Files (*.*)|*.*||");

	// Create an Open dialog
	CFileDialog fileDlg(true, _T(".tif"), _T(""), OFN_FILEMUSTEXIST, szFilters);

	if(fileDlg.DoModal() != IDOK)
		return;

	Image.Destroy();
	CString pathName = fileDlg.GetPathName();
	CALL_CHECKED_RETVOID_EXT(Image.Load(pathName), S_OK, _T("Could not load file\r\n"));

	// scale the image and fit in window
    CRect rect;
    PicView.GetWindowRect(&rect);
    ScreenToClient(&rect);
    InvalidateRect(rect, FALSE);

	// roi analysis etc
	if (!blobAnalysis)
		return;
		
	// copy Image to Buffer
	DEBUG_MESSAGE_EXT(_T("Copying Image to Buffer"));

	using namespace std;
	unsigned int x, y, w, h;
	BYTE *d;

	w = Image.GetWidth();
	h = Image.GetHeight();
	if (Buffer)
		free(Buffer);

	if (!(Buffer = (BYTE *)malloc(w * h))) {
		ERROR_MESSAGE_EXT(_T("malloc error \r\n"));
		return;
	}

	d = Buffer;
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			*d++ = Image.GetPixel(x, y) & 0x000000FF;
		}
	}

	DEBUG_MESSAGE_EXT(_T(" - done\r\n"));

	Analyze(this, &rect, true);
}

//-----------------------------------------------------------------------------
/*void CCameraTab::OnBnClickedBlobCheck() {

	blobAnalysis = (BlobAnalysis.GetCheck() == BST_CHECKED);
	WriteCfgFile();
}*/

//-----------------------------------------------------------------------------
void CCameraTab::OnCbnSelchangeModeSelect() {

    int pos = ModeSelect.GetCurSel();
    if (pos != CB_ERR) {
        CString selectedItem;
        ModeSelect.GetLBText(pos, selectedItem);

		if (selectedItem.Left (7) == _T("Unknown")) {
			IfcbHandle->RunButton.EnableWindow(FALSE);
		} else {
            /* collect information about selected mode */
            UINT32_TYPE nMode;
            UINT32_TYPE nColorCode;
            UINT32_TYPE nWidth;
            UINT32_TYPE nHeight;
            UINT32_TYPE nBufferCount = 3;
            DWORD_PTR nIndex = ModeSelect.GetItemData(pos);

            nMode = m_mapListedModes[nIndex].nMode;
            nColorCode = m_mapListedModes[nIndex].nColorCode;
            UCC_GetFreeModeInfo(CameraInfo.m_CamId, nMode, E_FREE_MODE_WIDTH, &nWidth);
            UCC_GetFreeModeInfo(CameraInfo.m_CamId, nMode, E_FREE_MODE_HEIGHT, &nHeight);

            /* call UCC_PrepareFreeGrab for selected mode */
            if(UCC_PrepareFreeGrab(CameraInfo.m_CamId, &nMode, &nColorCode, &nWidth, &nHeight, &nBufferCount,NULL,NULL,NULL) != S_OK) {
                MessageBox(_T("could not set new mode"),_T("error"));
            } else {
				IfcbHandle->RunButton.EnableWindow(TRUE);
            }
        }
    }
}

//-----------------------------------------------------------------------------
// handle shutter slider changes.
//-----------------------------------------------------------------------------
void CCameraTab::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {

    CALL_UCC_CHECKED(UCC_SetFeatureValue(CameraInfo.m_CamId, E_FEAT_SHUTTER, ShutterSlider.GetPos()), S_OK, ,_T("Feature not set") );
    CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

//-----------------------------------------------------------------------------
void CCameraTab::OnBnClickedViewImages() {

	viewImages = (ViewImages.GetCheck() == BST_CHECKED);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CCameraTab::OnStnDblclickPicview() {

	if (Image.IsNull())
		return;

	ZoomDlg = new CZoomDlg(this);
	ZoomDlg->Create(IDD_ZOOM, this);
	ZoomDlg->ShowWindow(SW_SHOW);
}

//-----------------------------------------------------------------------------
void CCameraTab::OnBnClickedZoomButt() {

	OnStnDblclickPicview();
}

//-----------------------------------------------------------------------------
void CCameraTab::OnNMCustomdrawExposureSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

