// RecordSound.cpp : implementation file
//

#include "stdafx.h"
#include <mmsystem.h>
#include "record.h"
#include "RecordSound.h"
#include "PlaySound.h"
#include "WriteSoundFile.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecordSound

IMPLEMENT_DYNCREATE(CRecordSound, CWinThread)


#define MAXINPUTBUFFERS 25

CRecordSound::CRecordSound(int iHertz){
	m_nInputBuffers = 0;
	m_nMaxInputBuffers = MAXINPUTBUFFERS;

	memset(&m_WaveFormatEx,0,sizeof(m_WaveFormatEx));
	m_WaveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
	m_WaveFormatEx.nChannels = 1;
	m_WaveFormatEx.wBitsPerSample = 16;
	m_WaveFormatEx.cbSize = 0;
	m_WaveFormatEx.nSamplesPerSec = iHertz;//20050;
	m_WaveFormatEx.nAvgBytesPerSec = m_WaveFormatEx.nSamplesPerSec
		*(m_WaveFormatEx.wBitsPerSample/8);
	m_WaveFormatEx.nBlockAlign = 
		(m_WaveFormatEx.wBitsPerSample/8)*
		m_WaveFormatEx.nChannels;

	m_bRecording = FALSE;
	m_Player = NULL;
	m_Writer = NULL;
}

CRecordSound::~CRecordSound()
{
}

BOOL CRecordSound::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

int CRecordSound::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CRecordSound, CWinThread)
	//{{AFX_MSG_MAP(CRecordSound)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_THREAD_MESSAGE(WM_RECORDSOUND_STARTRECORDING, OnStartRecording)
	ON_THREAD_MESSAGE(WM_RECORDSOUND_STOPRECORDING, OnStopRecording)
	ON_THREAD_MESSAGE(MM_WIM_DATA, OnSoundData)
	ON_THREAD_MESSAGE(WM_RECORDSOUND_SOUNDPLAYER, OnPtrSoundPlayer)
	ON_THREAD_MESSAGE(WM_RECORDSOUND_ENDTHREAD, OnEndThread)
	ON_THREAD_MESSAGE(WM_RECORDSOUND_WRITERTHREAD,OnPtrSoundWriter)
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CRecordSound message handlers
void CRecordSound::OnPtrSoundPlayer(WPARAM wParam, LPARAM lParam)
{
	m_Player = (CPlaySound*) lParam;
}

void CRecordSound::OnStartRecording(WPARAM wParam, LPARAM lParam)
{
	MMRESULT err = 0;

	if (m_bRecording)
		return;

	m_nDevice = wParam;
	m_WaveFormatEx = *((LPWAVEFORMATEX) lParam);

	// open wavein device
	err = ::waveInOpen( &m_hRecord, m_nDevice /*WAVE_MAPPER*/,
		&m_WaveFormatEx, ::GetCurrentThreadId(), 0, CALLBACK_THREAD);
	if (err )
	{
		char errorbuffer[MAX_PATH];
		char errorbuffer1[MAX_PATH];
		waveInGetErrorText( err, 
							errorbuffer,
							MAX_PATH);
		sprintf(errorbuffer1,"WAVEIN:%x:%s",err,errorbuffer);

		AfxMessageBox(errorbuffer1);
		return;
	}

		
	for (int i=0; i < m_nMaxInputBuffers; i++)
	{
		LPWAVEHDR lpHdr = CreateWaveHeader();
		err = ::waveInPrepareHeader(m_hRecord, lpHdr, sizeof(WAVEHDR));
		err = ::waveInAddBuffer(m_hRecord, lpHdr, sizeof(WAVEHDR));
		m_nInputBuffers++;
	}
	err = ::waveInStart(m_hRecord);
	if (err )
	{
		char errorbuffer[MAX_PATH];
		char errorbuffer1[MAX_PATH];
		waveInGetErrorText( err, 
							errorbuffer,
							MAX_PATH);
		sprintf(errorbuffer1,"WAVEIN:%x:%s",err,errorbuffer);

		AfxMessageBox(errorbuffer1);
		return;
	}

	m_bRecording = TRUE;

	if (m_Player)
		m_Player->PostThreadMessage(WM_PLAYSOUND_STARTPLAYING, 0, (LPARAM) &m_WaveFormatEx);
	
	if (m_Writer)
	{
		PWRITESOUNDFILE pwsf= (PWRITESOUNDFILE) new WRITESOUNDFILE;
		ZeroMemory(pwsf,sizeof(WRITESOUNDFILE));
		char *p = pwsf->lpszFileName;
		strcpy(p,"sound.wav");
		memcpy(&pwsf->waveFormatEx,&m_WaveFormatEx,sizeof(m_WaveFormatEx));
		m_Writer->PostThreadMessage(WM_WRITESOUNDFILE_FILENAME,0,(LPARAM)pwsf);
	}
}

void CRecordSound::OnStopRecording(WPARAM wParam, LPARAM lParam)
{
	MMRESULT err = 0;
	if (!m_bRecording)
		return;

	err = ::waveInStop(m_hRecord);
	if (!err)
		err = ::waveInReset(m_hRecord);
	if (!err)
		m_bRecording = FALSE;
	Sleep(500);
	if (!err)
		err = ::waveInClose(m_hRecord);

	if (m_Player)
		m_Player->PostThreadMessage(WM_PLAYSOUND_STOPPLAYING,0,0);

	if (m_Writer)
		m_Writer->PostThreadMessage(WM_WRITESOUNDFILE_CLOSEFILE,0,0);
}

void CRecordSound::OnSoundData(WPARAM wParam, LPARAM lParam)
{
	LPWAVEHDR lpHdr = (LPWAVEHDR) lParam;

	if (lpHdr)
	{
		short * lpInt = (short*) lpHdr->lpData;
		DWORD cbRecorded = lpHdr->dwBytesRecorded;
		::waveInUnprepareHeader(m_hRecord, lpHdr, sizeof(WAVEHDR));

		ProcessSoundData(lpInt, cbRecorded/sizeof(short));
		if (m_Writer)
		{
			WAVEHDR* pWriteHdr = new WAVEHDR;
			if (!pWriteHdr)
				return;
			memcpy(pWriteHdr,lpHdr,sizeof(WAVEHDR));
			BYTE * pSound = new BYTE[lpHdr->dwBufferLength];
			if (!pSound)
			{
				delete pWriteHdr;
				return;
			}
			memcpy(pSound,lpHdr->lpData,lpHdr->dwBufferLength);
			pWriteHdr->lpData = (char*)pSound;
			m_Writer->PostThreadMessage(WM_WRITESOUNDFILE_WRITEBLOCK, GetCurrentThreadId(), (LPARAM) pWriteHdr);
		}

		if (m_Player)
		{
			m_Player->PostThreadMessage(WM_PLAYSOUND_PLAYBLOCK, GetCurrentThreadId(), (LPARAM) lpHdr);
		}
		else
		{
			delete lpInt;
			delete lpHdr;
		}
		
		//char debugbuffer[256];
		//sprintf(debugbuffer, "SOUND BUFFER returned: %d\n",cbRecorded);
		//TRACE(debugbuffer);
		
		if (m_bRecording)
		{
			LPWAVEHDR lpHdr = CreateWaveHeader();
			::waveInPrepareHeader(m_hRecord, lpHdr, sizeof(WAVEHDR));
			::waveInAddBuffer(m_hRecord, lpHdr, sizeof(WAVEHDR));
			m_nInputBuffers++;
		}
	}
}

LPWAVEHDR CRecordSound::CreateWaveHeader()
{
	LPWAVEHDR lpHdr = new WAVEHDR;
	ZeroMemory(lpHdr, sizeof(WAVEHDR));

	lpHdr->dwBufferLength = (m_WaveFormatEx.nBlockAlign * SOUNDSAMPLES);
	lpHdr->lpData = (char *) new BYTE[lpHdr->dwBufferLength];

	return lpHdr;
}

void CRecordSound::ProcessSoundData(short* sound, DWORD dwSamples)
{
}

void CRecordSound::OnEndThread(WPARAM wParam, LPARAM lParam)
{
	if (m_bRecording)
	{
		OnStopRecording(0, 0);
	}
	::PostQuitMessage(0);
}

void CRecordSound::OnPtrSoundWriter(WPARAM wParam, LPARAM lParam)
{
	m_Writer = (CWriteSoundFile*) lParam;
}
	