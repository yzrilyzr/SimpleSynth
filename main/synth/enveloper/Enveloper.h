#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"

namespace yzrilyzr_simplesynth{
	ECLASS(Enveloper,public NoteProcessor){
	public:
	U_CLASS_INFO(Enveloper);
	};
}