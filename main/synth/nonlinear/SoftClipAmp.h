#pragma once
#include "SimpleSynth.h"
#include "synth/composed/AmpUnaryComposition.h"

namespace yzrilyzr_simplesynth{
	ECLASS(SoftClipAmp, public AmpUnaryComposition){
	private:
	double inputGain;
	double threshold;    // 软剪辑阈值
	double outputGain;
	public:
	SoftClipAmp();
	SoftClipAmp(NoteProcPtr a, double inputGain, double threshold, double outputGain);
	u_sample getAmp(Note & note) override;
	NoteProcPtr clone() override;
	std::string toString() const override;
	};
}