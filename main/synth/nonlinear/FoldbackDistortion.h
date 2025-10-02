#pragma once
#include "SimpleSynth.h"
#include "synth/composed/AmpUnaryComposition.h"

namespace yzrilyzr_simplesynth{
	ECLASS(FoldbackDistortion, public AmpUnaryComposition){
	private:
	double inputGain;
	double threshold;    // 折叠阈值
	double foldRatio;    // 折叠比率（控制折叠后的信号衰减）
	double outputGain;
	public:
	FoldbackDistortion();
	FoldbackDistortion(NoteProcPtr a, double inputGain, double threshold, double foldRatio, double outputGain);
	u_sample getAmp(Note & note) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	};
}