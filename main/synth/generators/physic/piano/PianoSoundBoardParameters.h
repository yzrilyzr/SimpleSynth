#pragma once
#include "SimpleSynth.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(PianoSoundBoardParameters){
	public:
		double eq1=0.0, eq2=0.0, eq3=0.0,eq4=0.0,eq5=0.0;
		double c1=0.0, c3=0.0;
	};
}
