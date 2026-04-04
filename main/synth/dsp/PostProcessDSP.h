#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "synth/composed/AmpUnaryComposition.h"
#include "events/NoteData.hpp"
#include "dsp/DSP.h"

namespace yzrilyzr_simplesynth{
	ECLASS(PostProcessDSP, public AmpUnaryComposition){
	private:
	yzrilyzr_dsp::DSPPtr dsp=nullptr;
	public:
	~PostProcessDSP()=default;
	PostProcessDSP();
	PostProcessDSP(NoteProcPtr a, yzrilyzr_dsp::DSPPtr dsp);
	void init(ChannelConfig & cfg) override;
	u_sample getAmp(const Note & note) override;
	void postProcess(u_sample * input, u_index length) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	};
}