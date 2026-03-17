#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "AmpBinaryComposition.h"

namespace yzrilyzr_simplesynth{
	ECLASS(AmpMultiplier, public AmpBinaryComposition){
	public:
	AmpMultiplier() : AmpMultiplier(nullptr, nullptr){}
	AmpMultiplier(NoteProcPtr a, NoteProcPtr b) : AmpBinaryComposition(a, b){}
	inline u_sample getAmp(const Note & note) override{
		return a->getAmp(note) * b->getAmp(note);
	}
	bool noMoreData(const Note & note) override{
		return a->noMoreData(note) && b->noMoreData(note);
	}
	NoteProcPtr clone() override{
		return mksp<AmpMultiplier>(a->clone(), b->clone());
	}
	yzrilyzr_lang::String toString() const override;
	U_CLASS_INFO(AmpMultiplier);
	};
}
