#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "synth/composed/AmpUnaryComposition.h"
#include "events/Note.h"

namespace yzrilyzr_simplesynth{
	ECLASS(NoteVolBalance, public AmpUnaryComposition){
	private:
	std::vector<u_sample> val;
	public:
	NoteVolBalance(NoteProcPtr a, std::vector<u_sample> val) :AmpUnaryComposition(a), val(val){}
	u_sample getAmp(const Note & note)override{
		if(val.size() != CHANNEL_MAX_NOTE_ID)return a->getAmp(note);
		return val[note.id] * a->getAmp(note);
	}
	};
}