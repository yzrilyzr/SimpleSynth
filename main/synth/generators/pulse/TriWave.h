#pragma once
#include "synth/generators/Osc.h"

namespace yzrilyzr_simplesynth{
	ECLASS(TriWave, public Osc){
	public:
	TriWave() : TriWave(nullptr){}
	TriWave(std::shared_ptr<PhaseSrc> freq) : Osc(freq){}
	u_sample getAmp(Note & note) override;
	yzrilyzr_lang::String toString() const override;
	};
}