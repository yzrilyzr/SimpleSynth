#pragma once
#include "interface/NoteProcessor.h"
#include "events/Note.h"

namespace yzrilyzr_simplesynth{
	ECLASS(NoiseSrc, public NoteProcessor){
	public:
	NoiseSrc(){}
	u_sample getAmp(Note & note) override;
	std::string toString() const override;
	};
}