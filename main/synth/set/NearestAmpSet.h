#pragma once
#include "events/Note.h"
#include "interface/NoteProcessor.h"
#include "interface/PhaseSrc.h"
#include "yzrutil.h"
/**
* 根据note选取一个最邻近的NoteProcessor进行合成
*/

namespace yzrilyzr_simplesynth{
	ECLASS(NearestAmpSet, public NoteProcessor){
	public:
	NoteProcPtr notes[CHANNEL_MAX_NOTE_ID]={nullptr};
	~NearestAmpSet();
	NearestAmpSet();
	yzrilyzr_lang::String toString() const override;
	bool noMoreData(const Note & note) const override;
	u_sample getAmp(const Note & note) override;
	};
	EBCLASS(NearestAmpSetBuilder){
	private:
	u_sp<NearestAmpSet> objptr=nullptr;
	public:
	NearestAmpSetBuilder();
	NearestAmpSetBuilder & add(int note, NoteProcPtr noteProcessor);
	NoteProcPtr build();
	};
}