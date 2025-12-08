#pragma once
#include "events/ChannelEvent.h"
#include "interface/InstrumentProvider.h"

namespace yzrilyzr_simplesynth{
	class IMixer;
	class Mixer;
	class Mixer2;
	EBCLASS(MixerSequence){
		private:
		struct EventWrapper{
			ChannelEvent * event;
			u_index index;
		};
		std::shared_ptr<InstrumentProvider> instrument=nullptr;
		static bool compareMixerEvents(const EventWrapper & a, const EventWrapper & b);
		public:
		
		std::unordered_map<s_midichannel_id, std::vector<EventWrapper>> channelEvents;
		void postToSequence(s_midichannel_id channel, ChannelEvent * n1, u_time startAt);
		void sortPosted();
		void postToMixer(IMixer * mixer, u_time startDelay)const;
		void postToMixer(IMixer * mixer, u_time startDelay, u_time sequenceOffset, const yzrilyzr_lang::String & groupName)const;
		void postToMixer(IMixer * mixer, u_time startDelay, const yzrilyzr_lang::String & groupName)const;
		void setInstrument(std::shared_ptr<InstrumentProvider> midiInstrument);
		std::shared_ptr<InstrumentProvider> getInstrument()const;
		u_time getDuration()const;
	};
}