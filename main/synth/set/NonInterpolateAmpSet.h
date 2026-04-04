#pragma once
#include "SimpleSynth.h"

#include "interface/NoteProcessor.h"
#include "collection/HashMap.hpp"
/**
* 每个note独立NoteProcessor合成
*/

namespace yzrilyzr_simplesynth{
	ECLASS(NonInterpolateAmpSet, public NoteProcessor){
	public:
	NoteProcPtr set[CHANNEL_MAX_NOTE_ID];
	NonInterpolateAmpSet(){}
	NonInterpolateAmpSet & add(int note, NoteProcPtr noteProcessor);
	bool has(int note)const;
	u_sample getAmp(const Note & note)override;
	void init(ChannelConfig & cfg)override;
	bool noMoreData(const Note & note) const override;
	NoteProcPtr clone()override;
	U_CLASS_INFO(NonInterpolateAmpSet)
	};
}