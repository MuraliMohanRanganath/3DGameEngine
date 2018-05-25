#include "Audio.h"
#include<XAudio2.h>
#include<x3daudio.h>
#include <xaudio2fx.h>
#include "../Asserts/Asserts.h"
#include "../Math/cVector.h"
#include "../Graphics/Transform.h"
#include <vector>
#include<memory>
#include<thread>

#ifdef _XBOX //Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX //Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

namespace
{
	struct EmitterAudio
	{
		X3DAUDIO_EMITTER emitter;
		std::string  soundPath;
		bool* o_soundPlaying;
	};

	class VoiceCallback : public IXAudio2VoiceCallback
	{
	public:
		HANDLE hBufferEndEvent;

		VoiceCallback() : hBufferEndEvent(CreateEvent(NULL, FALSE, FALSE, NULL)) {}
		~VoiceCallback() { CloseHandle(hBufferEndEvent); }

		//Called when the voice has just finished playing a contiguous audio stream.
		void STDMETHODCALLTYPE OnStreamEnd() {
			SetEvent(hBufferEndEvent);
		}

		//Unused methods are stubs
		void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() {

		}
		void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 SamplesRequired) {

		}

		void STDMETHODCALLTYPE OnBufferEnd(void * pBufferContext) {
			SetEvent(hBufferEndEvent);

		}
		void STDMETHODCALLTYPE OnBufferStart(void * pBufferContext) {    }
		void STDMETHODCALLTYPE OnLoopEnd(void * pBufferContext) {    }
		void STDMETHODCALLTYPE OnVoiceError(void * pBufferContext, HRESULT Error) { }
	};

	HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD & dwChunkDataPosition);
	HRESULT ReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset);
	void PlayMySound(const XAUDIO2_BUFFER& i_buffer, const WAVEFORMATEXTENSIBLE& wfx);
	void PlayDSound(const EmitterAudio* i_emmiter);
	IXAudio2* pXAudio2 = NULL;
	IXAudio2MasteringVoice* pMasterVoice = NULL;
	IXAudio2SourceVoice* pMusicSourceVoice = NULL;
	IXAudio2SubmixVoice* pSubmixVoice = NULL;
	IUnknown* pReverbEffect;
	static const X3DAUDIO_CONE Listener_DirectionalCone = { X3DAUDIO_PI*5.0f / 6.0f, X3DAUDIO_PI*11.0f / 6.0f, 1.0f, 0.75f, 0.0f, 0.25f, 0.708f, 1.0f };

	float effect_volume = 0;
	float music_volume = 0;

	//3d Audio instance
	X3DAUDIO_HANDLE X3DInstance;

	//player listener
	X3DAUDIO_LISTENER s_listener;

	//static emmiters
	std::vector<EmitterAudio> s_static_emitters;
	//moving emmiters
}

void eae6320::Audio::Update() {
	for (size_t i = 0; i < s_static_emitters.size(); ++i) 
	{
		if (!*s_static_emitters[i].o_soundPlaying) 
		{
			std::thread t(PlayDSound, &s_static_emitters[i]);
			t.detach();
		}
	}
	s_static_emitters.clear();
}

bool eae6320::Audio::Initialize() 
{

	HRESULT hr;
	//create xaudio2 device
	if (FAILED(hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
		return false;
	//create xaudio2 mastering volume
	if (FAILED(hr = pXAudio2->CreateMasteringVoice(&pMasterVoice)))
		return false;

	XAUDIO2_VOICE_DETAILS details;
	pMasterVoice->GetVoiceDetails(&details);

	UINT32 rflags = 0;
#if (_WIN32_WINNT < 0x0602 /*_WIN32_WINNT_WIN8*/) && defined(_DEBUG)
	rflags |= XAUDIO2FX_DEBUG;
#endif

	if (FAILED(hr = XAudio2CreateReverb(&pReverbEffect, rflags)))
		return false;
	
	XAUDIO2_EFFECT_DESCRIPTOR effects[] = { { pReverbEffect, TRUE, 1 } };
	XAUDIO2_EFFECT_CHAIN effectChain = { 1, effects };

	if (FAILED(hr = pXAudio2->CreateSubmixVoice(&pSubmixVoice, 1, details.InputSampleRate, 0,0,nullptr, &effectChain)))
		return false;


	XAUDIO2FX_REVERB_PARAMETERS native;
	XAUDIO2FX_REVERB_I3DL2_PARAMETERS largeRoom = XAUDIO2FX_I3DL2_PRESET_LARGEROOM;
	ReverbConvertI3DL2ToNative(&largeRoom, &native);
	pSubmixVoice->SetEffectParameters(0, &native, sizeof(native));


	//create x3daudio
	DWORD dwChannelmask;
	pMasterVoice->GetChannelMask(&dwChannelmask);

	const float SPEEDOFSOUND = X3DAUDIO_SPEED_OF_SOUND;
	X3DAudioInitialize(dwChannelmask, SPEEDOFSOUND, X3DInstance);
	return true;
}

void eae6320::Audio::ModifyMusicVolume(const float i_value)
{
	if (pMusicSourceVoice) {
		float volume = 0;
		pMusicSourceVoice->GetVolume(&volume);

		if (volume >= 1.0f && i_value > 0)
			return;
		else if (volume <= 0 && i_value < 0)
		{
			pMusicSourceVoice->SetVolume(0);
			return;
		}

		volume += i_value;
		pMusicSourceVoice->SetVolume(volume);
	}
	else {
		if (music_volume >= 1.0f && i_value > 0)
			return;
		else if (music_volume <= 0 && i_value < 0)
			return;
		music_volume += i_value;


	}
}

void eae6320::Audio::ModifyEffectVolume(const float i_value)
{
	if (effect_volume >= 1.0f && i_value > 0)
		return;
	else if (effect_volume <= 0 && i_value < 0) {
		effect_volume = 0;
		return;
	}
	effect_volume += i_value;
}

void eae6320::Audio::UpdateListener(const Graphics::Transform & i_transform)
{
	s_listener.OrientFront = X3DAUDIO_VECTOR(i_transform.getForward().x, i_transform.getForward().y, i_transform.getForward().z);
	s_listener.OrientTop = X3DAUDIO_VECTOR(i_transform.getUp().x, i_transform.getUp().y, i_transform.getUp().z);
	s_listener.Position = X3DAUDIO_VECTOR(i_transform.getPosition().x, i_transform.getPosition().y, i_transform.getPosition().z);
	s_listener.pCone = (X3DAUDIO_CONE*)&Listener_DirectionalCone;

}
void eae6320::Audio::AddStaticEmitter(const eae6320::Graphics::Transform & i_transform, const char* const i_path, bool* o_soundPlaying)
{
	EmitterAudio emitteraudio;
	emitteraudio.emitter = { 0 };
	emitteraudio.emitter.ChannelCount = 1;
	emitteraudio.emitter.CurveDistanceScaler = FLT_MIN;
	emitteraudio.emitter.OrientFront = X3DAUDIO_VECTOR(i_transform.getForward().x, i_transform.getForward().y, i_transform.getForward().z);
	emitteraudio.emitter.OrientTop = X3DAUDIO_VECTOR(i_transform.getUp().x, i_transform.getUp().y, i_transform.getUp().z);
	emitteraudio.emitter.Position = X3DAUDIO_VECTOR(i_transform.getPosition().x, i_transform.getPosition().y, i_transform.getPosition().z);
	emitteraudio.emitter.Velocity = X3DAUDIO_VECTOR(0, 0, 0);
	emitteraudio.soundPath = i_path;
	emitteraudio.o_soundPlaying = o_soundPlaying;
	s_static_emitters.push_back(emitteraudio);

}
bool eae6320::Audio::CleanUp() {
	pXAudio2->Release();
	pXAudio2 = NULL;


	return true;
}

bool eae6320::Audio::PlayMusic(const char* const i_path) {
	EAE6320_ASSERTF(i_path != NULL, "Invalid audio file location");
	//open the audio file with CreateFile
	HANDLE hFile;
	{
		hFile = CreateFile(
			i_path,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if (INVALID_HANDLE_VALUE == hFile)
			//return HRESULT_FROM_WIN32(GetLastError());
			return false;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
			//return HRESULT_FROM_WIN32(GetLastError());
			return false;
	}
	//Locate the 'RIFF' chunk in the udio file, and check the file type
	DWORD dwChunkSize;
	DWORD dwChunkPosition;
	//check the file type should be fourccWAVE or XWMA
	{
		FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
		DWORD filetype;
		ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
		if (filetype != fourccWAVE)
			return S_FALSE;
	}
	//locate the 'fmt chunk and copy its contents into a WAVEFORMATEXTENSIBLE structure
	WAVEFORMATEXTENSIBLE wfx = { 0 };
	{
		FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
		ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);
	}
	//locate the 'data' chunk and read its contents into a buffer
	//fill outt he audio data buffer with the contents of the fourccDATA chunk
	FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
	BYTE* pDataBuffer = new BYTE[dwChunkSize];
	ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);
	//populate the XAUDIO2_BUFFER structure
	XAUDIO2_BUFFER musicBuffer;
	musicBuffer = { 0 };
	musicBuffer.AudioBytes = dwChunkSize;	//buffer containing audio data
	musicBuffer.pAudioData = pDataBuffer;	//size of the audio buffer in bytes
	musicBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	musicBuffer.Flags = XAUDIO2_END_OF_STREAM;	//tell the source voice not to expect any data after this buffer


	HRESULT hr;
	if (FAILED(hr = pXAudio2->CreateSourceVoice(&pMusicSourceVoice, &(wfx.Format))))
		//		return hr;
		return false;
	if (FAILED(hr = pMusicSourceVoice->SubmitSourceBuffer(&musicBuffer)))
		return false;
	pMusicSourceVoice->SetVolume(music_volume);
	if (FAILED(hr = pMusicSourceVoice->Start(0)))
		return false;



	return true;
}

bool eae6320::Audio::PlayEffect(const char* const i_path) {

	IXAudio2SourceVoice* pEffectSourceVoice = NULL;

	EAE6320_ASSERTF(i_path != NULL, "Invalid audio file location");
	//open the audio file with CreateFile
	HANDLE hFile;
	{
		hFile = CreateFile(
			i_path,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if (INVALID_HANDLE_VALUE == hFile)
			//return HRESULT_FROM_WIN32(GetLastError());
			return false;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
			//return HRESULT_FROM_WIN32(GetLastError());
			return false;
	}
	//Locate the 'RIFF' chunk in the udio file, and check the file type
	DWORD dwChunkSize;
	DWORD dwChunkPosition;
	//check the file type should be fourccWAVE or XWMA
	{
		FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
		DWORD filetype;
		ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
		if (filetype != fourccWAVE)
			return S_FALSE;
	}
	//locate the 'fmt chunk and copy its contents into a WAVEFORMATEXTENSIBLE structure
	WAVEFORMATEXTENSIBLE wfx = { 0 };
	{
		FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
		ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);
	}
	//locate the 'data' chunk and read its contents into a buffer
	//fill outt he audio data buffer with the contents of the fourccDATA chunk
	FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
	BYTE* pDataBuffer = new BYTE[dwChunkSize];
	ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);


	XAUDIO2_BUFFER effectBuffer = { 0 };
	effectBuffer.AudioBytes = dwChunkSize;	//buffer containing audio data
	effectBuffer.pAudioData = pDataBuffer;	//size of the audio buffer in bytes
	effectBuffer.Flags = XAUDIO2_END_OF_STREAM;	//tell the source voice not to expect any data after this buffer

	std::thread t(PlayMySound, effectBuffer, wfx);
	t.detach();

	return true;
}

float eae6320::Audio::GetMusicVolume()
{
	if (pMusicSourceVoice) {
		float volume = 0;
		pMusicSourceVoice->GetVolume(&volume);
		return volume;
	}
	return 0;
}

float eae6320::Audio::GetEffectVolume()
{
	/*if (pEffectSourceVoice!=nullptr) {
	float volume = 0;
	pEffectSourceVoice->GetVolume(&volume);
	return volume;
	}*/
	return effect_volume;
}

namespace {
	HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD & dwChunkDataPosition) {
		HRESULT hr = S_OK;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
			return HRESULT_FROM_WIN32(GetLastError());

		DWORD dwChunkType;
		DWORD dwChunkDataSize;
		DWORD dwRIFFDataSize = 0;
		DWORD dwFileType;
		DWORD bytesRead = 0;
		DWORD dwOffset = 0;
		while (hr == S_OK) {
			DWORD dwRead;
			if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());
			if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());
			switch (dwChunkType)
			{
			case fourccRIFF:
				dwRIFFDataSize = dwChunkDataSize;
				dwChunkDataSize = 4;
				if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
					hr = HRESULT_FROM_WIN32(GetLastError());
				break;
			default:
				if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
					return HRESULT_FROM_WIN32(GetLastError());
			}
			dwOffset += sizeof(DWORD) * 2;
			if (dwChunkType == fourcc) {
				dwChunkSize = dwChunkDataSize;
				dwChunkDataPosition = dwOffset;
				return S_OK;
			}
			dwOffset += dwChunkDataSize;
			if (bytesRead >= dwRIFFDataSize)
				return S_FALSE;


		}
		return S_OK;
	}
	HRESULT ReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset) {
		HRESULT hr = S_OK;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
			return HRESULT_FROM_WIN32(GetLastError());
		DWORD dwRead;
		if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}


	void PlayMySound(const XAUDIO2_BUFFER& i_buffer, const WAVEFORMATEXTENSIBLE& wfx) {

		IXAudio2SourceVoice* pEffectSourceVoice = NULL;
		VoiceCallback voiceCallback;
		HRESULT hr;
		if (FAILED(hr = pXAudio2->CreateSourceVoice(&pEffectSourceVoice, &(wfx.Format), 0, XAUDIO2_DEFAULT_FREQ_RATIO, &voiceCallback, NULL, NULL)))
			//		return hr;
			return;
		if (FAILED(hr = pEffectSourceVoice->SubmitSourceBuffer(&i_buffer)))
			return;
		pEffectSourceVoice->SetVolume(effect_volume);
		if (FAILED(hr = pEffectSourceVoice->Start(0)))
			return;
		WaitForSingleObjectEx(voiceCallback.hBufferEndEvent, INFINITE, TRUE);

		pEffectSourceVoice->Stop();
		delete i_buffer.pAudioData;
		pEffectSourceVoice->DestroyVoice();

	}


	void PlayDSound(const EmitterAudio* i_emmiter) {
		bool* soundplay = i_emmiter->o_soundPlaying;
		*soundplay = true;
		IXAudio2SourceVoice* pEffectSourceVoice = NULL;

		//EAE6320_ASSERTF(i_emmiter.soundPath != NULL, "Invalid audio file location");
		//open the audio file with CreateFile
		HANDLE hFile;
		{
			hFile = CreateFile(
				i_emmiter->soundPath.c_str(),
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

			if (INVALID_HANDLE_VALUE == hFile)
				//return HRESULT_FROM_WIN32(GetLastError());
				return;
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
				//return HRESULT_FROM_WIN32(GetLastError());
				return;
		}
		//Locate the 'RIFF' chunk in the udio file, and check the file type
		DWORD dwChunkSize;
		DWORD dwChunkPosition;
		//check the file type should be fourccWAVE or XWMA
		{
			FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
			DWORD filetype;
			ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
			if (filetype != fourccWAVE)
				return;
		}
		//locate the 'fmt chunk and copy its contents into a WAVEFORMATEXTENSIBLE structure
		WAVEFORMATEXTENSIBLE wfx = { 0 };
		{
			FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
			ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);
		}
		//locate the 'data' chunk and read its contents into a buffer
		//fill outt he audio data buffer with the contents of the fourccDATA chunk
		FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
		BYTE* pDataBuffer = new BYTE[dwChunkSize];
		ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);


		XAUDIO2_BUFFER deffectBuffer = { 0 };
		deffectBuffer.AudioBytes = dwChunkSize;	//buffer containing audio data
		deffectBuffer.pAudioData = pDataBuffer;	//size of the audio buffer in bytes
		deffectBuffer.Flags = XAUDIO2_END_OF_STREAM;	//tell the source voice not to expect any data after this buffer

		X3DAUDIO_DSP_SETTINGS DSPSettings;
		DSPSettings = { 0 };
		XAUDIO2_VOICE_DETAILS voiceDetails;
		pMasterVoice->GetVoiceDetails(&voiceDetails);

		FLOAT32* matrix = new FLOAT32[voiceDetails.InputChannels];

		DSPSettings.SrcChannelCount = 1;
		DSPSettings.DstChannelCount = voiceDetails.InputChannels;
		DSPSettings.pMatrixCoefficients = matrix;
		VoiceCallback voiceCallback;
		HRESULT hr;
		if (FAILED(hr = pXAudio2->CreateSourceVoice(&pEffectSourceVoice, &(wfx.Format), 0, XAUDIO2_DEFAULT_FREQ_RATIO, &voiceCallback, NULL, NULL)))
			//		return hr;
			return;
		if (FAILED(hr = pEffectSourceVoice->SubmitSourceBuffer(&deffectBuffer)))
			return;

		X3DAudioCalculate(X3DInstance, &s_listener, &(i_emmiter->emitter), X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_LPF_REVERB | X3DAUDIO_CALCULATE_REVERB, &DSPSettings);

		pEffectSourceVoice->SetOutputMatrix(pMasterVoice, 1, voiceDetails.InputChannels, DSPSettings.pMatrixCoefficients);
		pEffectSourceVoice->SetFrequencyRatio(DSPSettings.DopplerFactor);

		pEffectSourceVoice->SetOutputMatrix(pSubmixVoice, 1, 1, &DSPSettings.ReverbLevel);

		XAUDIO2_FILTER_PARAMETERS FilterParameters = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * DSPSettings.LPFDirectCoefficient), 1.0f };
		pEffectSourceVoice->SetFilterParameters(&FilterParameters);
		pEffectSourceVoice->SetVolume(effect_volume);

		XAUDIO2_FILTER_PARAMETERS FilterParametersReverb = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * DSPSettings.LPFReverbCoefficient), 1.0f };
		pEffectSourceVoice->SetOutputFilterParameters(pSubmixVoice, &FilterParametersReverb);

		if (FAILED(hr = pEffectSourceVoice->Start(0)))
			return;
		WaitForSingleObjectEx(voiceCallback.hBufferEndEvent, INFINITE, TRUE);

		pEffectSourceVoice->Stop();
		delete[] deffectBuffer.pAudioData;
		delete[] DSPSettings.pMatrixCoefficients;
		pEffectSourceVoice->DestroyVoice();
		*soundplay = false;
	}
}