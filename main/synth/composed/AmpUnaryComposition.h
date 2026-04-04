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
	inline void postProcess(u_sample * input, u_index length) override{
		a->postProcess(input, length);
	}
	inline bool noMoreData(const Note & note)const override{
		return a->noMoreData(note);
	}
	void cc(ChannelConfig & cfg, ChannelControl & cc)override;
	void onRegisterParam() override;
	};
}