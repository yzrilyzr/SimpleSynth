#define SDL_MAIN_HANDLED
#include "Mixer2.h"
#include "SDL.h"
#include "SynthUtil.h"
#include "dsp/EnvelopDetector.h"
#include "dsp/DSPGroupBuilder.h"
#include "dsp/HRIR.h"
#include "dsp/Simple3D.h"
#include "instrument/DLSFormatInstrument.h"
#include "instrument/ReplaceableInstrument.h"
#include "instrument/SF2FormatInstrument.h"
#include "instrument/SimpleDrumSet.h"
#include "instrument/SimpleMIDIInstrument.h"
#include "instrument/TR808DrumSet.h"
#include "lang/Runtime.h"
#include "array/Array.hpp"
#include "lang/String.h"
#include "lang/System.h"
#include "lang/Thread.h"
#include "util/Util.h"
#include "windows.h"
#include "yzrutil.h"
#include "util/Convert.h"
#include <iostream>
#include <string>

#if defined(_WIN32)&&defined (_DEBUG)
#include "vld.h"
#include "crtdbg.h"
#endif

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include "SDL_opengles2.h"
#else
#endif
#include "MixerSequence.h"
#include "io/FileInputStream.h"
#include "util/WAVWriter.h"

#pragma comment(lib,"winmm.lib")
#if defined(_WIN32) && defined (_DEBUG)
#endif
#include <shared_mutex>

using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_util;
using namespace yzrilyzr_io;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_array;

int floatBufferLen=256;
u_sample_rate exportSampleRate=48000;
std::shared_ptr<Mixer2> mixer2=nullptr;
bool exportMode=false;
bool exportTrackMode=false;
bool exportLimiter=true;
bool exportChannelDSP=true;
HMIDIIN hMidiIn;
HMIDIOUT hMidiOut;
MIDIHDR midiHdr;
#define SYSEX_BUFFER_SIZE 1024
BYTE sysexBuffer[SYSEX_BUFFER_SIZE]={0};

std::shared_mutex mixerDspLock;

std::optional<std::future<void>> g_mixFuture;

void closeAndExit(){
	midiInStop(hMidiIn);
	midiInClose(hMidiIn);
	midiOutClose(hMidiOut);
	SDL_CloseAudio();
	SynthUtil::deleteStatic();
	SDL_Quit();
}

void fill_audio_pcm2(void * userdata, Uint8 * stream, int len){
	mixer2->awaitMix();
	u_time t=(u_time)System::nanoTime();
	//mixer2->mix();
	for(uint32_t sample=0, j=0, chc=mixer2->getOutputChannelCount(), buf=mixer2->getBufferSize(); sample < buf; sample++){
		for(u_index ch=0; ch < chc; ch++){
			double f1=mixer2->getOutput(ch)[sample];
			f1=Util::clamp(f1, -1.0, 1.0);
			int32_t c1=(int32_t)(f1 * 0x7fffffff);
			stream[j++]=(Uint8)(c1 & 0xff);
			stream[j++]=(Uint8)((c1 >> 8) & 0xff);
			stream[j++]=(Uint8)((c1 >> 16) & 0xff);
			stream[j++]=(Uint8)((c1 >> 24) & 0xff);
		}
	}
	u_time_f processTime=(u_time_f)((u_time_f)(System::nanoTime() - t) / 1000000000.0);
	if(processTime > mixer2->getProcessStandardTime()){
		std::cout << "Too Heavy" << std::endl;
	}
	mixer2->asyncMix();
}
void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2){
	switch(wMsg){
		case MIM_DATA:
		{
			DWORD_PTR midiMessage=dwParam1;
			BYTE status=midiMessage & 0xFF;
			BYTE data1=(midiMessage >> 8) & 0xFF;
			BYTE data2=(midiMessage >> 16) & 0xFF;
			SynthUtil::sendMIDIBytes(mixer2.get(), status, data1, data2, "WM_MIDI_Instant");
		}
		break;
		case MIM_LONGDATA:
		{
			LPMIDIHDR phdr=(LPMIDIHDR)dwParam1;
			//String sysex(phdr->lpData, 0, phdr->dwBytesRecorded);
			//std::cout << sysex.c_str() << std::endl;
			//SynthUtil::sendSysexMessage(sysex);
		}
		break;
		case MIM_OPEN:
			std::cout << "MIDI Open" << std::endl;
			break;
		case MIM_CLOSE:
			std::cout << "MIDI Close" << std::endl;
			break;
		case MIM_ERROR:
			std::cout << "MIDI Error" << std::endl;
			break;
		default:
			break;
	}
}
void simpleSynthOut(const String & deviceName, uint64_t ev){
	if(hMidiOut == NULL){
		return;
	}
	MMRESULT result=midiOutShortMsg(hMidiOut, ev);
	if(result != MMSYSERR_NOERROR){
		std::cerr << "Failed to send MIDI message. Error: " << result << std::endl;
	}
}
int openMIDIDevice(){
	bool deviceInFound=false;
	bool deviceOutFound=false;
	UINT numDevices=midiInGetNumDevs();
	// 枚举所有 MIDI 输入设备
	for(UINT i=0; i < numDevices; ++i){
		MIDIINCAPS mic;
		if(midiInGetDevCaps(i, &mic, sizeof(MIDIINCAPS)) == MMSYSERR_NOERROR){
			String deviceName(mic.szPname);
			if(deviceName.contains("SimpleSynthIn")){
				MMRESULT result=midiInOpen(&hMidiIn, i, reinterpret_cast<DWORD_PTR>(MidiInProc), 0, CALLBACK_FUNCTION);
				if(result != MMSYSERR_NOERROR){
					std::cerr << "Failed to open MIDI input device." << std::endl;
					continue;
				}
				deviceInFound=true;
				ZeroMemory(&midiHdr, sizeof(MIDIHDR));
				midiHdr.lpData=(LPSTR)sysexBuffer;
				midiHdr.dwBufferLength=SYSEX_BUFFER_SIZE;
				midiInPrepareHeader(hMidiIn, &midiHdr, sizeof(MIDIHDR));
				midiInAddBuffer(hMidiIn, &midiHdr, sizeof(MIDIHDR));
				midiInStart(hMidiIn);
			}
		}
	}
	numDevices=midiOutGetNumDevs();
	// 枚举所有 MIDI 输入设备
	for(UINT i=0; i < numDevices; ++i){
		MIDIOUTCAPS moc;
		if(midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS)) == MMSYSERR_NOERROR){
			String deviceName(moc.szPname);
			if(deviceName.contains("SimpleSynthOut")){
				MMRESULT result=midiOutOpen(&hMidiOut, i, 0, 0, CALLBACK_NULL);
				if(result != MMSYSERR_NOERROR){
					std::cerr << "Failed to open MIDI output device." << std::endl;
					continue;
				}
				deviceOutFound=true;
				SynthUtil::setMIDICallback(simpleSynthOut);
			}
		}
	}
	if(!deviceInFound){
		std::cerr << "MIDI device 'SimpleSynthIn' not found." << std::endl;
		return 1;
	}
	if(!deviceOutFound){
		std::cerr << "MIDI device 'SimpleSynthOut' not found." << std::endl;
		return 1;
	}
	return 0;
}
std::shared_ptr<DSP> AWeightedFilter(u_sample_rate sr){
	return DSPGroupBuilder()
		.begin(DSPGroupBuilder::TYPE_CHAIN)
		.biquad(sr, HIGHPASS, 20.598996, 0.707, 0)
		.biquad(sr, HIGHPASS, 107.65265, 0.707, 0)
		.biquad(sr, HIGHSHELF, 737.86223, 0.5, 5)
		.biquad(sr, HIGHSHELF, 1000, 0.5, 5)
		.biquad(sr, HIGHSHELF, 5000.86223, 0.5, 5)
		.build();
}
void calibrate(int channel, int  program){
	int bufSize=256;
	int sampleRate=48000;
	Mixer2 mixer1(bufSize);
	mixer1.setSampleRate(sampleRate);
	mixer1.setSynthMode(IMixer::MODE_THREAD_POOL, -1);
	mixer1.getGlobalConfig().set(mixer2->getGlobalConfig());
	mixer1.setUseLimiter(false);
	mixer1.setChannelUseDSP(false);
	mixer1.postEvent(new ProgramChange(channel, program), 0);
	mixer1.postEvent(new ChannelControl(channel, MIDIFile::CC::VOLUME, 127), 0);
	mixer1.postEvent(new ChannelControl(channel, MIDIFile::CC::EXPRESSION, 127), 0);
	int index=-1;
	EnvelopDetector rmsED(0, 10000, 10000, EnvelopDetector::RMS, 20);
	EnvelopDetector awRmsED(0, 10000, 10000, EnvelopDetector::RMS, 100);
	EnvelopDetector peakED(0, 10000, 10000, EnvelopDetector::PEAK, 0);
	EnvelopDetector awPeakED(0, 10000, 10000, EnvelopDetector::PEAK, 0);
	awRmsED.init(sampleRate);
	rmsED.init(sampleRate);
	peakED.init(sampleRate);
	awPeakED.init(sampleRate);
	std::shared_ptr<DSP> awFilter=AWeightedFilter(sampleRate);
	awFilter->init(sampleRate);
	SampleArray rms(128);
	SampleArray awRms(128);
	SampleArray peak(128);
	SampleArray awPeak(128);
	SampleArray buffer(bufSize);
	while(index < 128){
		mixer1.mix();
		u_sample * output=mixer1.getOutput(0);
		rmsED.procBlock(output, bufSize);
		peakED.procBlock(output, bufSize);
		awFilter->procBlock(output, buffer._array, bufSize);
		awRmsED.procBlock(buffer._array, bufSize);
		awPeakED.procBlock(buffer._array, bufSize);
		if(!mixer1.hasData()){
			if(index >= 0){
				awRms[index]=awRmsED.getEnvValue();
				awPeak[index]=awPeakED.getEnvValue();
				rms[index]=rmsED.getEnvValue();
				peak[index]=peakED.getEnvValue();
			}
			index++;
			u_time time=mixer1.getCurrentTime();
			mixer1.postEvent(new NoteOn(channel, index, 1), time);
			mixer1.postEvent(new NoteOff(channel, index, 1), time + 0.5);
			rmsED.resetMemory();
			peakED.resetMemory();
			awPeakED.resetMemory();
			awRmsED.resetMemory();
			awFilter->resetMemory();
			std::cout << index << std::endl;
		}
	}
	std::cout << "RMS: " << rms.toString() << std::endl << std::endl;
	std::cout << "A-Weighted RMS: " << awRms.toString() << std::endl << std::endl;
	std::cout << "PEAK: " << peak.toString() << std::endl << std::endl;
	std::cout << "A-Weighted PEAK: " << awPeak.toString() << std::endl << std::endl;
	SampleArray invRms(128);
	SampleArray invAwRms(128);
	SampleArray invPeak(128);
	SampleArray invAwPeak(128);
	for(u_index i=0;i < 128;i++){
		if(rms[i] > 0)invRms[i]=0.707 / rms[i];
		if(awRms[i] > 0)invAwRms[i]=0.707 / awRms[i];
		if(peak[i] > 0)invPeak[i]=1.0 / peak[i];
		if(awPeak[i] > 0)invAwPeak[i]=1.0 / awPeak[i];
	}
	std::cout << "RMS Cali: " << invRms.toString() << std::endl << std::endl;
	std::cout << "A-Weighted RMS Cali: " << invAwRms.toString() << std::endl << std::endl;
	std::cout << "PEAK Cali: " << invPeak.toString() << std::endl << std::endl;
	std::cout << "A-Weighted PEAK Cali: " << invAwPeak.toString() << std::endl << std::endl;
}
void exportWAV(const String & fileName, std::shared_ptr<MixerSequence> seq){
	if(exportTrackMode){
		u_index bufLength=65536;
		Mixer2 mixer1(bufLength);
		mixer1.setSampleRate(exportSampleRate);
		WAVWriter * raf[16];
		for(u_index i=0;i < 16;i++){
			raf[i]=new WAVWriter(fileName + "-track-" + std::to_string(i) + ".wav");
			raf[i]->prepare(mixer1.getSampleRate(), 64, WAVWriter::FORMAT_FLOAT, 2);
		}
		mixer1.setSynthMode(IMixer::MODE_THREAD_POOL, -1);
		mixer1.getGlobalConfig().set(mixer2->getGlobalConfig());
		mixer1.setUseLimiter(false);
		mixer1.setChannelUseDSP(exportChannelDSP);
		seq->postToMixer(&mixer1, 0);
		u_index chc=mixer1.getOutputChannelCount();
		while(mixer1.hasData()){
			mixer1.mix();
			std::cout << mixer1.getPostedEventCount() << std::endl;
			for(u_index channelID=0;channelID < 16;channelID++){
				std::shared_ptr<IChannel> c=mixer1.IMixer::getMIDIChannel(channelID);
				if(c == nullptr)continue;
				WAVWriter & rafi=*raf[channelID];
				for(uint32_t sample=0, j=0; sample < bufLength; sample++){
					for(u_index ch=0; ch < chc; ch++){
						double f1=(double)c->getOutput(ch)[sample];
						rafi.writeDouble(f1);
					}
					rafi.nextFrame();
				}
			}
		}
		for(u_index i=0;i < 16;i++){
			raf[i]->end();
		}
	} else if(exportMode){
		WAVWriter raf(fileName + ".wav");
		u_index bufLength=8192;
		Mixer2 mixer1(bufLength);
		mixer1.setSampleRate(exportSampleRate);
		raf.prepare(mixer1.getSampleRate(), 64, WAVWriter::FORMAT_FLOAT, 2);
		mixer1.setSynthMode(IMixer::MODE_THREAD_POOL, -1);
		mixer1.getGlobalConfig().set(mixer2->getGlobalConfig());
		mixer1.setUseLimiter(exportLimiter);
		mixer1.setChannelUseDSP(exportChannelDSP);
		seq->postToMixer(&mixer1, 0);
		std::cout << mixer1.getPostedEventCount() << std::endl;
		while(mixer1.hasData()){
			mixer1.mix();
			std::cout << mixer1.getPostedEventCount() << std::endl;
			for(uint32_t sample=0, j=0, chc=mixer1.getOutputChannelCount(), buf=mixer1.getBufferSize(); sample < buf; sample++){
				for(u_index ch=0; ch < chc; ch++){
					double f1=(double)mixer1.getOutput(ch)[sample];
					raf.writeDouble(f1);
				}
				raf.nextFrame();
			}
		}
		raf.end();
	}
	std::cout << "Write" << std::endl;
}

int main(int argc, char * argv[]){
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif // DEBUG
	if(SDL_Init(SDL_INIT_AUDIO) != 0){
		fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}
	mixer2=std::make_shared < Mixer2>(floatBufferLen);
	mixer2->setSynthMode(IMixer::MODE_THREAD_POOL, -1);
	mixer2->setSampleRate(48000);
	std::shared_ptr<SimpleMIDIInstrument> simple=std::make_shared<SimpleMIDIInstrument>();
	std::shared_ptr <ReplaceableInstrument> rin=std::make_shared<ReplaceableInstrument>(simple);
	mixer2->getGlobalConfig().setInstrumentProvider(rin);
	SDL_AudioSpec spec;
	spec.freq=48000;
	spec.format=AUDIO_S32SYS;
	spec.channels=2;
	spec.silence=0;
	spec.samples=floatBufferLen;
	spec.callback=fill_audio_pcm2;
	spec.userdata=nullptr;
	if(SDL_OpenAudio(&spec, nullptr)){
		fprintf(stderr, "Failed to open audio device, %s\n", SDL_GetError());
		return -1;
	}
	SDL_PauseAudio(0);
	openMIDIDevice();
	mixer2->setUseLimiter(true);
	std::string a;
	while(true){
		std::getline(std::cin, a);
		String str=a;
		try{
			if(str.length() > 0 && str.startsWith("\"") && str.endsWith("\"")){
				str=str.substring(1, str.length() - 1);
				a=str.tostring();
			}
			if(str.endsWith(".xm")){
				std::shared_ptr<MixerSequence> seq=SynthUtil::parseXM(FileInputStream(str));
				if(seq == nullptr)throw Exception("File read error");
				if(exportMode || exportTrackMode)exportWAV(File(str).getName(), seq);
				else seq->postToMixer(mixer2.get(), 1, "Console_XM");
			} else if(str.endsWith(".mid")){
				std::shared_ptr<MixerSequence> seq=SynthUtil::parseMIDI(FileInputStream(str));
				if(seq == nullptr)throw Exception("File read error");
				static int inc=0;
				if(exportMode || exportTrackMode)exportWAV(File(str).getName(), seq);
				else seq->postToMixer(mixer2.get(), 1, "Console_MIDI" + std::to_string(inc++));
			} else if(str.endsWith(".hrir")){
				mixer2->getGlobalConfig().set3DEffect(HRIR::parseHRIR(FileInputStream(str)));
			} else if(str == "3d"){
				mixer2->getGlobalConfig().set3DEffect(std::make_shared<Simple3D>());
			} else if(str.endsWith(".sf2")){
				mixer2->getGlobalConfig().setInstrumentProvider(std::make_shared<SF2FormatInstrument>(FileInputStream(str)));
			} else if(str.endsWith(".dls")){
				mixer2->getGlobalConfig().setInstrumentProvider(std::make_shared<DLSFormatInstrument>(FileInputStream(str)));
			} else if(str == ""){
				mixer2->reset();
			} else if(str.startsWith("cali")){
				auto split=str.split(" ");//channel program
				uint8_t channel=parseInt(split[1].tostring());
				uint8_t program=parseInt(split[2].tostring());
				calibrate(channel, program);
			} else if(str == "def"){
				rin->setDrumSet(std::make_shared<SimpleDrumSet>());
				mixer2->getGlobalConfig().setInstrumentProvider(rin);
			} else if(str == "808"){
				rin->setDrumSet(std::make_shared<TR808DrumSet>());
			} else if(str == "export"){
				exportMode=!exportMode;
				std::cout << "ExportMode: " << exportMode << std::endl;
			} else if(str == "exporttrack"){
				exportTrackMode=!exportTrackMode;
				std::cout << "ExportTrack: " << exportTrackMode << std::endl;
			} else if(str == "exportsamplerate"){
				std::cout << "Input new sample rate" << std::endl;
				std::getline(std::cin, a);
				exportSampleRate=parseInt(a);
				std::cout << "ExportSampleRate: " << (int)exportSampleRate << std::endl;
			} else if(str == "limiter"){
				exportLimiter=!exportLimiter;
				std::cout << "ExportLimiter:" << exportLimiter << std::endl;
			} else if(str == "channeldsp"){
				exportChannelDSP=!exportChannelDSP;
				std::cout << "ExportChannelDSP:" << exportChannelDSP << std::endl;
			} else if(str == "exit"){
				break;
			} else if(str == "exportinfo"){
				std::cout << "ExportMode: " << exportMode << std::endl;
				std::cout << "ExportTrack: " << exportTrackMode << std::endl;
				std::cout << "ExportLimiter:" << exportLimiter << std::endl;
				std::cout << "ExportSampleRate: " << (int)exportSampleRate << std::endl;
			} else if(str.startsWith("midi ")){
				auto & split=str.split(" ");
				uint8_t ty=parseInt(split[1].tostring(), 16);
				uint8_t data1=parseInt(split[2].tostring(), 16);
				uint8_t data2=parseInt(split[3].tostring(), 16);
				SynthUtil::sendMIDIBytes(mixer2.get(), ty, data1, data2);
			}
		} catch(Exception e){
			std::cout << "Error: " << e.what() << std::endl;
		}
	}
	closeAndExit();
	return 0;
}