#include "IChannel.h"
#include "IMixer.h"
#include "dsp/Phaser.h"
#include "dsp/Freeverb.h"
#include "dsp/Chorus.h"
#include "util/Util.h"

using namespace yzrilyzr_lang;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
namespace yzrilyzr_simplesynth{
	String IChannel::toString()const{
		return "IChannel";
	}
	String IChannel::getName() const{
		return name;
	}
	s_midichannel_id IChannel::getChannelID()const{
		return channelID;
	}
	String IChannel::getGroupName()const{
		return groupName;
	}
	void IChannel::setName(const String & nam){
		name=nam;
	}
	void IChannel::setChorus(u_normal_01_f chorus){
		auto & cfg=getConfig();
		u_sample_rate sampleRate=cfg.sampleRate;
		u_index channelCount=cfg.mixer->getOutputChannelCount();
		for(u_index i=0;i < channelCount;i++){
			auto & au=getChorus(i);
			au.depthMs=50.0f * chorus;
			au.init(sampleRate);
		}
	}
	void IChannel::setReverb(u_normal_01_f reverb){
		auto & cfg=getConfig();
		u_sample_rate sampleRate=cfg.sampleRate;
		u_index channelCount=cfg.mixer->getOutputChannelCount();
		for(u_index i=0;i < channelCount;i++){
			auto & au=getReverb(i);
			au.roomSize=reverb * 0.95f;
			au.damper=Util::clamp(0.3f - reverb * 0.3f, 0.0f, 0.3f);
			au.wetRatio=Util::clamp(reverb, 0.0f, 0.5f);
			au.init(sampleRate);
		}
	}
	void IChannel::setDetune(u_normal_01_f detune){
		auto & cfg=getConfig();
		cfg.Detune=detune;
	}
	void IChannel::setPhaser(u_normal_01_f cphase){
		auto & cfg=getConfig();
		u_sample_rate sampleRate=cfg.sampleRate;
		u_index channelCount=cfg.mixer->getOutputChannelCount();
		for(u_index i=0; i < channelCount; i++){
			auto & au=getPhaser(i);
			au.wetRatio=cphase;
			au.init(sampleRate);
		}
	}
	bool IChannel::isDrumSetChannel()const{
		return IMixer::isDrumSetChannel(channelID);
	}
}