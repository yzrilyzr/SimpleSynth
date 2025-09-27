#pragma once
#include "SimpleSynth.h"
namespace yzrilyzr_dsp{
	class Chorus;
	class Freeverb;
	class Phaser;
}
namespace yzrilyzr_simplesynth{
	class IMixer;
	class InstrumentProvider;
	class NoteTuning;
	class NoteProcessor;
	class ChannelConfig;
	typedef std::shared_ptr<NoteProcessor> NoteProcPtr;
	EBCLASS(IChannel){
		protected:
		std::shared_ptr<InstrumentProvider> instrument=nullptr;
		std::shared_ptr<NoteTuning> tuning=nullptr;
		IMixer * mixer=nullptr;
		std::string name;
		std::string groupName;
		s_midichannel_id channelID=0;
		NoteProcPtr noteProcessor=nullptr;
		public:
		bool ENABLE_MIDI_CHANNEL_CONTROL=true;     //启用MIDI CC标准事件
		bool ENABLE_MIDI_PROGRAM_CHANGE=true;     //启用MIDI PC标准事件
		bool ENABLE_MIDI_CC_ADSR=false;//启用MIDI CC标准事件控制ADSR
		bool ENABLE_MIDI_CC_EFFECT=false;//启用MIDI CC标准事件控制效果器
		virtual void setSampleRate(u_sample_rate sr)=0;
		virtual u_sample * getOutput(uint32_t chIndex)const=0;
		virtual ChannelConfig & getConfig()=0;
		void setInstrumentProvider(std::shared_ptr<InstrumentProvider> instr);
		std::shared_ptr<InstrumentProvider> getInstrumentProvider()const;
		virtual yzrilyzr_dsp::Chorus & getChorus(size_t ch)const=0;
		virtual yzrilyzr_dsp::Freeverb & getReverb(size_t ch)const=0;
		virtual yzrilyzr_dsp::Phaser & getPhaser(size_t ch)const=0;
		virtual void reset()=0;
		IMixer * getMixer()const;
		void setMixer(IMixer * pMixer);
		std::string getName() const;
		void setName(const std::string & name);
		virtual std::string toString()const override;
		s_midichannel_id getChannelID()const;
		std::string getGroupName()const;
		bool isDrumSetChannel()const{
			return channelID == 9;
		}
	};
}