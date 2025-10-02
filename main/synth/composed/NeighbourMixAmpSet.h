#pragma once
#include "SimpleSynth.h"

#include "interface/NoteProcessor.h"
#include "synth/generators/Osc.h"


namespace yzrilyzr_simplesynth{
	ECLASS(NeighbourMixAmpSet, public Osc){
	private:
		NoteProcPtr notes[CHANNEL_MAX_NOTE_ID][2]={nullptr};
		double notesRatio[CHANNEL_MAX_NOTE_ID][2]={0};
	public:
		NeighbourMixAmpSet(std::shared_ptr<PhaseSrc> freq);
		NeighbourMixAmpSet();
		NeighbourMixAmpSet *add(int note, NoteProcPtr noteProcessor);
		yzrilyzr_lang::String toString() const override;
		bool noMoreData(Note &note) override;
		NeighbourMixAmpSet *build();
		u_sample getAmp(Note &note) override;
	};
}