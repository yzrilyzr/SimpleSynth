#include "NoteModulation.h"
#include "events/Note.h"

using namespace yzrilyzr_lang;

namespace yzrilyzr_simplesynth{
	void NoteModulation::onRegisterParam(){
		AmpUnaryComposition::onRegisterParam();
	}
	void NoteModulation::applyMod(Note & note){}
	u_sample NoteModulation::getAmp(const Note & note){
		static thread_local Note myNote;
		myNote.set(note);
		myNote.uniqueID=note.uniqueID;
		applyMod(myNote);
		return a->getAmp(myNote);
	}
	NoteProcPtr NoteModulation::clone(){
		return mksp<NoteModulation>(a->clone());
	}
	String NoteModulation::toString() const{
		return "NoteModulation";
	}
}