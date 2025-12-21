#include "NonInterpolateAmpSet.h"
#include "collection/HashMap.hpp"
#include "events/Note.h"

namespace yzrilyzr_simplesynth{
	NonInterpolateAmpSet & NonInterpolateAmpSet::add(int note, NoteProcPtr noteProcessor){
		set[note]=noteProcessor;
		return *this;
	}
	bool NonInterpolateAmpSet::has(int note)const{
		return set[note] != nullptr;
	}
	u_sample NonInterpolateAmpSet::getAmp(Note & note){
		NoteProcPtr src=set[note.id];
		if(src == nullptr) return 0;
		return src->getAmp(note);
	}
	void NonInterpolateAmpSet::init(ChannelConfig & cfg){
		NoteProcessor::init(cfg);
		for(auto & src : set){
			if(src == nullptr)continue;
			src->init(cfg);
		}
	}
	bool NonInterpolateAmpSet::noMoreData(Note & note){
		NoteProcPtr src=set[note.id];
		if(src == nullptr) return true;
		return src->noMoreData(note);
	}
	NoteProcPtr NonInterpolateAmpSet::clone(){
		return u_sp<NonInterpolateAmpSet>(this);
	}
}