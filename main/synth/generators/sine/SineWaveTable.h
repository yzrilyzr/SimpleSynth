#pragma once
#include "synth/generators/Osc.h"
#include "events/Note.h"
#include "array/Array.hpp"

namespace yzrilyzr_simplesynth{
	ECLASS(SineWaveTable, public Osc){
	private:
	std::shared_ptr<yzrilyzr_array::DoubleArray> aa;
	double baseFreq;
	public:
	~SineWaveTable();
	SineWaveTable(double baseFreq, std::shared_ptr<yzrilyzr_array::DoubleArray> freqAndAmp);
	SineWaveTable(std::shared_ptr<PhaseSrc> freq, double baseFreq, std::shared_ptr<yzrilyzr_array::DoubleArray> freqAndAmp);
	u_sample getAmp(Note & note) override;
	private:
	u_sample a(double x, u_freq notef);
	};
}