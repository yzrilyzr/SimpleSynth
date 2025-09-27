#pragma once
#include "interface/IMixer.h"
#include "interface/NoteProcessor.h"
#include "MixerSequence.h"
#include "SimpleSynth.h"
#include "events/ChannelEvent.h"
#include "io/InputStream.h"
#include "util/MIDIFile.h"

#include "array/SampleArray.h"

namespace yzrilyzr_simplesynth{
	class NoteProcessor;
	class Mixer2;
	ECLASS(TickChange, public ChannelEvent){
		public:
		long startAtTick;
		float tick;
		TickChange(long startAtTick, float tick) : startAtTick(startAtTick), tick(tick){}
		uint8_t getType() override{ throw - 1; }
		ChannelEvent * clone() override{ throw - 1; }
		std::string toString() const override;
	};
	EBCLASS(SynthUtil){
		public:
		typedef void(*MIDICallback)(const std::string & deviceName, uint64_t ev);
		static std::shared_ptr<MixerSequence> parseMIDI(yzrilyzr_io::InputStream & is);
		//static std::shared_ptr<IChannel> getMIDIChannelOrNew(IMixer * mixer, const std::string & groupName, s_midichannel_id channelID);
		//static std::shared_ptr<IChannel> getMIDIChannelOrNew(IMixer * mixer, s_midichannel_id channelID);
		static std::shared_ptr<MixerSequence> parseXM(yzrilyzr_io::InputStream & is);
		static NoteProcPtr getDefault();
		static void sendMIDIBytes(IMixer * mixer, uint8_t ty, uint8_t data1, uint8_t data2);
		static void sendMIDIBytes(IMixer * mixer, uint8_t ty, uint8_t data1, uint8_t data2, const std::string & groupName);
		static void sendMIDIEvent(ChannelEvent * event, const std::string & deviceName);
		static ChannelEvent * MIDIBytes2Event(uint8_t ty, uint8_t data1, uint8_t data2);
		static uint64_t Event2MIDIBytes(ChannelEvent * event);
		static uint64_t MergeMIDIBytes(uint8_t ty, uint8_t data1, uint8_t data2);
		static yzrilyzr_array::SampleArray * noise(int32_t length, int32_t sampleRate, u_freq f1, u_freq f2);
		static yzrilyzr_array::SampleArray * NOISE;
		static void deleteStatic();
		static bool isInstrumentProviderSustainable(s_program_id program);
		static void setMIDICallback(MIDICallback callback);
		private:
		static MIDICallback callback;
	};
	EBCLASS(FixedRandom){
		private:
		yzrilyzr_array::SampleArray * data=nullptr;
		public:
		FixedRandom(yzrilyzr_array::SampleArray * data);
		FixedRandom();
		//double next();
		double next(size_t * index);
	};
}