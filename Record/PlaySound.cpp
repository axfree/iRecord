// PlaySound.cpp : implementation file
//

#include "stdafx.h"
#include "winbase.h"
#include <mmsystem.h>
#include <math.h>



#include "record.h"
#include "PlaySound.h"

#include "RecordDlg.h"




#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlaySound

IMPLEMENT_DYNCREATE(CPlaySound, CWinThread)

#define BLOCKSAHEAD 3

CPlaySound::CPlaySound(int iHertz)
{
	m_nOutputBuffers = 0;
	m_nMaxOutputBuffers = 0;
	memset(&m_WaveFormatEx,0x00,sizeof(m_WaveFormatEx));
	m_WaveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
	m_WaveFormatEx.nChannels = 1;
	m_WaveFormatEx.wBitsPerSample = 16;
	m_WaveFormatEx.cbSize = 0;
	m_WaveFormatEx.nSamplesPerSec = iHertz;
	m_WaveFormatEx.nAvgBytesPerSec = m_WaveFormatEx.nSamplesPerSec
		*(m_WaveFormatEx.wBitsPerSample/8);
	m_WaveFormatEx.nBlockAlign = 
		(m_WaveFormatEx.wBitsPerSample/8)*
		m_WaveFormatEx.nChannels;
	m_pSemaphore = new CSemaphore(BLOCKSAHEAD, BLOCKSAHEAD);

	m_bPlay = FALSE;
}

CPlaySound::~CPlaySound()
{
	if (m_pSemaphore)
		delete m_pSemaphore;
}

BOOL CPlaySound::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

int CPlaySound::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CPlaySound, CWinThread)
	//{{AFX_MSG_MAP(CPlaySound)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_THREAD_MESSAGE(WM_PLAYSOUND_STARTPLAYING, OnStartPlaying)
	ON_THREAD_MESSAGE(WM_PLAYSOUND_STOPPLAYING, OnStopPlaying)
	ON_THREAD_MESSAGE(WM_PLAYSOUND_PLAYBLOCK, OnWriteSoundData)
	ON_THREAD_MESSAGE(MM_WOM_DONE, OnEndPlaySoundData)
	ON_THREAD_MESSAGE(WM_PLAYSOUND_ENDTHREAD, OnEndThread)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlaySound message handlers
void CPlaySound::OnStartPlaying(WPARAM wParam, LPARAM lParam)
{
	MMRESULT err = 0;

	if (m_bPlay)
		return;

	m_WaveFormatEx = *((LPWAVEFORMATEX) lParam);

	// open wavein device
	err = ::waveOutOpen( &m_hPlay, WAVE_MAPPER,
		&m_WaveFormatEx, ::GetCurrentThreadId(), 0, CALLBACK_THREAD);
	if (err)
	{
		char errorbuffer[MAX_PATH];
		char errorbuffer1[MAX_PATH];
		waveOutGetErrorText( err, 
							errorbuffer,
							MAX_PATH);
		sprintf(errorbuffer1,"WAVEOUT:%x:%s",err,errorbuffer);
		AfxMessageBox(errorbuffer1);  
	}


	if (!err)
	{
		m_bPlay = TRUE;
	}
}

void CPlaySound::OnStopPlaying(WPARAM wParam, LPARAM lParam)
{
	MMRESULT err = 0;
	if (!m_bPlay)
		return;

	if (m_bPlay)
	{
		err = ::waveOutReset(m_hPlay);
		
		if (!err)
			m_bPlay = FALSE;
		Sleep(500);
		if (!err)
			err = ::waveOutClose(m_hPlay);
	}
}

void CPlaySound::OnEndPlaySoundData(WPARAM wParam, LPARAM lParam)
{
	LPWAVEHDR lpHdr = (LPWAVEHDR) lParam;
	if (lpHdr)
	{
		::waveOutUnprepareHeader(m_hPlay, lpHdr, sizeof(WAVEHDR));
	
		if (lpHdr->lpData)
			delete ((BYTE*) lpHdr->lpData);
		delete lpHdr;
		m_pSemaphore->Unlock();
	}
}

void CPlaySound::OnWriteSoundData(WPARAM wParam, LPARAM lParam)
{
	LPWAVEHDR lpHdr = (LPWAVEHDR) lParam;
	MMRESULT err = 0;

	if (lpHdr)
	{
		//char debugbuffer[256];
		//sprintf(debugbuffer, "SOUND BUFFER written: %d, %d\n",lpHdr->dwBufferLength,m_nOutputBuffers);
		//TRACE(debugbuffer);

		if (m_bPlay)
		{
			short * lpInt = (short *) lpHdr->lpData;
			DWORD dwSamples = lpHdr->dwBufferLength/sizeof(short);

			ProcessSoundData(lpInt, dwSamples);

			err = ::waveOutPrepareHeader(m_hPlay, lpHdr, sizeof(WAVEHDR));
			if (err)
				TRACE("error from waveoutprepareheader\n");
			
			err = ::waveOutWrite(m_hPlay, lpHdr, sizeof(WAVEHDR));
			if (err)
				TRACE("error from waveoutwrite\n");

			m_nOutputBuffers++;
		}
	}
}

LPWAVEHDR CPlaySound::CreateWaveHeader()
{
	LPWAVEHDR lpHdr = new WAVEHDR;
	ZeroMemory(lpHdr, sizeof(WAVEHDR));

	lpHdr->dwBufferLength = (m_WaveFormatEx.nBlockAlign * SOUNDSAMPLES);
	lpHdr->lpData = (char *) new BYTE[lpHdr->dwBufferLength];

	return lpHdr;
}

void CPlaySound::ProcessSoundData(short *sound, DWORD dwSamples)
{
}

void CPlaySound::OnEndThread(WPARAM wParam, LPARAM lParam)
{
	if (m_bPlay)
		OnStopPlaying(0,0);

	::PostQuitMessage(0);
}