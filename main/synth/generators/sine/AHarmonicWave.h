#pragma once
#include "synth/generators/Osc.h"
#include "events/Note.h"
#include "SineHarmonicWave.h"

namespace yzrilyzr_simplesynth{
	ECLASS(AHarmonicWave, public SineHarmonicWave){
	public:
		AHarmonicWave(): AHarmonicWave(nullptr){}
		AHarmonicWave(std::shared_ptr<PhaseSrc> freq);
	};
}