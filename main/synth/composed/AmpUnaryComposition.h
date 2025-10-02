#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"

namespace yzrilyzr_simplesynth{
	ECLASS(AmpUnaryComposition, public NoteProcessor){
	protected:
	NoteProcPtr a=nullptr;
	public:
	AmpUnaryComposition(NoteProcPtr a1);
	yzrilyzr_lang::String toString() const override;
	void init(ChannelConfig & cfg) override;
	inline u_sample postProcess(u_sample output) override{
		return a->postProcess(output);
	}
	inline bool noMoreData(Note & note) override{
		return a->noMoreData(note);
	}
	void cc(ChannelConfig & cfg, ChannelControl & cc)override;
	};
}