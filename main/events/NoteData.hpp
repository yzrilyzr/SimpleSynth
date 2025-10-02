#pragma once
#include "SimpleSynth.h"
#include "Note.h"
#include "lang/Exception.h"

namespace yzrilyzr_simplesynth{
	template<typename T>
	EBCLASS(NoteData){
	public:
	T * data[CHANNEL_MAX_VOICE]={nullptr};
	NoteData(){}
	~NoteData(){
		for(u_index i=0;i < CHANNEL_MAX_VOICE;i++){
			delete data[i];
		}
	}
	virtual T * init(T * data, Note & note)=0;
	T * getData(Note & note){
		if(note.uniqueID >= CHANNEL_MAX_VOICE)throw yzrilyzr_lang::IndexOutOfBoundsException("Note Unique ID >= 128");
		T * t=data[note.uniqueID];
		if(t == nullptr){
			t=this->init(nullptr, note);
			data[note.uniqueID]=t;
		} else if(note.dataInvalidated){
			this->init(t, note);
		}
		return t;
	}
	};
}