#pragma once
#include "SimpleSynth.h"
#include "Hammer.h"

namespace yzrilyzr_simplesynth{
	ECLASS(StulovHammer, public Hammer){
	public:
	u_sample a=0;
	u_sample mi=0;
	u_sample K=0;
	u_sample p=0;
	u_sample sampleRate=0;
	u_sample F=0;
	u_sample Z2i=0;
	u_sample upprev=0;
	u_sample alpha=0;
	u_sample x=0;
	u_sample v=0;
	int S=0;
	u_sample dt=0;
	u_sample dti=0;
	void init(u_sample sampleRate, u_sample m, u_sample K, u_sample p, u_sample Z, u_sample alpha) override;
	u_sample load(u_sample in) override;
	void trigger(u_sample v) override;
	yzrilyzr_lang::String toString()const override;
	};
}
