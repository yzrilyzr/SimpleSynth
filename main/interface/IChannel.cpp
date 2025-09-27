#include "IChannel.h"
#include "IMixer.h"
namespace yzrilyzr_simplesynth{
	void IChannel::setInstrumentProvider(std::shared_ptr<InstrumentProvider> instr){
		instrument=instr;
	}
	std::shared_ptr<InstrumentProvider> IChannel::getInstrumentProvider()const{
		if(instrument == nullptr)return mixer->getInstrumentProvider();
		return instrument;
	}
	std::string IChannel::toString()const{
		return "IChannel";
	}
	IMixer * IChannel::getMixer()const{
		return mixer;
	}
	void IChannel::setMixer(IMixer * pMixer){
		mixer=pMixer;
	}
	std::string IChannel::getName() const{
		return name;
	}
	s_midichannel_id IChannel::getChannelID()const{
		return channelID;
	}
	std::string IChannel::getGroupName()const{
		return groupName;
	}
	void IChannel::setName(const std::string & nam){
		name=nam;
	}
}