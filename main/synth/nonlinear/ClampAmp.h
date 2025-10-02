#pragma once
#include "SimpleSynth.h"
#include "synth/composed/AmpUnaryComposition.h"

namespace yzrilyzr_simplesynth{
	ECLASS(ClampAmp, public AmpUnaryComposition){
	private:
	u_sample clamp=1;
	u_sample inputGain=1;
	u_sample outputGain=1;
	public:
	ClampAmp();
	ClampAmp(NoteProcPtr a, u_sample inputGain, u_sample clamp, u_sample outputGain);
	u_sample getAmp(Note & note) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	};
}