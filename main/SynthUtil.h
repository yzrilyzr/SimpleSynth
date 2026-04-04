#pragma once
#include "MixerSequence.h"
#include "SimpleSynth.h"
#include "array/Array.hpp"
#include "events/ChannelEvent.h"
#include "interface/IMixer.h"
#include "interface/NoteProcessor.h"
#include "io/InputStream.h"
#include "util/MIDIFile.h"

namespace yzrilyzr_simplesynth{
	class NoteProcessor;
	class Mixer2;
	ECLASS(TickChange, public ChannelEvent){
		public:
		long startAtTick;
		float tick;
		TickChange(long startAtTick, float tick) : startAtTick(startAtTick), tick(tick){}
		uint8_t getType() override{ throw - 1; }
		u_up<ChannelEvent>  clone() override{ throw - 1; }
		yzrilyzr_lang::String toString() const override;
	};
	EBCLASS(SynthUtil){
		public:
		typedef void(*MIDICallback)(const yzrilyzr_lang::String & deviceName, uint64_t ev);
		static u_sp<MixerSequence> parseMIDI(yzrilyzr_io::InputStream & is);
		//static u_sp<IChannel> getMIDIChannelOrNew(IMixer * mixer, const yzrilyzr_lang::String & groupName, s_midichannel_id channelID);
		//static u_sp<IChannel> getMIDIChannelOrNew(IMixer * mixer, s_midichannel_id channelID);
		static u_sp<MixerSequence> parseXM(yzrilyzr_io::InputStream & is);
		static void sequenceToMIDI(u_sp<MixerSequence> seq,yzrilyzr_io::OutputStream & os);
		static NoteProcPtr getDefault();
		static void sendMIDIBytes(IMixer * mixer, uint8_t ty, uint8_t data1, uint8_t data2);
		static void sendMIDIBytes(IMixer * mixer, uint8_t ty, uint8_t data1, uint8_t data2, const yzrilyzr_lang::String & groupName);
		static void sendMIDIEvent(u_up<ChannelEvent>  event, const yzrilyzr_lang::String & deviceName);
		static u_up<ChannelEvent>  MIDIBytes2Event(uint8_t ty, uint8_t data1, uint8_t data2);
		static uint64_t Event2MIDIBytes(u_up<ChannelEvent>  event);
		static uint64_t MergeMIDIBytes(uint8_t ty, uint8_t data1, uint8_t data2);
		static yzrilyzr_array::SampleArray noise(u_index length, u_sample_rate sampleRate, u_freq f1, u_freq f2);
		static yzrilyzr_array::SampleArray NOISE;
		static void deleteStatic();
		static bool isInstrumentProviderSustainable(s_program_id program);
		static void setMIDICallback(MIDICallback callback);
		private:
		static MIDICallback callback;
	};
	EBCLASS(FixedRandom){
		private:
		yzrilyzr_array::SampleArray data;
		u_index index;
		public:
		FixedRandom(const yzrilyzr_array::SampleArray & data);
		FixedRandom();
		u_sample next();
	};
}