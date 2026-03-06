#pragma once
#include "SimpleSynth.h"

#include "interface/NoteProcessor.h"
#include "synth/composed/AmpUnaryComposition.h"
#include "events/NoteData.hpp"

namespace yzrilyzr_simplesynth{
	EBCLASS(NoteShiftKeyData){
	public:
	s_phase freqTimeSynth=0;
	};
	ECLASS(NoteShift, public AmpUnaryComposition), NoteData<NoteShiftKeyData>{
	private:
	int32_t shift;
	public:
	NoteShift();
	NoteShift(NoteProcPtr a, int shift);
	u_sample getAmp(const Note & note) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override{
		return yzrilyzr_lang::StringFormat::format("NoteShift(%s,%d)", a, shift);
	}
	NoteShiftKeyData * init(NoteShiftKeyData * data, const Note & note) override;
	};
}