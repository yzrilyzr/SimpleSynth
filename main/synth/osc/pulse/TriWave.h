#pragma once
#include "interface/NoteProcessor.h"

namespace yzrilyzr_simplesynth{
	ECLASS(TriWave, public NoteProcessor){
	public:
	TriWave(){}
	u_sample getAmp(const Note & note) override;
	yzrilyzr_lang::String toString() const override;
	U_CLASS_INFO(TriWave)
	};
}