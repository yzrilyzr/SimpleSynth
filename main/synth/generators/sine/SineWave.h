#pragma once
#include "synth/generators/Osc.h"
#include "events/Note.h"

namespace yzrilyzr_simplesynth{
	ECLASS(SineWave, public Osc){
	public:
	SineWave() : SineWave(nullptr){}
	SineWave(u_sp<PhaseSrc> freq) : Osc(freq){}
	u_sample getAmp(Note & note) override;
	yzrilyzr_lang::String toString()const override;
	};
}