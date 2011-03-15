// recordDlg.cpp : implementation file
//

#include "stdafx.h"
#include <mmsystem.h>
#include "record.h"
#include "recordDlg.h"
#include "RecordSound.h"
#include "WriteSoundFile.h"
#include "PlayMMSound.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecordDlg dialog

CRecordDlg::CRecordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRecordDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRecordDlg)
		// NOTE: the ClassWizard will add member initialization here
	m_nDevice = 0;
	m_nSource = -1;
	m_nSampleRate = -1;
	m_nSampleFormat = -1;
	m_nPriority = 3;
	//}}AFX_DATA_INIT

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRecordDlg)
	DDX_Control(pDX, IDC_RECORD, m_ctrlRecord);
	DDX_Control(pDX, IDC_COMBO1, m_ctrlDevice);
	DDX_Control(pDX, IDC_COMBO4, m_ctrlSource);
	DDX_Control(pDX, IDC_SLIDER_PRIO, m_ctrlPriority);
	DDX_Control(pDX, IDC_LINK_HOME, m_btnHome);
	DDX_CBIndex(pDX, IDC_COMBO1, m_nDevice);
	DDX_CBIndex(pDX, IDC_COMBO2, m_nSampleRate);
	DDX_CBIndex(pDX, IDC_COMBO3, m_nSampleFormat);
	DDX_CBIndex(pDX, IDC_COMBO4, m_nSource);
	DDX_Slider(pDX, IDC_SLIDER_PRIO, m_nPriority);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRecordDlg, CDialog)
	//{{AFX_MSG_MAP(CRecordDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RECORD, OnRecord)
	//ON_BN_CLICKED(IDC_PLAY, OnPlay)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecordDlg message handlers

BOOL CRecordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


	// TODO: Add extra initialization here

	m_btnHome.SetURL(_T("http:\\\\www.axfree.net\\iRecord"));
	m_btnHome.SetToolTipText(_T("Go to iRecord Home"));
	m_btnHome.SetRegularColor(RGB(0,155,155));

	m_ctrlPriority.SetRange(0, 5, TRUE);

	MMRESULT	err;
	WAVEINCAPS	wic;

	/* Get the number of Digital Audio In devices in this computer */
	UINT iNumDevs = waveInGetNumDevs();

	/* Go through all of those devices, displaying their names */
	for (UINT i = 0; i < iNumDevs; i++) {
		/* Get info about the next device */
		if (!waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS))) {
			/* Display its Device ID and name */
			m_ctrlDevice.AddString(wic.szPname);
			//printf("Device ID #%u: %s\r\n", i, wic.szPname);
		}
	}

	//m_ctrlDevice.SetCurSel(0);

	/*
	UINT nDevices = ::mixerGetNumDevs();

	for (UINT i = 0; i < nDevices; i++) {
		HMIXER hMixer;
		MIXERCAPS mc;
		MIXERLINE ml;

		if (::mixerOpen(&hMixer, i, 0, 0, MIXER_OBJECTF_MIXER) == MMSYSERR_NOERROR) {
			if (::mixerGetDevCaps(i, &mc, sizeof(mc)) == MMSYSERR_NOERROR)
				m_ctrlDevice.AddString(mc.szPname);

			// This device should have a WAVEIN destination line. Let's get its ID so
			// that we can determine what source lines are available to record from
			ml.cbStruct = sizeof(MIXERLINE);
			ml.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
			if (::mixerGetLineInfo((HMIXEROBJ) hMixer, &ml, MIXER_GETLINEINFOF_COMPONENTTYPE) == MMSYSERR_NOERROR) {
				// Get how many source lines are available from which to record. For example, there could
				// be a Mic In (MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE), Line In (MIXERLINE_COMPONENTTYPE_SRC_LINE),
				// and/or SPDIF In (MIXERLINE_COMPONENTTYPE_SRC_DIGITAL)
				UINT nSources = ml.cConnections;

				for (UINT j = 0; j < nSources; j++) {
					ml.cbStruct = sizeof(MIXERLINE);
					ml.dwSource = j;
					if (::mixerGetLineInfo((HMIXEROBJ) hMixer, &ml, MIXER_GETLINEINFOF_SOURCE) == MMSYSERR_NOERROR) {
						//if (ml.dwComponentType != MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER)
						//	printf("\t#%lu: %s\n", j, ml.szName);
						CString cs;
						cs.Format("%s - %s", ml.Target.szPname, ml.szName);
						m_ctrlSource.AddString(cs);

						for (UINT k = 0; k < ml.cControls; k++) {
							MIXERLINECONTROLS mlc;
							MIXERCONTROL m;

							mlc.cbStruct = sizeof(mlc);
							mlc.dwControlID = k;
							mlc.cbmxctrl = sizeof(m);
							mlc.pamxctrl = &m;
							mlc.cControls = 1;
							if (::mixerGetLineControls((HMIXEROBJ) hMixer, &mlc, MIXER_GETLINECONTROLSF_ONEBYID) == 0) {
							}
						}
					}
				}
			}
			err = ::mixerClose(hMixer);
		}
	}
	 */

	CWinApp* pApp = AfxGetApp();

	m_nDevice = pApp->GetProfileInt(_T("Settings"), _T("Device"), 0);
	m_nSource = pApp->GetProfileInt(_T("Settings"), _T("Source"), -1);
	m_nSampleRate = pApp->GetProfileInt(_T("Settings"), _T("SampleRate"), 3);
	m_nSampleFormat = pApp->GetProfileInt(_T("Settings"), _T("SampleFormat"), 3);
	m_nPriority = pApp->GetProfileInt(_T("Settings"), _T("Priority"), 3);

	UpdateData(FALSE);

	m_bRecording = FALSE;
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRecordDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRecordDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRecordDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CRecordDlg::OnDestroy() 
{
	CWinApp* pApp = AfxGetApp();

	UpdateData(TRUE);

	pApp->WriteProfileInt(_T("Settings"), _T("Device"), m_nDevice);
	pApp->WriteProfileInt(_T("Settings"), _T("Source"), m_nSource);
	pApp->WriteProfileInt(_T("Settings"), _T("SampleRate"), m_nSampleRate);
	pApp->WriteProfileInt(_T("Settings"), _T("SampleFormat"), m_nSampleFormat);
	pApp->WriteProfileInt(_T("Settings"), _T("Priority"), m_nPriority);

	CDialog::OnDestroy();
}

void CRecordDlg::OnRecord() 
{
	if (m_bRecording)
	{
		m_ctrlRecord.SetWindowText("&Start Recording");
		m_RecordThread->PostThreadMessage(WM_RECORDSOUND_STOPRECORDING, 0, 0L);

		m_bRecording = FALSE;
	}
	else
	{
		const int nSampleRates[] = { 8000, 11025, 22050, 44100, 48000, 96000 };
		const DWORD dwPrios[] = {
			IDLE_PRIORITY_CLASS,
			BELOW_NORMAL_PRIORITY_CLASS,
			NORMAL_PRIORITY_CLASS,
			ABOVE_NORMAL_PRIORITY_CLASS,
			HIGH_PRIORITY_CLASS,
			REALTIME_PRIORITY_CLASS
		};

		m_ctrlRecord.SetWindowText("&Stop Recording");

		UpdateData(TRUE);

		//m_nDevice = 0;
		m_WaveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
		m_WaveFormatEx.nChannels = (m_nSampleFormat & 1) ? 2 : 1;
		m_WaveFormatEx.wBitsPerSample = (m_nSampleFormat < 2) ? 8 : 16;
		m_WaveFormatEx.cbSize = 0;
		m_WaveFormatEx.nSamplesPerSec = nSampleRates[m_nSampleRate];
		m_WaveFormatEx.nAvgBytesPerSec = m_WaveFormatEx.nSamplesPerSec * (m_WaveFormatEx.wBitsPerSample / 8);
		m_WaveFormatEx.nBlockAlign = (m_WaveFormatEx.wBitsPerSample / 8) * m_WaveFormatEx.nChannels;

		::SetPriorityClass(::GetCurrentProcess(), dwPrios[m_nPriority]);

		m_RecordThread->PostThreadMessage(WM_RECORDSOUND_STARTRECORDING, (WPARAM) m_nDevice, (LPARAM) &m_WaveFormatEx);

		m_bRecording = TRUE;
	}
}



void CRecordDlg::OnPlay() 
{
	// TODO: Add your control notification handler code here
	if (m_PlayMMSound)
	{
		m_PlayMMSound->PostThreadMessage(WM_PLAYMMSOUND_PLAYSOUNDPTR, 0, (LPARAM) m_PlayThread);
		m_PlayMMSound->PostThreadMessage(WM_PLAYMMSOUND_PLAYFILE, 0, (LPARAM) "sound.wav");
	}
}

/* 
글쓴이:Bev M Ewen-Smith (coaa@mail.telepac.pt)
제목:Re: mixer API 
이 목록안에 있는 유일한 글입니다.  
View: Original Format 
뉴스그룹:comp.os.ms-windows.programmer.multimedia
날짜:2000/05/03 
 

You could try this - it works for me but beware that the names of the inputs
vary from installation to installation.

Regards

Bev

--
                           Bev and Jan Ewen-Smith
COAA, sitio do Poio, Mexilhoeira Grande 8500-149, Portugal
   Tel 00 351 282 471180        Fax 00 351 282 471516
><>        coaa@mail.telepac.pt        www.ip.pt/coaa       ><>

-----
 // set mixer to line
 SelectSource("Auxiliary"); // or line in or auxiliary or analog
 SetVolume("Auxiliary",volumesetting);
 UnMute("Auxiliary"); // or "_LINE"
 // Auxiliary/Line-In/Analog
 // Microphone
 // CD/CD Record
 // Wave/Wave Record
--------
 */

void
CRecordDlg::UnMute (HMIXER hmx, char *componentsource)
{
  // Open the mixer device
  //HMIXER hmx;
  //mixerOpen   (&hmx, 0, 0, 0, 0);

  // Get the line info for the wave in destination line
  MIXERLINE mxl;
  mxl.cbStruct = sizeof (mxl);
  mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
  mixerGetLineInfo ((HMIXEROBJ) hmx, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE);

  // Now find the microphone (or whatever) source line connected to this wave in
  // destination
  DWORD cConnections = mxl.cConnections;
  for (DWORD j = 0; j < cConnections; j++)
    {
      mxl.dwSource = j;
      mixerGetLineInfo ((HMIXEROBJ) hmx, &mxl, MIXER_GETLINEINFOF_SOURCE);
      if (strcmp (componentsource, mxl.szName) == 0)
        break;
    }
  // Find a mute control, if any, of the microphone line
  LPMIXERCONTROL pmxctrl = (LPMIXERCONTROL) malloc (sizeof (MIXERCONTROL));
  MIXERLINECONTROLS mxlctrl = {
    sizeof (mxlctrl),   
    mxl.dwLineID,
    MIXERCONTROL_CONTROLTYPE_MUTE, 
    1,
    sizeof (MIXERCONTROL), 
    pmxctrl
  };
  if (!mixerGetLineControls ((HMIXEROBJ) hmx, &mxlctrl, MIXER_GETLINECONTROLSF_ONEBYTYPE))
    {
      // Found, so proceed
      DWORD cChannels = mxl.cChannels;
      if (MIXERCONTROL_CONTROLF_UNIFORM & pmxctrl->fdwControl)
        cChannels = 1;

      LPMIXERCONTROLDETAILS_BOOLEAN pbool = (LPMIXERCONTROLDETAILS_BOOLEAN) malloc (cChannels * sizeof (MIXERCONTROLDETAILS_BOOLEAN));
      MIXERCONTROLDETAILS mxcd = { 
        sizeof (mxcd), 
        pmxctrl->dwControlID,
        cChannels, 
        (HWND) 0,
        sizeof (MIXERCONTROLDETAILS_BOOLEAN),
        (LPVOID) pbool
      };
      mixerGetControlDetails ((HMIXEROBJ) hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);
      // Unmute the microphone line (for both channels)
      pbool[0].fValue = pbool[cChannels - 1].fValue = 0;
      mixerSetControlDetails ((HMIXEROBJ) hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

      free (pmxctrl);
      free (pbool);
    }
  else
    free (pmxctrl);

  //mixerClose (hmx);
}

void
CRecordDlg::SetVolume (HMIXER hmx, char *componentsource, int level)
{
  // Open the mixer device
  //HMIXER hmx;
  //mixerOpen   (&hmx, 0, 0, 0, 0);

  // Get the line info for the wave in destination line
  MIXERLINE mxl;
  mxl.cbStruct = sizeof (mxl);
  mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
  mixerGetLineInfo ((HMIXEROBJ) hmx, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE);

  // Now find the microphone source line connected to this wave in
  // destination
  DWORD cConnections = mxl.cConnections;
  for (DWORD j = 0; j < cConnections; j++)
    {
      mxl.dwSource = j;
      mixerGetLineInfo ((HMIXEROBJ) hmx, &mxl, MIXER_GETLINEINFOF_SOURCE);
      // if (componentsource == mxl.dwComponentType)
      if (strcmp (componentsource, mxl.szName) == 0)    // TEST naming
        break;
    }
  // Find a volume control, if any, of the microphone line
  LPMIXERCONTROL pmxctrl = (LPMIXERCONTROL) malloc (sizeof (MIXERCONTROL));
  MIXERLINECONTROLS mxlctrl = {
    sizeof (mxlctrl),
    mxl.dwLineID,
    MIXERCONTROL_CONTROLTYPE_VOLUME,
    1,
    sizeof (MIXERCONTROL), 
    pmxctrl
  };
  if (!mixerGetLineControls ((HMIXEROBJ) hmx, &mxlctrl, MIXER_GETLINECONTROLSF_ONEBYTYPE))
    {
      // Found!
      DWORD cChannels = mxl.cChannels;
      if (MIXERCONTROL_CONTROLF_UNIFORM & pmxctrl->fdwControl)
        cChannels = 1;

      LPMIXERCONTROLDETAILS_UNSIGNED pUnsigned = (LPMIXERCONTROLDETAILS_UNSIGNED) malloc (cChannels * sizeof (MIXERCONTROLDETAILS_UNSIGNED));
      MIXERCONTROLDETAILS mxcd = { 
        sizeof (mxcd),
        pmxctrl->dwControlID,
        cChannels,
        (HWND) 0,
        sizeof (MIXERCONTROLDETAILS_UNSIGNED),
        (LPVOID) pUnsigned
      };
      mixerGetControlDetails ((HMIXEROBJ) hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);
      // Set the volume to the middle  (for both channels as needed)
      pUnsigned[0].dwValue = pUnsigned[cChannels - 1].dwValue = (DWORD) ((pmxctrl->Bounds.dwMinimum + pmxctrl->Bounds.dwMaximum) * level / 20.0);
      mixerSetControlDetails ((HMIXEROBJ) hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

      free (pmxctrl);
      free (pUnsigned);
    }
  else
    free (pmxctrl);
  
  //mixerClose (hmx);
}

void
CRecordDlg::SelectSource (HMIXER hmx, char *sourcename)
{
  // Open the mixer device
  //HMIXER hmx;
  //mixerOpen   (&hmx, 0, 0, 0, 0);

  // Get the line info for the wave in destination line
  MIXERLINE mxl;
  mxl.cbStruct = sizeof (mxl);
  mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
  mixerGetLineInfo ((HMIXEROBJ) hmx, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE);

  // Find a LIST control, if any, for the wave in line
  LPMIXERCONTROL pmxctrl = (LPMIXERCONTROL) malloc (mxl.cControls * sizeof (MIXERCONTROL));
  MIXERLINECONTROLS mxlctrl = {
    sizeof (mxlctrl),   
    mxl.dwLineID, 
    0,
    mxl.cControls, 
    sizeof (MIXERCONTROL),
    pmxctrl
  };
  mixerGetLineControls ((HMIXEROBJ) hmx, &mxlctrl, MIXER_GETLINECONTROLSF_ALL);

  // Now walk through each control to find a type of LIST control. This
  // can be either Mux, Single-select, Mixer or Multiple-select.
  DWORD i;
  for (i = 0; i < mxl.cControls; i++)
    if ((pmxctrl[i].dwControlType & MIXERCONTROL_CT_CLASS_MASK) == MIXERCONTROL_CT_CLASS_LIST)
      break;

  if (i < mxl.cControls)
    {                           // Found a LIST control
      // Check if the LIST control is a Mux or Single-select type
      BOOL bOneItemOnly = FALSE;
      switch (pmxctrl[i].dwControlType)
        {
        case MIXERCONTROL_CONTROLTYPE_MUX:
        case MIXERCONTROL_CONTROLTYPE_SINGLESELECT:
          bOneItemOnly = TRUE;
        }

      DWORD cChannels = mxl.cChannels;
	  DWORD cMultipleItems = 0;
      if (pmxctrl[i].fdwControl & MIXERCONTROL_CONTROLF_UNIFORM)
        cChannels = 1;
      if (pmxctrl[i].fdwControl & MIXERCONTROL_CONTROLF_MULTIPLE)
        cMultipleItems = pmxctrl[i].cMultipleItems;

      // Get the text description of each item

      LPMIXERCONTROLDETAILS_LISTTEXT plisttext = (LPMIXERCONTROLDETAILS_LISTTEXT) malloc (cChannels * cMultipleItems * sizeof (MIXERCONTROLDETAILS_LISTTEXT));
      MIXERCONTROLDETAILS mxcd = {
        sizeof (mxcd),
        pmxctrl[i].
        dwControlID,
        cChannels,
        (HWND) cMultipleItems, 
        sizeof (MIXERCONTROLDETAILS_LISTTEXT),
        (LPVOID) plisttext
      };
      mixerGetControlDetails ((HMIXEROBJ) hmx, &mxcd, MIXER_GETCONTROLDETAILSF_LISTTEXT);

      // Now get the value for each item
      LPMIXERCONTROLDETAILS_BOOLEAN plistbool = (LPMIXERCONTROLDETAILS_BOOLEAN) malloc (cChannels * cMultipleItems * sizeof (MIXERCONTROLDETAILS_BOOLEAN));
      mxcd.cbDetails = sizeof (MIXERCONTROLDETAILS_BOOLEAN);
      mxcd.paDetails = plistbool;
      mixerGetControlDetails ((HMIXEROBJ) hmx, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);

      // Select the "Microphone" item
      for (DWORD j = 0; j < cMultipleItems; j += cChannels)
        {
          if (strcmp (plisttext[j].szName, sourcename) == 0)
            // Select   it for both left and right channels
            plistbool[j].fValue = plistbool[j + cChannels - 1].fValue = 1;
          //else if (bOneItemOnly) ---- deselect the others
          else
            // Mux or   Single-select allows only one item to be selected
            // so   clear other items as necessary
            plistbool[j].fValue = plistbool[j + cChannels - 1].fValue = 0;
        }

      // Now actually set the new values in
      mixerSetControlDetails ((HMIXEROBJ) hmx, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);

      free (pmxctrl);
      free (plisttext);
      free (plistbool);
    }
  else
    free (pmxctrl);
  
  //mixerClose (hmx);
}
