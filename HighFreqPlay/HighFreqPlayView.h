// HighFreqPlayView.h : interface of the CHighFreqPlayView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Fourier.h"
#include "pm.h"
#include "Pmpolicy.h"
class CHighFreqPlayView : 
	public CDialogImpl<CHighFreqPlayView>
{
public:
	enum { IDD = IDD_MAIN_WND };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	BEGIN_MSG_MAP_EX(CHighFreqPlayView)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDC_BUTTON_ID, OnBnIDClick)
	END_MSG_MAP()

	CEdit edit_id;
	CTrackBarCtrl slider_volume;
// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	void OnPaint(CDCHandle dc)
	{
		CPaintDC ddc(m_hWnd);
	}
	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		edit_id=GetDlgItem(IDC_EDIT_ID);
		slider_volume=GetDlgItem(IDC_SLIDER_VOLUME);
		slider_volume.SetRange(40,140);
		slider_volume.SetThumbLength(20);
		slider_volume.SetPos(40);

		unsigned int posid=0;
		unsigned int volume=0;
		FILE* optfile=_wfopen(selfPath+L"set.txt",L"r");
		if(optfile)
		{
			fscanf(optfile,"%u\n",&posid);
			fscanf(optfile,"%u\n",&volume);
			fclose(optfile);
		}
		if(volume)
			slider_volume.SetPos(volume);
		BuildBuffer(posid);

		CAtlString idstr;
		idstr.Format(_T("%x"),posid);
		edit_id.SetWindowText(idstr);

		return TRUE;
	}
	LRESULT OnBnIDClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAtlString str;
		edit_id.GetWindowText(str);
		wchar_t *end=NULL;
		unsigned long res=wcstoul(str,&end,16);
		BuildBuffer(res);

		FILE* optfile=_wfopen(selfPath+L"set.txt",L"w");
		if(optfile)
		{
			fprintf(optfile,"%d\n",res);
			fprintf(optfile,"%d\n",slider_volume.GetPos());
			fclose(optfile);
		}

		return S_OK;
	}

	WAVEFORMATEX wFormatEx;
#define SampleCount (8192)
	short outsample[SampleCount];
	HWAVEOUT hwo;
	WAVEHDR whdr,whdr1;
	UINT devId;
	CHighFreqPlayView():waveout_runing(false)
	{
		PowerPolicyNotify(PPN_UNATTENDEDMODE, 1);
		ZeroMemory(outsample,sizeof(outsample));
		MMRESULT mmres;
		devId=0;

		wFormatEx.wFormatTag = WAVE_FORMAT_PCM;
		wFormatEx.nChannels = 1;
		wFormatEx.wBitsPerSample = 16;
		wFormatEx.nSamplesPerSec = 44100;
		wFormatEx.nBlockAlign = wFormatEx.wBitsPerSample * wFormatEx.nChannels / 8;
		wFormatEx.nAvgBytesPerSec = wFormatEx.nSamplesPerSec* wFormatEx.nBlockAlign;

		// Open audio device
		for (devId = 0; devId < waveOutGetNumDevs(); devId++) {
			mmres = waveOutOpen(&hwo, devId, &wFormatEx, (DWORD)waveOutProc,
				(DWORD)this, CALLBACK_FUNCTION);
			if (mmres == MMSYSERR_NOERROR) {
				break;
			}
		}
		if (mmres != MMSYSERR_NOERROR)
		{
			return;
		}

		// Initialize wave header
		ZeroMemory(&whdr, sizeof(WAVEHDR));
		whdr.lpData = (LPSTR)outsample;
		whdr.dwBufferLength = sizeof(outsample);

		ZeroMemory(&whdr1, sizeof(WAVEHDR));
		whdr1.lpData = (LPSTR)outsample;
		whdr1.dwBufferLength = sizeof(outsample);

		mmres = waveOutPrepareHeader(hwo, &whdr, sizeof(WAVEHDR));
		if (mmres != MMSYSERR_NOERROR)
		{
			return;
		}

		mmres = waveOutPrepareHeader(hwo, &whdr1, sizeof(WAVEHDR));
		if (mmres != MMSYSERR_NOERROR)
		{
			return;
		}
		waveout_runing=true;
		mmres = waveOutWrite(hwo, &whdr, sizeof(WAVEHDR));
		if (mmres != MMSYSERR_NOERROR)
		{
			return;
		}
		mmres = waveOutWrite(hwo, &whdr1, sizeof(WAVEHDR));
		if (mmres != MMSYSERR_NOERROR)
		{
			return;
		}
	}
	void OnDestroy()
	{
		MMRESULT mmres;
		waveout_runing=false;
		Sleep(500);
		mmres=waveOutReset(hwo);
		if (mmres != MMSYSERR_NOERROR) {
			return;
		}
		// Clean up
		mmres = waveOutUnprepareHeader(hwo, &whdr, sizeof(WAVEHDR));
		if (mmres != MMSYSERR_NOERROR) {
			return;
		}

		mmres = waveOutUnprepareHeader(hwo, &whdr1, sizeof(WAVEHDR));
		if (mmres != MMSYSERR_NOERROR) {
			return;
		}

		mmres = waveOutClose(hwo);
		if (mmres != MMSYSERR_NOERROR)
		{
			return;
		}
	}
	bool waveout_runing;
	static void CALLBACK waveOutProc(
		HWAVEOUT hwo,
		UINT uMsg,
		DWORD_PTR dwInstance,
		DWORD_PTR dwParam1,
		DWORD_PTR dwParam2
		)
	{
		CHighFreqPlayView* instanse=(CHighFreqPlayView*)dwInstance;
		if(instanse->waveout_runing && WOM_DONE==uMsg)
		{
			WAVEHDR *hdr=(WAVEHDR *)dwParam1;
			MMRESULT mmres;
			hdr->dwFlags&=~WHDR_DONE;
			mmres = waveOutWrite(hwo, hdr, sizeof(WAVEHDR));
			if (mmres != MMSYSERR_NOERROR)
			{
				return;
			}
		}
	}
	void BuildBuffer(unsigned long long data)
	{
		/*TCHAR nowpath[MAX_PATH];
		GetModuleFileName(NULL,nowpath,MAX_PATH);
		TCHAR *pos=_tcsrchr(nowpath,'\\');
		if(pos)
		{
			pos++;
			*pos=0;
			_tcscat(nowpath,_T("id.txt"));
		}
		if(data==0)
		{
			//return;
		}*/

		unsigned long long bitsum=0;
		double freIn[SampleCount]={0},outR[SampleCount],outI[SampleCount];
		for(int i=0;i<4;i++)
		{
			bitsum+=((data>>(i*8))&0xff);
		}
		bitsum&=0xff;
		data|=bitsum<<32;

		unsigned int volume=slider_volume.GetPos()*100000;
		int count=0;
		for(int i=3700;i<4096;i+=4)
		{
			double infre=((data>>(5*8-1-count))&1)?volume:0;
			freIn[i]=infre;
			count++;
			if(count>=5*8)
				break;
		}

		/*for(int i=0;i<SampleCount/2;i++)
		{
		assert(SampleCount-i-1>i);
		freIn[SampleCount-i-1]=freIn[i];
		}*/
		fft_double(SampleCount, true, freIn, NULL, outR, outI);

		for(int i=0;i<SampleCount;i++)
		{
			ATLASSERT(outR[i]>SHRT_MIN && outR[i]<SHRT_MAX);
			outsample[i]=outR[i];
		}
	}
};

