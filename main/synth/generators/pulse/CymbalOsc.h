#pragma once
#include "interface/NoteProcessor.h"
#include "synth/generators/Osc.h"
#include "interface/PhaseSrc.h"
#include "SynthUtil.h"

namespace yzrilyzr_simplesynth{
	ECLASS(CymbalOsc, public Osc){
	private:
	FixedRandom random;
	NoteProcPtr mul=nullptr;
	public:
	~CymbalOsc()=default;
	CymbalOsc();
	CymbalOsc(double mul);
	CymbalOsc(NoteProcPtr mul);
	CymbalOsc(u_sp<PhaseSrc> freqSrc, NoteProcPtr mul);
	void init(ChannelConfig & cfg) override;
	u_sample getAmp(Note & note) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	private:
	static u_sample square(u_freq Hz, u_time time);
	};
}