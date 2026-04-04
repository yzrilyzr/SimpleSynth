#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"


namespace yzrilyzr_simplesynth{
	ECLASS(NeighbourMixAmpSet, public NoteProcessor){
	private:
		NoteProcPtr notes[CHANNEL_MAX_NOTE_ID][2]={nullptr};
		double notesRatio[CHANNEL_MAX_NOTE_ID][2]={0};
	public:
		NeighbourMixAmpSet();
		NeighbourMixAmpSet *add(int note, NoteProcPtr noteProcessor);
		yzrilyzr_lang::String toString() const override;
		bool noMoreData(const Note & note) const override;
		NeighbourMixAmpSet *build();
		u_sample getAmp(const Note & note) override;
	};
}