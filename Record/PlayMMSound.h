#if !defined(AFX_PLAYMMSOUND_H__F3383123_E3DD_11D3_8685_806A49C10000__INCLUDED_)
#define AFX_PLAYMMSOUND_H__F3383123_E3DD_11D3_8685_806A49C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PlayMMSound.h : header file
//
#define WM_PLAYMMSOUND_PLAYFILE		(WM_USER+301)
#define WM_PLAYMMSOUND_CLOSEFILE	(WM_USER+302)
#define WM_PLAYMMSOUND_PLAYSOUNDPTR	(WM_USER+303)
#define WM_PLAYMMSOUND_ENDTHREAD	(WM_USER+304)


/////////////////////////////////////////////////////////////////////////////
// CPlayMMSound thread

class CPlayMMSound : public CWinThread
{
	DECLARE_DYNCREATE(CPlayMMSound)
public:
	BYTE* ReadSoundFile(int*dwBytesRead);
	CPlayMMSound();           // protected constructor used by dynamic creation

protected:
	BOOL OpenSoundFile(CString csFileName);
	void OnEndThread(WPARAM wParam, LPARAM lParam);



// Attributes
public:
	CEvent* m_pStopEvent;
	CPlaySound* m_pPlaySound;
	BOOL m_bContinuePlaying;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayMMSound)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL
	static UINT PlaySound( LPVOID pParam );


// Implementation
protected:
	virtual ~CPlayMMSound();

	// Generated message map functions
	//{{AFX_MSG(CPlayMMSound)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG


	afx_msg void/*LRESULT*/ PlaySoundFile(WPARAM wParam, LPARAM lParam);
	afx_msg void/*LRESULT*/ CloseSoundFile(WPARAM wParam, LPARAM lParam);
	afx_msg void/*LRESULT*/ OnPlaySoundPtr(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()


	char m_FileName[MAX_PATH];
	MMCKINFO m_MMCkInfoParent;
	MMCKINFO m_MMCkInfoChild;
	HMMIO m_hmmio;
	WAVEFORMATEX m_PCMWaveFmtRecord;
	BYTE m_WaveFormatExtras[MAX_PATH]; // for non PCM waveformatex there are extra bytes at the end
	BYTE * m_SoundBuffer;
	CWinThread* m_pSoundThread;
	int m_BytesToRead;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYMMSOUND_H__F3383123_E3DD_11D3_8685_806A49C10000__INCLUDED_)
