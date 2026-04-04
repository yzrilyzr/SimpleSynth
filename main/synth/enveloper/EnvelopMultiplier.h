#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "synth/operator/AmpMultiplier.h"
#include "Enveloper.h"
#include "events/Note.h"

namespace yzrilyzr_simplesynth{
	ECLASS(EnvelopMultiplier, public AmpMultiplier){
	public:
	EnvelopMultiplier(){}
	EnvelopMultiplier(NoteProcPtr env, NoteProcPtr src) : AmpMultiplier(std::move(env), std::move(src)){}
	void onRegisterParam()override{
		AmpMultiplier::onRegisterParam();
		registerParamAlias("B", "Src");
		registerParamAlias("A", "Env");
	}
	bool noMoreData(const Note & note)const override{
		bool nmd=a->noMoreData(note);
		if(nmd){
			b->noMoreData(note);
			const_cast<Note &>(note).requestClose(*note.cfg);
		}
		return nmd;
	}
	NoteProcPtr clone()override{
		return mksp<EnvelopMultiplier>(a->clone(), b->clone());
	}
	yzrilyzr_lang::String toString()const override{
		return yzrilyzr_lang::StringFormat::object2string("EnvelopMultiplier", a, b);
	}
	void postProcess(u_sample * input, u_index length)override{
		b->postProcess(input, length);
	}
	U_CLASS_INFO(EnvelopMultiplier)
	};
}