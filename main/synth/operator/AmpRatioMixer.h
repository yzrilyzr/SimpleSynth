#pragma once
#include "synth/composed/AmpBinaryComposition.h"
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"

namespace yzrilyzr_simplesynth{
	ECLASS(AmpRatioMixer, public AmpBinaryComposition){
	public:
	u_normal_11 ratio=0;
	AmpRatioMixer() : AmpRatioMixer(nullptr, nullptr, 0){}
	AmpRatioMixer(NoteProcPtr a, NoteProcPtr b, u_normal_11 ratio) :ratio(ratio), AmpBinaryComposition(a, b){}
	u_sample getAmp(const Note & note) override{
		return a->getAmp(note) * (ratio + 1.0) / 2.0 + b->getAmp(note) * (-ratio + 1.0) / 2.0;
	}
	bool noMoreData(const Note & note)const override{
		return a->noMoreData(note) && b->noMoreData(note);
	}
	NoteProcPtr clone() override{
		return mksp<AmpRatioMixer>(a->clone(), b->clone(), ratio);
	}
	yzrilyzr_lang::String toString() const override;
	void onRegisterParam() override;
	};
}