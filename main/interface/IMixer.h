#pragma once
#include "SimpleSynth.h"
#include "array/Array.hpp"
#include "events/ChannelConfig.h"
#include <memory>
#include <shared_mutex>
#include <vector>
#include <future>
#include <optional>

namespace yzrilyzr_dsp{
	class DSPChain;
	class DSP3D;
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
		int8_t synthMode=0;                       // 合成模式
		u_index outputChannelCount=2;
		std::shared_mutex mixLock;
		std::shared_mutex dspLock;
		std::shared_mutex eventLock;
		std::shared_mutex channelLock;
		u_time_f processTime=0;
		ChannelConfig globalChannelConfig;
		std::optional<std::future<void>> mixFuture;
		bool enable_MIDI_CC_EFFECT=false;
		bool enable_MIDI_CC_ADSR=false;

		
		public:
		static constexpr const int8_t MODE_SINGLE_THREAD=0;
		static constexpr const int8_t MODE_FUTURE=1;
		static constexpr const int8_t MODE_THREAD_POOL=2;
		static constexpr const char * const DEFAULT_MIDI_CHANNEL_GROUP_NAME="DefaultMIDIChannelGroup";

		virtual void mix()=0;
		virtual void asyncMix();
		virtual void awaitMix();

		virtual u_index getBufferSize()const=0;
		virtual void setBufferSize(u_index bs)=0;

		u_index getOutputChannelCount()const;
		virtual u_sample * getOutput(uint32_t chIndex)const=0;

		virtual std::vector<u_sp<IChannel>> getAllChannels()const=0;

		bool hasMIDIChannel(s_midichannel_id id);
		virtual bool hasMIDIChannel(const yzrilyzr_lang::String & group, s_midichannel_id id)=0;
		u_sp<IChannel> getMIDIChannel(s_midichannel_id id);
		virtual u_sp<IChannel> getMIDIChannel(const yzrilyzr_lang::String & group, s_midichannel_id ch)=0;
		static bool isDrumSetChannel(s_midichannel_id id);

		int8_t getSynthMode() const;
		virtual void setSynthMode(int8_t mode, int32_t cores)=0;

		ChannelConfig & getGlobalConfig();

		u_sample_rate getSampleRate()const;
		virtual void setSampleRate(u_sample_rate sr);

		virtual void sendInstantEvent(ChannelEvent * event)=0;
		virtual void postEvent(ChannelEvent * event, u_time startAt)=0;

		virtual void resetLimiter()=0;
		virtual u_time getCurrentTime()const=0;
		virtual u_sp<yzrilyzr_dsp::DSPChain> * getEQ()=0;
		virtual void reset()=0;
		virtual bool hasData()=0;
		virtual u_index getCurrentProcessingNoteCount()=0;
		virtual u_index getPostedEventCount()=0;
		virtual s_sample_index getCurrentSampleIndex() const=0;

		void setUseEQ(bool use);
		bool isUseEQ() const;
		void setUseLimiter(bool use);
		bool isUseLimiter() const;
		void setChannelUseDSP(bool use);
		bool isChannelUseDSP() const;
		u_time_f getProcessTime() const;
		u_time_f getProcessStandardTime() const;
		virtual yzrilyzr_lang::String toString()const override;
		std::shared_mutex & getDSPLock();
		std::shared_mutex & getEventLock();
		std::shared_mutex & getChannelLock();
	};
}