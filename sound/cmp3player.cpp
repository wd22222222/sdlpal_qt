#include <string>
#include "../main/command.h"
#include "../main/cpalevent.h"
#include "../sound/libmad/music_mad.h"
#include "../sound/libmad/resampler.h"
#include "caudiodevice.h"

Mp3player::Mp3player() 
{
#ifdef  USE_EVENTHANDLER
	CPalEventHandler _m;
	Uint32 customEventType = _m.getCustomEventHandler(eventTypeCustom::eventMp3MusicPlay);
	//注册播放音乐函数
	_m.registCallback(customEventType, [this](const SDL_Event* event) {
		//传入 1 指向音乐缓存 2 缓存长度
		auto stream = static_cast<ByteArray*>(event->user.data1);
		//将需要的数据传入缓存，stream 来自 CAudioDevice::PAL_FillAudioBuffer
		//在此过程中改写stream->data()指向的内存
		fillBuffer(*stream, static_cast<int>(
			reinterpret_cast<intptr_t>(event->user.data2)));
		CPalEventHandler _m;
		//解锁,在音乐播放csound 中加的锁，确保传递成功
		_m.setCustomMutex((UINT)eventTypeCustom::eventMp3MusicPlay, FALSE);
		});
#endif //USE_EVENTHANDLER
	iMusic = -1;
    fLoop = FALSE;
}

int Mp3player::Player(int iNum, bool floop, float)
{
	fLoop = floop;

	if (iNum == iMusic)
	{
		return TRUE;
	}

	MP3_Close();

	iMusic = iNum;

	if (iNum > 0)
	{
		std::string fname = lpAudioData->PalDir + CPalEvent::va("mp3\\%2.2d.mp3", iNum);
		
		pMP3 = mad_openFile(fname.c_str(), 
			&lpAudioData->lpAudioDevice->spec, lpAudioData->ResampleQuality);
		if (pMP3)
		{
			mad_start(pMP3);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return TRUE;
	}
}

VOID Mp3player::MP3_Close()
{
	mad_stop(pMP3);
	mad_closeFile(pMP3);
	pMP3 = nullptr;
}


void Mp3player::fillBuffer(ByteArray& stream, int len) {


	if (pMP3) {
		if (!mad_isPlaying(pMP3) && fLoop)
		{
			mad_seek(pMP3, 0);
			mad_start(pMP3);
		}

		if (mad_isPlaying(pMP3))
			mad_getSamples(pMP3, stream.data(), len);
	}

}

Mp3player::~Mp3player()
{
	MP3_Close();
}

