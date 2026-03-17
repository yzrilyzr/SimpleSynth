#pragma once
#include "interface/NoteProcessor.h"
#include "events/Note.h"

namespace yzrilyzr_simplesynth{
	ECLASS(NoiseSrc, public NoteProcessor){
	public:
	NoiseSrc(){}
	u_sample getAmp(const Note & note) override;
	yzrilyzr_lang::String toString() const override;
	U_CLASS_INFO(NoiseSrc)

	};
}