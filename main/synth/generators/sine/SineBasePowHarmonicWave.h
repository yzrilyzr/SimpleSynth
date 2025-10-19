#pragma once
#include "synth/generators/Osc.h"
#include "events/Note.h"
#include "array/Array.hpp"


namespace yzrilyzr_simplesynth{
	/**
	 * 指数谐波表
	 */
	ECLASS(SineBasePowHarmonicWave, public Osc){
	private:
	yzrilyzr_array::DoubleArray aa;
	public:
		~SineBasePowHarmonicWave();
		SineBasePowHarmonicWave(const yzrilyzr_array::DoubleArray & freqAndAmp);
		SineBasePowHarmonicWave(std::shared_ptr<PhaseSrc> freq, const yzrilyzr_array::DoubleArray& freqAndAmp);
		u_sample getAmp(Note &note) override;
	private:
		u_sample a(double x, int id);
	};
}