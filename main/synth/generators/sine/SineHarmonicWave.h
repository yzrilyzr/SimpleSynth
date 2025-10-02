#pragma once
#include "synth/generators/Osc.h"
#include "events/Note.h"
#include "array/Array.hpp"


namespace yzrilyzr_simplesynth{
	/**
	* n次正弦谐波
	*/
	ECLASS(SineHarmonicWave, public Osc){
	private:
	yzrilyzr_array::DoubleArray *aa;
	public:
		~SineHarmonicWave();
		SineHarmonicWave(yzrilyzr_array::DoubleArray *freqAndAmp);
		SineHarmonicWave(std::shared_ptr<PhaseSrc> freq, yzrilyzr_array::DoubleArray *freqAndAmp);
		u_sample getAmp(Note &note) override;
	private:
		u_sample a(double x);
	};
}