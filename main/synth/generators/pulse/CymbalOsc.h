#pragma once
#include "interface/NoteProcessor.h"
#include "synth/generators/Osc.h"
#include "interface/PhaseSrc.h"
#include "SynthUtil.h"

namespace yzrilyzr_simplesynth{
	ECLASS(CymbalOsc, public Osc){
	private:
	FixedRandom random;
	NoteProcPtr mul;
	public:
	~CymbalOsc()=default;
	CymbalOsc();
	CymbalOsc(double mul);
	CymbalOsc(NoteProcPtr mul);
	CymbalOsc(std::shared_ptr<PhaseSrc> freqSrc, NoteProcPtr mul);
	u_sample getAmp(Note & note) override;
	NoteProcPtr clone() override;
	std::string toString() const override;
	private:
	static u_sample square(u_freq Hz, u_time time);
	};
}