#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "AmpUnaryComposition.h"
#include "events/NoteData.hpp"
#include "dsp/DSP.h"

namespace yzrilyzr_simplesynth{
	ECLASS(PostProcessDSP, public AmpUnaryComposition){
	private:
	std::shared_ptr<yzrilyzr_dsp::DSP> dsp=nullptr;
	public:
	~PostProcessDSP()=default;
	PostProcessDSP();
	PostProcessDSP(NoteProcPtr a, yzrilyzr_dsp::DSPPtr dsp);
	void init(ChannelConfig & cfg) override;
	u_sample getAmp(Note & note) override;
	u_sample postProcess(u_sample output) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	};
}