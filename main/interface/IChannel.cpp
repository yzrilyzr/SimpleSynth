#include "IChannel.h"
#include "IMixer.h"
using namespace yzrilyzr_lang;
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
}