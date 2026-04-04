#pragma once
#include "interface/NoteProcessor.h"

namespace yzrilyzr_simplesynth{
	ECLASS(SquareWave, public NoteProcessor){
	public:
	SquareWave(){}
	u_sample getAmp(const Note & note) override;
	yzrilyzr_lang::String toString() const override;
	U_CLASS_INFO(SquareWave)
	};
}