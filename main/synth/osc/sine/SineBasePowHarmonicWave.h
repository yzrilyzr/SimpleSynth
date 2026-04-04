#pragma once
#include "interface/NoteProcessor.h"
#include "events/Note.h"
#include "array/Array.hpp"

namespace yzrilyzr_simplesynth{
	/**
	 * 指数谐波表
	 */
	ECLASS(SineBasePowHarmonicWave, public NoteProcessor){
	private:
	yzrilyzr_array::DoubleArray aa;
	public:
		~SineBasePowHarmonicWave();
		SineBasePowHarmonicWave(const yzrilyzr_array::DoubleArray & freqAndAmp);
		u_sample getAmp(const Note & note) override;
	private:
		u_sample a(double x, int id);
	};
}