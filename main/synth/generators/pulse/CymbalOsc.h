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
	yzrilyzr_array::IntArray osc;
	u_normal_01 mix;
	public:
	~CymbalOsc()=default;
	CymbalOsc();
	CymbalOsc(NoteProcPtr mul, u_normal_01 mix);
	CymbalOsc(NoteProcPtr mul, u_normal_01 mix, const yzrilyzr_array::IntArray & arr);
	CymbalOsc(u_sp<PhaseSrc> freqSrc, NoteProcPtr mul, u_normal_01 mix, const yzrilyzr_array::IntArray & arr);

	void init(ChannelConfig & cfg) override;
	u_sample getAmp(const Note & note) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	void onRegisterParam() override;
	U_GET_CLASS_NAME(CymbalOsc)
	private:
	static u_sample square(u_freq Hz, u_time time);
	};
}