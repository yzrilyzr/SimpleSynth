#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "AmpUnaryComposition.h"
#include "events/NoteData.hpp"
#include "array/Array.hpp"
#include "array/Array.hpp"

namespace yzrilyzr_simplesynth{
	EBCLASS(MultiKeyTriggerKeyData){
	public:
	Note ** notes=nullptr;
	u_index size=0;
	~MultiKeyTriggerKeyData(){
		for(u_index i=0;i < size;i++){
			delete notes[i];
		}
		delete[] notes;
	}
	};
	ECLASS(MultiKeyTrigger, public AmpUnaryComposition, NoteData<MultiKeyTriggerKeyData>){
	private:
	u_index notesCount=0;
	yzrilyzr_array::IntArray idShift=nullptr;
	yzrilyzr_array::DoubleArray velocityMul=nullptr;
	std::atomic<int> uniqueID{0};
	public:
	~MultiKeyTrigger();
	MultiKeyTrigger();
	MultiKeyTrigger(NoteProcPtr a, const yzrilyzr_array::IntArray & noteShift, const yzrilyzr_array::DoubleArray & velocityMul);
	u_sample getAmp(Note & note) override;
	NoteProcPtr clone() override;
	MultiKeyTriggerKeyData * init(MultiKeyTriggerKeyData * data, Note & note) override;
	yzrilyzr_lang::String toString() const override;
	};
}