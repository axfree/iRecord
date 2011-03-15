// PlayMMSound.cpp : implementation file
//

#include "stdafx.h"
#include "record.h"
#include "PlaySound.h"
#include "PlayMMSound.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlayMMSound

IMPLEMENT_DYNCREATE(CPlayMMSound, CWinThread)

CPlayMMSound::CPlayMMSound()
{
	m_pPlaySound = NULL;
	strcpy(m_FileName,"");
	ZeroMemory(&m_MMCkInfoParent,sizeof(MMCKINFO));
	ZeroMemory(&m_MMCkInfoChild,sizeof(MMCKINFO));
	m_SoundBuffer = NULL;
	m_pSoundThread = NULL;
	m_pStopEvent = new CEvent(FALSE,TRUE);
}

CPlayMMSound::~CPlayMMSound()
{
	if (m_SoundBuffer)
		delete m_SoundBuffer;
	if (m_pStopEvent)
		delete m_pStopEvent;
}

BOOL CPlayMMSound::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

int CPlayMMSound::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CPlayMMSound, CWinThread)
	//{{AFX_MSG_MAP(CPlayMMSound)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_THREAD_MESSAGE(WM_PLAYMMSOUND_PLAYFILE, PlaySoundFile)
	ON_THREAD_MESSAGE(WM_PLAYMMSOUND_CLOSEFILE, CloseSoundFile)
	ON_THREAD_MESSAGE(WM_PLAYMMSOUND_PLAYSOUNDPTR, OnPlaySoundPtr)
	ON_THREAD_MESSAGE(WM_PLAYMMSOUND_ENDTHREAD, OnEndThread)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayMMSound message handlers

void/*LRESULT*/  CPlayMMSound::PlaySoundFile(WPARAM wParam,LPARAM lParam)
{
	char *pFileName = (char*) lParam;
	strcpy(m_FileName,pFileName);
	if (!OpenSoundFile(m_FileName))
		return/* FALSE*/;
	return /*TRUE*/;
}

BOOL CPlayMMSound::OpenSoundFile(CString csFileName)
{
	// code taken from Visual C++ Multimedia -- Aitken and Jarol p 122
	
    CloseSoundFile(0,0);
	m_hmmio = mmioOpen((LPSTR)(LPCTSTR)csFileName,NULL,MMIO_READ);
	if (!m_hmmio)
	{
		AfxMessageBox("unable to open Sound MM File");
		return FALSE;
	}
	m_MMCkInfoParent.fccType = mmioFOURCC('W','A','V','E');
	int err = mmioDescend(m_hmmio, &m_MMCkInfoParent,NULL,MMIO_FINDRIFF);
	if (err)
	{
		AfxMessageBox("Error descending into file");
		mmioClose(m_hmmio,0);
		m_hmmio = NULL;
		return FALSE;
	}
	m_MMCkInfoChild.ckid = mmioFOURCC('f','m','t',' ');
	err = mmioDescend(m_hmmio,&m_MMCkInfoChild,&m_MMCkInfoParent,MMIO_FINDCHUNK);
	if (err)
	{
		AfxMessageBox("Error descending in file");
		mmioClose(m_hmmio,0);
		m_hmmio = NULL;
		return FALSE;
	}
	DWORD bytesRead = mmioRead(m_hmmio,(LPSTR)&m_PCMWaveFmtRecord,m_MMCkInfoChild.cksize);
	if (bytesRead < 0)
	{
		AfxMessageBox("Error reading PCM wave format record");
		mmioClose(m_hmmio,0);
		return FALSE;
	}
	
	// open output sound file
	err = mmioAscend(m_hmmio,&m_MMCkInfoChild,0);
	if (err)
	{
		AfxMessageBox("Error ascending in File");
		mmioClose(m_hmmio,0);
		m_hmmio = NULL;
		return FALSE;
	}
	m_MMCkInfoChild.ckid = mmioFOURCC('d','a','t','a');
	err = mmioDescend(m_hmmio,&m_MMCkInfoChild,
		&m_MMCkInfoParent,MMIO_FINDCHUNK);
	if (err)
	{
		AfxMessageBox("error reading data chunk");
		mmioClose(m_hmmio,0);
		m_hmmio = NULL;
		return FALSE;
	}
	m_BytesToRead = m_PCMWaveFmtRecord.nChannels*
		(m_PCMWaveFmtRecord.wBitsPerSample/sizeof(BYTE))
		*m_PCMWaveFmtRecord.nBlockAlign*100;
	if (m_BytesToRead > m_MMCkInfoChild.cksize)
		 m_BytesToRead = m_MMCkInfoChild.cksize;

	m_bContinuePlaying = TRUE;
	m_pStopEvent->ResetEvent();
	m_pSoundThread = AfxBeginThread(PlaySound,this);

	return TRUE;
}

BYTE* CPlayMMSound::ReadSoundFile(int*dwBytesRead)
{
	if (m_BytesToRead <= 0)
		return NULL;
	if (!m_hmmio)
		return NULL;
	TRACE("ReadSoundFile\n");
	BYTE * pSoundBuffer = new BYTE[m_BytesToRead];
	if (!pSoundBuffer)
		return NULL;
	DWORD dwRetc = mmioRead(m_hmmio,(LPSTR)pSoundBuffer, m_BytesToRead);
	if (dwRetc == -1)
	{
		AfxMessageBox("Error reading WAVE file\n");
		if (pSoundBuffer)
			delete pSoundBuffer;
		return NULL;
	}
	else if (dwRetc == 0)
	{
		CloseSoundFile(0,0);
		m_bContinuePlaying = FALSE;
	}
	if (dwBytesRead)
		*dwBytesRead = dwRetc;
	char debugBuffer[MAX_PATH];
	sprintf(debugBuffer,"read %d bytes\n",dwRetc);
	TRACE(debugBuffer);
	return pSoundBuffer;
}

void  CPlayMMSound::CloseSoundFile(WPARAM wParam,LPARAM lParam)
{
	if (m_pStopEvent)
		m_pStopEvent->SetEvent();
	TRACE("STOP EVENT SET\n");
	m_bContinuePlaying = FALSE;
	if (m_hmmio)
	{
		mmioClose(m_hmmio,0);
		m_hmmio = NULL;
	}
}

void  CPlayMMSound::OnPlaySoundPtr(WPARAM wParam,LPARAM lParam)
{
	m_pPlaySound = (CPlaySound*) lParam;
}

void CPlayMMSound::OnEndThread(WPARAM wParam, LPARAM lParam)
{
	CloseSoundFile(0,0);

	::PostQuitMessage(0);
}

UINT CPlayMMSound::PlaySound( LPVOID pParam )
{
	CPlayMMSound* pPlayMMSound = (CPlayMMSound*) pParam;
	if (pPlayMMSound &&
			pPlayMMSound->m_pPlaySound)
	{
		pPlayMMSound->m_pPlaySound->PostThreadMessage(WM_PLAYSOUND_STARTPLAYING,GetCurrentThreadId(),(LPARAM)0);
	}

	if (!pPlayMMSound)
		return FALSE;
	if (!pPlayMMSound->m_pPlaySound)
		return FALSE;
	HANDLE hHandle[2];
	hHandle[1] = (HANDLE) pPlayMMSound->m_pPlaySound->m_pSemaphore->m_hObject;
	hHandle[0] = (HANDLE) pPlayMMSound->m_pStopEvent->m_hObject;
	int dwBytesRead = 0;
	BYTE* pSoundBuffer = pPlayMMSound->ReadSoundFile(&dwBytesRead);
	while(pPlayMMSound->m_bContinuePlaying)
	{
		DWORD dwRetc = WaitForMultipleObjects(2,hHandle,FALSE,INFINITE);
		if (dwRetc == WAIT_FAILED)
		{
			DWORD dwRetc = ::GetLastError();
			char errorBuffer[MAX_PATH];
			sprintf(errorBuffer,"WaitForMultipleObjects failed: %d\n",dwRetc);
			TRACE(errorBuffer);
		}
		dwRetc -= WAIT_OBJECT_0;
		if (dwRetc == 0) // stop event;
		{
			TRACE("STOP Event seen\n");
			break;
		}
		if (!pPlayMMSound->m_bContinuePlaying)
			break;
		
		
		WAVEHDR* pWaveHdr = new WAVEHDR;
		ZeroMemory(pWaveHdr,sizeof(WAVEHDR));
		pWaveHdr->lpData = (char*)pSoundBuffer;
		pWaveHdr->dwBufferLength = dwBytesRead;
		if (pPlayMMSound &&
			pPlayMMSound->m_pPlaySound)
		{
			pPlayMMSound->m_pPlaySound->PostThreadMessage(WM_PLAYSOUND_PLAYBLOCK,GetCurrentThreadId(),(LPARAM)pWaveHdr);
		}
		pSoundBuffer = pPlayMMSound->ReadSoundFile(&dwBytesRead);
	}

	if (pPlayMMSound &&
		pPlayMMSound->m_pPlaySound)
	{
	//	pPlayMMSound->m_pPlaySound->PostThreadMessage(WM_PLAYSOUND_STOPPLAYING,GetCurrentThreadId(),(LPARAM)0);
	}

	return TRUE;
}