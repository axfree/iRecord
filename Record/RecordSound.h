#if !defined(AFX_RECORDSOUND_H__5260C01B_03B2_11D2_A421_FC4B2C882A60__INCLUDED_)
#define AFX_RECORDSOUND_H__5260C01B_03B2_11D2_A421_FC4B2C882A60__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// RecordSound.h : header file
//

#define WM_RECORDSOUND_STARTRECORDING	(WM_USER+500)
#define WM_RECORDSOUND_STOPRECORDING	(WM_USER+501)
#define WM_RECORDSOUND_SOUNDPLAYER		(WM_USER+502)
#define WM_RECORDSOUND_ENDTHREAD		(WM_USER+503)
#define WM_RECORDSOUND_WRITERTHREAD		(WM_USER+504)


/////////////////////////////////////////////////////////////////////////////
// CRecordSound thread

#include <mmsystem.h>

#include "PlaySound.h"
#include "WriteSoundFile.h"

#define SOUNDSAMPLES 256 /*100*//*1000*/

class CRecordSound : public CWinThread
{
	DECLARE_DYNCREATE(CRecordSound)
public:
	CRecordSound(int iHertz=44100);           // protected constructor used by dynamic creation
	LPWAVEHDR CreateWaveHeader();
	virtual void ProcessSoundData(short * sound, DWORD dwSamples);
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecordSound)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CRecordSound();

	// Generated message map functions
	//{{AFX_MSG(CRecordSound)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	afx_msg void OnStartRecording(WPARAM wParam, LPARAM lParam);
	afx_msg void OnStopRecording(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSoundData(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPtrSoundPlayer(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPtrSoundWriter(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEndThread(WPARAM wParam, LPARAM lParam);


	DECLARE_MESSAGE_MAP()
protected:
	HWAVEIN m_hRecord;
	int m_nInputBuffers;
	int m_nMaxInputBuffers;
	int m_nDevice;
	WAVEFORMATEX m_WaveFormatEx; 
	BOOL m_bRecording;
	CPlaySound* m_Player;
	CWriteSoundFile* m_Writer;
	CWinThread *m_pSoundThread;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECORDSOUND_H__5260C01B_03B2_11D2_A421_FC4B2C882A60__INCLUDED_)
