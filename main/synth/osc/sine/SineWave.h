#pragma once
#include "interface/NoteProcessor.h"
#include "events/Note.h"

namespace yzrilyzr_simplesynth{
	ECLASS(SineWave, public NoteProcessor){
	public:
	SineWave(){}
	u_sample getAmp(const Note & note) override;
	void getAmpBlock(const Note * note, u_sample * output, u_index length) override;
	yzrilyzr_lang::String toString()const override;
	U_CLASS_INFO(SineWave);
	};
}