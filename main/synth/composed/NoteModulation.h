//TODO
//调制音符所有数据
#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "AmpUnaryComposition.h"


namespace yzrilyzr_simplesynth{
	ECLASS(NoteModulation, public AmpUnaryComposition){
	private:
	bool isMod=false;
	public:
	
	~NoteModulation()=default;
	NoteModulation() :AmpUnaryComposition(nullptr){}
	NoteModulation(NoteProcPtr a) :AmpUnaryComposition(a){}
	virtual void applyMod(Note & note);
	u_sample getAmp(const Note & note) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	void onRegisterParam() override;
	U_GET_CLASS_NAME(NoteModulation)
	};
}