#pragma once
#include "array/Array.hpp"
#include "array/Array.hpp"
#include "SimpleSynth.h"
#include "events/Note.h"
#include "events/NoteData.hpp"
#include "interface/NoteProcessor.h"
#include "AmpUnaryComposition.h"
#include <atomic>

namespace yzrilyzr_simplesynth{
	EBCLASS(SimpleDetunerKeyData){
	public:
	yzrilyzr_array::Array<Note *> * notes=nullptr;
	};
	ECLASS(SimpleDetuner, public AmpUnaryComposition, NoteData<SimpleDetunerKeyData>){
	private:
	std::atomic<int> uniqueID{0};
	public:
	int32_t count=1;
	s_note_id offset=0;

	~SimpleDetuner()=default;
	SimpleDetuner() :AmpUnaryComposition(nullptr){
		static int minCount=1, maxCount=7;
		registerParam("Count", yzrilyzr_util::ParamType::Int, &count, &minCount, &maxCount);
		registerParamNormal01("Offset", &offset);
	}
	SimpleDetuner(NoteProcPtr a, int32_t count, s_note_id offset) :
		AmpUnaryComposition(a),
		count(count),
		offset(offset){}
	u_sample getAmp(Note & note) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	SimpleDetunerKeyData * init(SimpleDetunerKeyData * data, Note & note) override;
	};
}