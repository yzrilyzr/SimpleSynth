#pragma once
#include "interface/NoteProcessor.h"
#include "synth/generators/Osc.h"

namespace yzrilyzr_simplesynth{
	ECLASS(SquareWave, public Osc){
	public:
	SquareWave() : SquareWave(nullptr){}
	SquareWave(u_sp<PhaseSrc> freq) : Osc(freq){}
	u_sample getAmp(const Note & note) override;
	yzrilyzr_lang::String toString() const override;
	U_GET_CLASS_NAME(SquareWave)
	};
}