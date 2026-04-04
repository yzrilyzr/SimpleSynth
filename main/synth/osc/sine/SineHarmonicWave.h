#pragma once
#include "interface/NoteProcessor.h"
#include "events/Note.h"
#include "array/Array.hpp"


namespace yzrilyzr_simplesynth{
	/**
	* n次正弦谐波
	*/
	ECLASS(SineHarmonicWave, public NoteProcessor){
	private:
	yzrilyzr_array::DoubleArray aa;
	public:
	~SineHarmonicWave();
	SineHarmonicWave(const yzrilyzr_array::DoubleArray& freqAndAmp);
	u_sample getAmp(const Note & note) override;
	private:
	u_sample a(double x);
	};
}