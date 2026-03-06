#pragma once
#include "SimpleSynth.h"
#include "synth/composed/AmpUnaryComposition.h"

namespace yzrilyzr_simplesynth{
	ECLASS(TapeSaturationDistortion, public AmpUnaryComposition){
	private:
	double inputGain;
	double drive;        // 驱动强度（控制饱和程度）
	double bias;         // 直流偏置（模拟磁带磁头偏置，增加泛音复杂度）
	double outputGain;
	public:
	TapeSaturationDistortion();
	TapeSaturationDistortion(NoteProcPtr a, double inputGain, double drive, double bias, double outputGain);
	u_sample getAmp(const Note & note) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	};
}