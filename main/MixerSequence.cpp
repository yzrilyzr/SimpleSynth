#include "MixerSequence.h"
#include "interface/IMixer.h"
#include "Mixer.h"
#include "Mixer2.h"
#include <algorithm>
using namespace yzrilyzr_collection;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void MixerSequence::postToSequence(s_midichannel_id channel, u_up<ChannelEvent>  n1, u_time startAt){
		n1->startAtTime=startAt;
		n1->channelID=channel;
		auto it=channelEvents.find(channel);
		if(it == channelEvents.end()){
			it=channelEvents.emplace(channel, std::vector<EventWrapper>()).first;
		}		
		it->second.emplace_back(EventWrapper {std::move(n1), static_cast<u_index>(it->second.size())});
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
	void MixerSequence::postToMixer(IMixer * mixer, u_time startDelay)const{
		postToMixer(mixer, startDelay, Mixer::DEFAULT_MIDI_CHANNEL_GROUP_NAME);
	}
	void MixerSequence::postToMixer(IMixer * mixer, u_time startDelay, const String & groupName)const{
		postToMixer(mixer, startDelay, 0, groupName);
	}
	void MixerSequence::postToMixer(IMixer * mixer, u_time startDelay, u_time sequenceOffset, const String & groupName)const{
		if(instrument != nullptr) mixer->getGlobalConfig().setInstrumentProvider(instrument);
		if(auto m1=U_INSTANCE_OF(Mixer, mixer)){
			u_time t1=mixer->getCurrentTime() + startDelay - sequenceOffset;
			for(auto & entry : channelEvents){
				s_midichannel_id index=entry.first;
				auto & events=entry.second;
				for(auto & eventw : events){
					u_up<ChannelEvent>  clone=eventw.event->clone();
					mixer->postEvent(std::move(clone), clone->startAtTime + t1);
				}
			}
		} else if(auto m2=U_INSTANCE_OF(Mixer2, mixer)){
			std::vector<EventWrapper>eventsv;
			for(auto & entry : channelEvents){
				auto & events=entry.second;
				u_index indexInChannel=0;
				for(auto & eventw : events){
					u_up<ChannelEvent>  clone=eventw.event->clone();
					clone->groupName=groupName;
					eventsv.emplace_back(EventWrapper{std::move(clone), indexInChannel});
					indexInChannel++;
				}
			}
			std::sort(eventsv.begin(), eventsv.end(), compareMixerEvents);
			u_time firstNoteAppearTime=0;
			for(auto & entry : eventsv){
				auto &ev=entry.event;
				if(ev->getType() == EventType::NOTE_ON){
					firstNoteAppearTime=ev->startAtTime;
					break;
				}
			}
			u_time curTime=mixer->getCurrentTime();
			u_time postAtTime=startDelay - sequenceOffset - firstNoteAppearTime;
			for(auto & entry : eventsv){
				auto &ev=entry.event;
				if(ev->startAtTime + postAtTime < 0){
					auto et=ev->getType();
					switch(et){
						case EventType::NOTE_ON:
						case EventType::NOTE_OFF:
						case EventType::NOTE_PRESSURE:
						case EventType::NOTE_PITCH_BEND:
							continue;
					}
				}
				m2->postEvent(std::move(ev), ev->startAtTime + curTime + postAtTime);
			}
		}
	}

	void MixerSequence::setInstrument(u_sp<InstrumentProvider> midiInstrument){
		this->instrument=midiInstrument;
	}
	u_sp<InstrumentProvider> MixerSequence::getInstrument()const{
		return this->instrument;
	}
	u_time MixerSequence::getDuration()const{
		u_time maxTime=0;
		for(auto & entry : channelEvents){
			s_midichannel_id index=entry.first;
			auto & events=entry.second;
			for(auto & eventw : events){
				maxTime=Math::max(eventw.event->startAtTime, maxTime);
			}
		}
		return maxTime;
	}
}