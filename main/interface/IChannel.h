#pragma once
#include "SimpleSynth.h"
namespace yzrilyzr_dsp{
	class Chorus;
	class Freeverb;
	class Phaser;
}
namespace yzrilyzr_simplesynth{
	class IMixer;
	class ChannelConfig;
	EBCLASS(IChannel){
		protected:
		yzrilyzr_lang::String name;
		yzrilyzr_lang::String groupName;
		s_midichannel_id channelID=0;
		public:
		bool ENABLE_MIDI_CHANNEL_CONTROL=true;     //启用MIDI CC标准事件
		bool ENABLE_MIDI_PROGRAM_CHANGE=true;     //启用MIDI PC标准事件
		bool ENABLE_MIDI_CC_ADSR=false;//启用MIDI CC标准事件控制ADSR
		bool ENABLE_MIDI_CC_EFFECT=false;//启用MIDI CC标准事件控制效果器
		virtual void setSampleRate(u_sample_rate sr)=0;
		virtual u_sample * getOutput(uint32_t chIndex)const=0;
		virtual ChannelConfig & getConfig()=0;
		virtual yzrilyzr_dsp::Chorus & getChorus(u_index ch)const=0;
		virtual yzrilyzr_dsp::Freeverb & getReverb(u_index ch)const=0;
		virtual yzrilyzr_dsp::Phaser & getPhaser(u_index ch)const=0;
		void setDetune(u_normal_01_f v);
		void setChorus(u_normal_01_f v);
		void setPhaser(u_normal_01_f v);
		void setReverb(u_normal_01_f v);
		virtual void reset()=0;
		yzrilyzr_lang::String getName() const;
		void setName(const yzrilyzr_lang::String & name);
		virtual yzrilyzr_lang::String toString()const override;
		s_midichannel_id getChannelID()const;
		yzrilyzr_lang::String getGroupName()const;
	};
}