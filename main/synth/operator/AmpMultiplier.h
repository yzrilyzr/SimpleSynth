#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "synth/composed/AmpBinaryComposition.h"

namespace yzrilyzr_simplesynth{
	ECLASS(AmpMultiplier, public AmpBinaryComposition){
	public:
	AmpMultiplier() : AmpMultiplier(nullptr, nullptr){}
	AmpMultiplier(NoteProcPtr a, NoteProcPtr b) : AmpBinaryComposition(a, b){}
	inline u_sample getAmp(const Note & note) override{
		return a->getAmp(note) * b->getAmp(note);
	}
	bool noMoreData(const Note & note)const override{
		return a->noMoreData(note) && b->noMoreData(note);
	}
	NoteProcPtr clone() override{
		return mksp<AmpMultiplier>(a->clone(), b->clone());
	}
	void getAmpBlock(const Note * note, u_sample * output, u_index length)override;
	yzrilyzr_lang::String toString() const override;
	U_CLASS_INFO(AmpMultiplier);
	};
}
