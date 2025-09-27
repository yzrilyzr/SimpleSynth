#pragma once
#include "SimpleSynth.h"
#include <memory>
#include <vector>
#include <mutex>

namespace yzrilyzr_dsp{
	class DSPChain;
}
namespace yzrilyzr_interpolator{
	class Interpolator;
}
namespace yzrilyzr_simplesynth{
	class IChannel;
	class InstrumentProvider;
	class NoteTuning;
	class ChannelEvent;
	EBCLASS(IMixer){
		protected:
		bool useLimiter=true;
		bool useEQ=false;
		bool channelUseDSP=true;
		u_sample_rate sampleRate=0;
		int8_t synthMode=0;                       // 合成模式
		size_t outputChannelCount=2;
		std::shared_ptr<InstrumentProvider> instrument=nullptr;
		std::shared_ptr<NoteTuning> tuning=nullptr;
		std::shared_ptr<yzrilyzr_interpolator::Interpolator> velocityMap=nullptr;
		std::mutex mixLock;
		std::mutex dspLock;
		std::mutex eventLock;
		std::mutex channelLock;
        u_time_f processTime=0;

		public:
		static constexpr const int8_t MODE_SINGLE_THREAD=0;
		static constexpr const int8_t MODE_FUTURE=1;
		static constexpr const int8_t MODE_THREAD_POOL=2;
		static constexpr const s_midichannel_id MIDI_DRUM_CHANNEL=9;
		static constexpr const char * const DEFAULT_MIDI_CHANNEL_GROUP_NAME="DefaultMIDIChannelGroup";

		virtual void mix()=0;

		virtual size_t getBufferSize()const=0;
		virtual void setBufferSize(size_t bs)=0;

		size_t getOutputChannelCount()const;
		virtual u_sample * getOutput(uint32_t chIndex)const=0;

		virtual std::vector<std::shared_ptr<IChannel>> getAllChannels()const=0;

		bool hasMIDIChannel(s_midichannel_id id);
		virtual bool hasMIDIChannel(const std::string & group, s_midichannel_id id)=0;
		std::shared_ptr<IChannel> getMIDIChannel(s_midichannel_id id);
		virtual std::shared_ptr<IChannel> getMIDIChannel(const std::string & group, s_midichannel_id ch)=0;
		
		int8_t getSynthMode() const;
		virtual void setSynthMode(int8_t mode, int32_t cores)=0;

		u_sample_rate getSampleRate()const;
		virtual void setSampleRate(u_sample_rate sr);
		
		std::shared_ptr<InstrumentProvider> getInstrumentProvider()const;
		void setInstrumentProvider(std::shared_ptr<InstrumentProvider> instr);

		std::shared_ptr<NoteTuning> getNoteTuning()const;
		void setNoteTuning(std::shared_ptr<NoteTuning> tun);

		std::shared_ptr<yzrilyzr_interpolator::Interpolator> getNoteVelocityMap()const;
		void setNoteVelocityMap(std::shared_ptr<yzrilyzr_interpolator::Interpolator> val);

		virtual void sendInstantEvent(ChannelEvent * event)=0;
		virtual void postEvent(ChannelEvent * event, u_time startAt)=0;

		virtual void resetLimiter()=0;
		virtual u_time getCurrentTime()const=0;
		virtual std::shared_ptr<yzrilyzr_dsp::DSPChain> * getEQ()=0;
		virtual void reset()=0;
		virtual bool hasData()=0;
		virtual size_t getCurrentProcessingNoteCount()=0;
		virtual size_t getPostedEventCount()=0;
		virtual s_sample_index getCurrentSampleIndex() const=0;
		void setUseEQ(bool use);
		bool isUseEQ() const;
		void setUseLimiter(bool use);
		bool isUseLimiter() const;
        void setChannelUseDSP(bool use);
		bool isChannelUseDSP() const;
        u_time_f getProcessTime() const;
		u_time_f getProcessStandardTime() const;
		virtual std::string toString()const override;
		std::mutex & getDSPLock();
		std::mutex & getEventLock();
		std::mutex & getChannelLock();
	};
}