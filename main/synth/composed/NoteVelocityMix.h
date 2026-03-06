#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "synth/composed/AmpUnaryComposition.h"

namespace yzrilyzr_simplesynth{
	/**
	* 混合原始力度和指定力度
	* 0:全部原始，1:全部指定
	*/
	ECLASS(NoteVelocityMix, public AmpUnaryComposition){
	public:
	u_normal_01 mix=0;
	s_note_vel overrideKeyVel=1;
	NoteVelocityMix();
	NoteVelocityMix(NoteProcPtr a, u_normal_01 mix);
	NoteVelocityMix(NoteProcPtr a, s_note_vel ovrd,u_normal_01 mix);
	u_sample getAmp(const Note & note)override;
	yzrilyzr_lang::String toString() const override;
	};
}