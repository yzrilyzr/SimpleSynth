#pragma once
#include "interface/NoteProcessor.h"
#include "events/Note.h"
#include "SineHarmonicWave.h"

namespace yzrilyzr_simplesynth{
	ECLASS(AHarmonicWave, public SineHarmonicWave){
	public:
	AHarmonicWave();
	};
}