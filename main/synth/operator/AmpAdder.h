#pragma once
#include "SimpleSynth.h"
#include "synth/composed/AmpBinaryComposition.h"

namespace yzrilyzr_simplesynth{
	ECLASS(AmpAdder, public AmpBinaryComposition){
	public:
	AmpAdder() : AmpAdder(nullptr, nullptr){}
	AmpAdder(NoteProcPtr a, NoteProcPtr b) : AmpBinaryComposition(a, b){}
	inline u_sample getAmp(const Note & note) override{
		return a->getAmp(note) + b->getAmp(note);
	}
	inline  bool noMoreData(const Note & note)const override{
		return a->noMoreData(note) && b->noMoreData(note);
	}
	NoteProcPtr clone() override{
		return mksp<AmpAdder>(a->clone(), b->clone());
	}
	void getAmpBlock(const Note * note, u_sample * output, u_index length)override;
	yzrilyzr_lang::String toString() const override;
	U_CLASS_INFO(AmpAdder);
	};
}