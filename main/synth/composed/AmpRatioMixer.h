#pragma once
#include "AmpBinaryComposition.h"
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"

namespace yzrilyzr_simplesynth{
	ECLASS(AmpRatioMixer, public AmpBinaryComposition){
	public:
	u_normal_11 ratio=0;
	AmpRatioMixer() : AmpRatioMixer(nullptr, nullptr, 0){
		registerParamNormal11("Ratio", &ratio);
	}
	AmpRatioMixer(NoteProcPtr a, NoteProcPtr b, u_normal_11 ratio) :ratio(ratio), AmpBinaryComposition(a, b){}
	u_sample getAmp(Note & note) override{
		return a->getAmp(note) * (ratio + 1.0) / 2.0 + b->getAmp(note) * (-ratio + 1.0) / 2.0;
	}
	bool noMoreData(Note & note) override{
		return a->noMoreData(note) && b->noMoreData(note);
	}
	NoteProcPtr clone() override{
		return std::make_shared<AmpRatioMixer>(a->clone(), b->clone(), ratio);
	}
	std::string toString() const override;
	};
}