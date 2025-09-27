#pragma once
#include "synth/generators/Osc.h"
#include "events/Note.h"

namespace yzrilyzr_simplesynth{
	ECLASS(SineWave, public Osc){
	public:
	SineWave() : SineWave(nullptr){}
	SineWave(std::shared_ptr<PhaseSrc> freq) : Osc(freq){}
	u_sample getAmp(Note & note) override;
	std::string toString()const override;
	};
}