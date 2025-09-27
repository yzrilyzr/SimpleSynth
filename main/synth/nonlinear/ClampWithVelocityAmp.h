#pragma once
#include "SimpleSynth.h"
#include "synth/composed/AmpUnaryComposition.h"

namespace yzrilyzr_simplesynth{
	ECLASS(ClampWithVelocityAmp, public AmpUnaryComposition){
	private:
	u_sample clamp=1;
	u_sample inputGain=1;
	u_sample outputGain=1;
	public:
	ClampWithVelocityAmp();
	ClampWithVelocityAmp(NoteProcPtr a, u_sample inputGain, u_sample clamp, u_sample outputGain);
	u_sample getAmp(Note & note) override;
	NoteProcPtr clone() override;
	std::string toString() const override;
	};
}