#pragma once
#include "SimpleSynth.h"
#include "Hammer.h"
#include "dsp/Integrator.h"
#include "dsp/UnitDelay.h"

namespace yzrilyzr_simplesynth{
	ECLASS(BanksHammer, public Hammer){
	public:
	yzrilyzr_dsp::Integrator intv;
	yzrilyzr_dsp::Integrator intvh;
	yzrilyzr_dsp::UnitDelay unitDelay;
	u_sample a=0;
	u_sample mi=0;
	u_sample K=0;
	u_sample p=0;
	u_sample sampleRate=0;
	u_sample F=0;
	u_sample Z2i=0;
	u_sample vh=0;
	u_sample oldvin=0;
	void init(u_sample sampleRate, u_sample m, u_sample K, u_sample p, u_sample Z, u_sample alpha) override;
	u_sample load(u_sample vin) override;
	void trigger(u_sample v) override;
	yzrilyzr_lang::String toString()const override;
	};
}
