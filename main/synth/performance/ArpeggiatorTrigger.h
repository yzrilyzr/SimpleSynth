#pragma once
#include "SimpleSynth.h"
#include "array/Array.hpp"
#include "events/NoteData.hpp"
#include "synth/composed/AmpUnaryComposition.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(ArpeggiatorTriggerKeyData){
	public:
	u_index lastTrigger=-1;
	yzrilyzr_array::Array<Note> notes;
	bool uniqueIDSet=false;
	};
	ECLASS(ArpeggiatorTrigger, public AmpUnaryComposition, NoteData<ArpeggiatorTriggerKeyData>){
	private:
	std::atomic<int> uniqueID{0};
	public:
	yzrilyzr_array::IntArray offsets;
	u_freq rate;
	public:
	ArpeggiatorTrigger(NoteProcPtr a, yzrilyzr_array::IntArray offsets, u_freq rate);
	ArpeggiatorTrigger();
	void init(ChannelConfig & cfg)override;
	u_sample getAmp(const Note & note) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	bool noMoreData(const Note & note) const override;
	void onRegisterParam()override;
	ArpeggiatorTriggerKeyData * init(ArpeggiatorTriggerKeyData * data, const Note & note) override;
	U_CLASS_INFO(ArpeggiatorTrigger);
	};
}
