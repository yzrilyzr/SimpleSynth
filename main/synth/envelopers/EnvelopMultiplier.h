#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "synth/composed/AmpMultiplier.h"
#include "Enveloper.h"
#include "events/Note.h"

namespace yzrilyzr_simplesynth{
	ECLASS(EnvelopMultiplier, public AmpMultiplier), public Enveloper{
	public:
	EnvelopMultiplier(){
		registerParamAlias("A", "Env");
		registerParamAlias("B", "Src");
	}
	EnvelopMultiplier(NoteProcPtr env, NoteProcPtr src) : AmpMultiplier(std::move(env), std::move(src)){}
	u_sample getAmp(Note & note)override{
		return a->getAmp(note) * b->getAmp(note);
	}
	bool noMoreData(Note & note)override{
		bool nmd=a->noMoreData(note);
		if(nmd){
			b->noMoreData(note);
			note.requestClose(*note.cfg);
		}
		return nmd;
	}
	NoteProcPtr clone()override{
		return mksp<EnvelopMultiplier>(a->clone(), b->clone());
	}
	yzrilyzr_lang::String toString()const override{
		return yzrilyzr_lang::StringFormat::object2string("EnvelopMultiplier", a, b);
	}
	u_sample postProcess(u_sample output)override{
		return b->postProcess(output);
	}
	};
}