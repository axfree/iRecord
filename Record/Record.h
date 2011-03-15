// record.h : main header file for the RECORD application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "RecordSound.h"
#include "PlaySound.h"
#include "PlayMMSound.h"



/////////////////////////////////////////////////////////////////////////////
// CRecordApp:
// See record.cpp for the implementation of this class
//

class CRecordApp : public CWinApp
{
public:
	CRecordApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecordApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL
	virtual int ExitInstance();

// Implementation

	//{{AFX_MSG(CRecordApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	protected:
		BOOL m_bRecording;
		CRecordSound* m_pRecordSound;
		CPlaySound* m_pPlaySound;
		CWriteSoundFile* m_pWriteSound;
		CPlayMMSound* m_pPlayMMSound;
		
	protected:
		BOOL InitRecording();
		BOOL InitPlaying();
		BOOL InitWriting();
		BOOL InitPlayMMSound();
		

};

