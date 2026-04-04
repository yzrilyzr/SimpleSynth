#pragma once
#include "SimpleSynth.h"
#include "array/Array.hpp"
#include "events/NoteData.hpp"
#include "synth/modulation/NoteModulation.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(ArpeggiatorModKeyData){
	public:
	s_phase phase;
	};
	ECLASS(ArpeggiatorMod, public NoteModulation,NoteData<ArpeggiatorModKeyData>){
	public:
	yzrilyzr_array::IntArray offsets;
	u_freq rate;
	public:
	ArpeggiatorMod(NoteProcPtr a, yzrilyzr_array::IntArray offsets, u_freq rate);
	ArpeggiatorMod();
	void init(ChannelConfig & cfg)override;
	void applyMod(Note & note)override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	void onRegisterParam()override;
	ArpeggiatorModKeyData * init(ArpeggiatorModKeyData * data, const Note & note) override;
	U_CLASS_INFO(ArpeggiatorMod);
	};
}
