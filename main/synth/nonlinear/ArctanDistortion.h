#pragma once
#include "SimpleSynth.h"
#include "synth/composed/AmpUnaryComposition.h"

namespace yzrilyzr_simplesynth{
	ECLASS(ArctanDistortion, public AmpUnaryComposition){
	private:
	double alpha;
	u_sample inputGain;
	u_sample outputGain;
	public:
	ArctanDistortion();
	ArctanDistortion(NoteProcPtr a, u_sample inputGain, double alpha, u_sample outputGain);
	u_sample getAmp(const Note & note) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	void onRegisterParam() override;
	U_CLASS_INFO(ArctanDistortion)
	};
}