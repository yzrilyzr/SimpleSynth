#include "MixerSequence.h"
#include "interface/IMixer.h"
#include "Mixer.h"
#include "Mixer2.h"
#include <algorithm>
using namespace yzrilyzr_collection;
namespace yzrilyzr_simplesynth{
	void MixerSequence::postToSequence(s_midichannel_id channel, ChannelEvent * n1, u_time startAt){
		n1->startAtTime=startAt;
		n1->channelID=channel;
		auto it=channelEvents.find(channel);
		if(it == channelEvents.end()){
			it=channelEvents.emplace(channel, std::vector<EventWrapper>()).first;
		}
		EventWrapper w={n1, it->second.size()};
		it->second.emplace_back(w);
	}
	bool MixerSequence::compareMixerEvents(const EventWrapper & a, const EventWrapper & b){
		if(a.event->startAtTime != b.event->startAtTime){
			// 时间不同：按时间排序
			return a.event->startAtTime < b.event->startAtTime;
		} else{
			// 时间相同：按原始索引排序（保持输入时的顺序）
			return a.index < b.index;
		}
	}
	void MixerSequence::sortPosted(){
		for(auto & entry : channelEvents){
			auto & ch=entry.second;
			std::sort(ch.begin(), ch.end(), compareMixerEvents);
		}
	}
	void MixerSequence::postToMixer(IMixer * mixer, u_time deltaLoadTime)const{
		postToMixer(mixer, deltaLoadTime, Mixer::DEFAULT_MIDI_CHANNEL_GROUP_NAME);
	}
	void MixerSequence::postToMixer(IMixer * mixer, u_time deltaLoadTime, const std::string & groupName)const{
		if(instrument != nullptr) mixer->setInstrumentProvider(instrument);
		u_time t1=mixer->getCurrentTime() + deltaLoadTime;
		if(auto m1=dynamic_cast<Mixer *>(mixer)){
			for(auto & entry : channelEvents){
				s_midichannel_id index=entry.first;
				auto & events=entry.second;
				for(auto & eventw : events){
					ChannelEvent * clone=eventw.event->clone();
					mixer->postEvent(clone, clone->startAtTime + t1);
				}
			}
		} else if(auto m2=dynamic_cast<Mixer2 *>(mixer)){
			std::vector<EventWrapper>eventsv;
			for(auto & entry : channelEvents){
				auto & events=entry.second;
				size_t indexInChannel=0;
				for(auto & eventw : events){
					ChannelEvent * clone=eventw.event->clone();
					clone->groupName=groupName;
					EventWrapper w={clone, indexInChannel};
					eventsv.emplace_back(w);
					indexInChannel++;
				}
			}
			std::sort(eventsv.begin(), eventsv.end(), compareMixerEvents);
			for(auto & entry : eventsv){
				m2->postEvent(entry.event, entry.event->startAtTime + t1);
			}
		}
	}
	void MixerSequence::setInstrument(std::shared_ptr<InstrumentProvider> midiInstrument){
		this->instrument=midiInstrument;
	}
	std::shared_ptr<InstrumentProvider> MixerSequence::getInstrument()const{
		return this->instrument;
	}
}