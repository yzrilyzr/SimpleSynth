#pragma once
#include "interface/NoteProcessor.h"
#include "synth/generators/Osc.h"

namespace yzrilyzr_simplesynth{
	ECLASS(SquareWave, public Osc){
	public:
	SquareWave() : SquareWave(nullptr){}
	SquareWave(std::shared_ptr<PhaseSrc> freq) : Osc(freq){}
	u_sample getAmp(Note & note) override;
	yzrilyzr_lang::String toString() const override;
	};
}