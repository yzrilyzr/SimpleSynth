#pragma once
#include "SimpleSynth.h"
#include "Hammer.h"

namespace yzrilyzr_simplesynth{
	ECLASS(StulovHammer, public Hammer){
	public:
	double a=0;
	double mi=0;
	double K=0;
	double p=0;
	double sampleRate=0;
	double F=0;
	double Z2i=0;
	double upprev=0;
	double alpha=0;
	double x=0;
	double v=0;
	int S=0;
	double dt=0;
	double dti=0;
	void init(double sampleRate, double m, double K, double p, double Z, double alpha) override;
	double load(double in) override;
	void trigger(double v) override;
	yzrilyzr_lang::String toString()const override;
	};
}
