#pragma once
#include "interface/NoteProcessor.h"
#include "synth/generators/Osc.h"

namespace yzrilyzr_simplesynth{
	ECLASS(SawWave, public Osc){
	public:
	SawWave() : SawWave(nullptr){}
	SawWave(std::shared_ptr<PhaseSrc> freq) : Osc(freq){}
	u_sample getAmp(Note & note) override;
	std::string toString() const override;
	};
}