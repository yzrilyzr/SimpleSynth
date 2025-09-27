#pragma once
#include "synth/generators/Osc.h"
#include "interface/NoteProcessor.h"
#include "events/Note.h"
#include <memory>
#include "interface/PhaseSrc.h"
#include "yzrutil.h"
/**
* 根据note选取一个最邻近的NoteProcessor进行合成
*/

namespace yzrilyzr_simplesynth{
	ECLASS(NearestAmpSet, public Osc){
	public:
	NoteProcPtr notes[CHANNEL_MAX_NOTE_ID]={nullptr};
	~NearestAmpSet();
	NearestAmpSet(std::shared_ptr<PhaseSrc> freq);
	NearestAmpSet();
	std::string toString() const override;
	bool noMoreData(Note & note) override;
	u_sample getAmp(Note & note) override;
	};
	EBCLASS(NearestAmpSetBuilder){
	private:
	std::shared_ptr<NearestAmpSet> objptr=nullptr;
	public:
	NearestAmpSetBuilder();
	NearestAmpSetBuilder & add(int note, NoteProcPtr noteProcessor);
	NoteProcPtr build();
	};
}