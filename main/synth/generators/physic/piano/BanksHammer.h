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
	double a=0;
	double mi=0;
	double K=0;
	double p=0;
	double sampleRate=0;
	double F=0;
	double Z2i=0;
	double vh=0;
	double oldvin=0;
	void init(double sampleRate, double m, double K, double p, double Z, double alpha) override;
	double load(double vin) override;
	void trigger(double v) override;
	std::string toString()const override;
	};
}
