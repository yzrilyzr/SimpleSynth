#pragma once
#include "interface/NoteProcessor.h"
#include "events/Note.h"
#include "array/Array.hpp"

namespace yzrilyzr_simplesynth{
	ECLASS(SineWaveTable, public NoteProcessor){
	private:
	yzrilyzr_array::DoubleArray aa;
	double baseFreq;
	public:
	~SineWaveTable();
	SineWaveTable()=default;
	SineWaveTable(double baseFreq, const yzrilyzr_array::DoubleArray & freqAndAmp);
	u_sample getAmp(const Note & note) override;
	private:
	u_sample a(double x, u_freq notef);
	};
}