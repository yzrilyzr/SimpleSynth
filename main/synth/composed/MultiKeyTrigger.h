#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "AmpUnaryComposition.h"
#include "events/NoteData.hpp"
#include "array/IntArray.h"
#include "array/DoubleArray.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(MultiKeyTriggerKeyData){
	public:
	Note ** notes=nullptr;
	size_t size=0;
	~MultiKeyTriggerKeyData(){
		for(size_t i=0;i < size;i++){
			delete notes[i];
		}
		delete[] notes;
	}
	};
	ECLASS(MultiKeyTrigger, public AmpUnaryComposition, NoteData<MultiKeyTriggerKeyData>){
	private:
	size_t notesCount=0;
	std::shared_ptr<yzrilyzr_array::IntArray> idShift=nullptr;
	std::shared_ptr<yzrilyzr_array::DoubleArray> velocityMul=nullptr;
	std::atomic<int> uniqueID{0};
	public:
	~MultiKeyTrigger();
	MultiKeyTrigger();
	MultiKeyTrigger(NoteProcPtr a, std::shared_ptr<yzrilyzr_array::IntArray> noteShift, std::shared_ptr<yzrilyzr_array::DoubleArray> velocityMul);
	u_sample getAmp(Note & note) override;
	NoteProcPtr clone() override;
	MultiKeyTriggerKeyData * init(MultiKeyTriggerKeyData * data, Note & note) override;
	std::string toString() const override;
	};
}