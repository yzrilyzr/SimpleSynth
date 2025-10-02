#pragma once
#include "SimpleSynth.h"
#include "events/NoteData.hpp"
#include "interface/NoteProcessor.h"
#include "AmpBinaryComposition.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(FreqModKeyData){
	public:
	s_phase phaseSynth=0;
	};

	ECLASS(FreqModAmp, public AmpBinaryComposition, NoteData<FreqModKeyData>){
	public:
	u_sample depth=0;
	FreqModAmp();
	FreqModAmp(NoteProcPtr dst, NoteProcPtr src, u_sample depth);
	u_sample getAmp(Note & note) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	FreqModKeyData * init(FreqModKeyData * data, Note & note) override;
	};
}