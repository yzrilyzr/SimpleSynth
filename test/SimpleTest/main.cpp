#define SDL_MAIN_HANDLED
#include "Mixer2.h"
#include "SDL.h"
#include "SynthUtil.h" 
#include "instrument/SimpleMIDIInstrument.h"
#include "lang/String.h"
#include "util/Convert.h"
#include "io/FileInputStream.h"

using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_util;
using namespace yzrilyzr_io;

int floatBufferLen=256;
u_sp<Mixer2> mixer2=nullptr;

void fill_audio_pcm2(void * userdata, Uint8 * stream, int len){
	mixer2->awaitMix();
	for(uint32_t sample=0, j=0, chc=mixer2->getOutputChannelCount(), buf=mixer2->getBufferSize(); sample < buf; sample++){
		for(u_index ch=0; ch < chc; ch++){
			double f1=mixer2->getOutput(ch)[sample];
			f1=Util::clamp(f1, -1.0, 1.0);
			int32_t c1=(int32_t)(f1 * Integer_MAX_VALUE);
			stream[j++]=(Uint8)(c1 & 0xff);
			stream[j++]=(Uint8)((c1 >> 8) & 0xff);
			stream[j++]=(Uint8)((c1 >> 16) & 0xff);
			stream[j++]=(Uint8)((c1 >> 24) & 0xff);
		}
	}
	mixer2->asyncMix();
}

void playData(){
	String midiEventsStr=String(Util::read(FileInputStream("miditext.txt")),UTF8).replace("\r", "");
	auto midiEvents=midiEventsStr.split("\n");
	for(String &event:midiEvents){
		u_index evl=event.length();
		int64_t timestamp=parseLong(event.substring(0, evl - 6));
		int32_t type=parseInt(event.substring(evl - 6, evl - 4), 16);
		int32_t data1=parseInt(event.substring(evl - 4, evl - 2), 16);
		int32_t data2=parseInt(event.substring(evl - 2, evl), 16);
		if(timestamp > 0){
			Thread::sleep(timestamp / 1000);
		}
		System::out.println(String("Event: ")+type+data1+data2);
		SynthUtil::sendMIDIBytes(mixer2.get(), type, data1, data2, "WM_MIDI_Instant");		
	}
}

int main(int argc, char * argv[]){
	if(SDL_Init(SDL_INIT_AUDIO) != 0){
		fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}
	mixer2=mksp<Mixer2>(floatBufferLen);
	mixer2->setSynthMode(IMixer::MODE_THREAD_POOL, -1);
	mixer2->setSampleRate(48000);
	mixer2->setUseLimiter(true);
	u_sp<SimpleMIDIInstrument> simple=mksp<SimpleMIDIInstrument>();
	mixer2->getGlobalConfig().setInstrumentProvider(simple);
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
	std::string a;
	playData();
	SDL_CloseAudio();
	SynthUtil::deleteStatic();
	SDL_Quit();
	return 0;
}