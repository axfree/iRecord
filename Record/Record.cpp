// record.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include <mmsystem.h>
#include "record.h"
#include "recordDlg.h"
#include "PlaySound.h"
#include "RecordSound.h"
#include "WriteSoundFile.h"
#include "PlayMMSound.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecordApp

BEGIN_MESSAGE_MAP(CRecordApp, CWinApp)
	//{{AFX_MSG_MAP(CRecordApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecordApp construction

CRecordApp::CRecordApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

}

/////////////////////////////////////////////////////////////////////////////
// The one and only CRecordApp object

CRecordApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CRecordApp initialization
int CRecordApp::ExitInstance()
{
	if (m_pPlayMMSound)
	{
		m_pPlayMMSound->PostThreadMessage(WM_PLAYMMSOUND_ENDTHREAD,0,0);
		::WaitForSingleObject(m_pPlayMMSound->m_hThread, 5000);
		m_pPlayMMSound = NULL;
	}

	if (m_pPlaySound)
	{
		m_pPlaySound->PostThreadMessage(WM_PLAYSOUND_ENDTHREAD,0,0);
		::WaitForSingleObject(m_pPlaySound->m_hThread, 5000);
		m_pPlaySound = NULL;

	}

	if (m_pRecordSound)
	{
		m_pRecordSound->PostThreadMessage(WM_RECORDSOUND_ENDTHREAD,0,0);
		::WaitForSingleObject(m_pRecordSound->m_hThread, 5000);
		m_pRecordSound = NULL;
	}

	if (m_pWriteSound)
	{
		m_pWriteSound->PostThreadMessage(WM_WRITESOUNDFILE_ENDTHREAD,0,0);
		::WaitForSingleObject(m_pWriteSound->m_hThread, 5000);
		m_pWriteSound = NULL;
	}
	
	return 0;
}

BOOL CRecordApp::InitInstance()
{
	//SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	SetRegistryKey(_T("Made21"));

	InitRecording();
	InitPlaying();
	InitWriting();
	InitPlayMMSound();
	
	if (m_pRecordSound)
	{
		m_pRecordSound->PostThreadMessage(WM_RECORDSOUND_SOUNDPLAYER, (WPARAM)0, (LPARAM) m_pPlaySound);
#ifdef USE_WRITESOUND
		m_pRecordSound->PostThreadMessage(WM_RECORDSOUND_WRITERTHREAD, (WPARAM)0, (LPARAM) m_pWriteSound);
#endif
	}
	


	CRecordDlg dlg;
	m_pMainWnd = &dlg;

	dlg.m_RecordThread = m_pRecordSound;
	dlg.m_PlayThread = m_pPlaySound;
	dlg.m_PlayMMSound = m_pPlayMMSound;

	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

BOOL CRecordApp::InitRecording()
{
	m_pRecordSound = new CRecordSound();
	m_pRecordSound->CreateThread();
	//m_pRecordSound->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
	return TRUE;
}

BOOL CRecordApp::InitPlayMMSound()
{
	m_pPlayMMSound = new CPlayMMSound();
	m_pPlayMMSound->CreateThread();
	return TRUE;
}

BOOL CRecordApp::InitWriting()
{
	m_pWriteSound = new CWriteSoundFile();
	m_pWriteSound->CreateThread();
	return TRUE;
}

BOOL CRecordApp::InitPlaying()
{
	m_pPlaySound = new CPlaySound();
	m_pPlaySound->CreateThread();
	//m_pPlaySound->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
	return TRUE;
}
