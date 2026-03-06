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
	yzrilyzr_array::Array<Note> notes;
	bool uniqueIDSet=false;
	};
	ECLASS(SimpleDetuner, public AmpUnaryComposition, NoteData<SimpleDetunerKeyData>){
	private:
	std::atomic<int> uniqueID{0};
	public:
	int32_t count=1;
	s_note_id offset=0;
	u_normal_01 initPhase=1;

	~SimpleDetuner()=default;
	SimpleDetuner() :AmpUnaryComposition(nullptr){}
	SimpleDetuner(NoteProcPtr a, int32_t count, s_note_id offset) :
		AmpUnaryComposition(a),
		count(count),
		offset(offset){}
	u_sample getAmp(const Note & note) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	void init(ChannelConfig &cfg) override;
	SimpleDetunerKeyData * init(SimpleDetunerKeyData * data, const Note & note) override;
	void onRegisterParam() override;
	U_GET_CLASS_NAME(SimpleDetuner)
	};
}