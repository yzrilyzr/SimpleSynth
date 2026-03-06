#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"

namespace yzrilyzr_simplesynth{
	ECLASS(AmpBinaryComposition, public NoteProcessor){
	public:
	NoteProcPtr a=nullptr;
	NoteProcPtr b=nullptr;
	AmpBinaryComposition(NoteProcPtr a1, NoteProcPtr b1);
	yzrilyzr_lang::String toString() const override;
	void init(ChannelConfig & cfg) override;
	void cc(ChannelConfig & cfg, ChannelControl & cc) override;
	void onRegisterParam() override;
	U_GET_CLASS_NAME(AmpBinaryComposition);
	};
}