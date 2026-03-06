#pragma once
#include "synth/generators/Osc.h"
#include "interface/NoteProcessor.h"
#include "SimpleSynth.h"
#include "SynthUtil.h"
#include "dsp/RingBuffer.h"
#include "dsp/IIR.h"
#include "array/Array.hpp"

namespace yzrilyzr_simplesynth{
	ECLASS(Sitar, public Osc, NoteData<yzrilyzr_dsp::RingBufferSample>){
	private:
	FixedRandom random;
	yzrilyzr_dsp::RingBufferSample * resonanceStrings=nullptr;
	yzrilyzr_dsp::RingBufferSample * resonanceStringDelays=nullptr;
	int resonanceStringsCount=13;
	u_sp<yzrilyzr_array::DoubleArray> resonanceStringFreq;
	u_sp<yzrilyzr_dsp::IIR> boxFilter=nullptr;
	u_sp<yzrilyzr_dsp::IIR> filter=nullptr;
	public:
	~Sitar();
	Sitar();
	void init(ChannelConfig & cfg) override;
	u_sample postProcess(u_sample output) override;
	NoteProcPtr clone() override;
	u_sample getAmp(const Note & note) override;
	yzrilyzr_dsp::RingBufferSample * init(yzrilyzr_dsp::RingBufferSample * buffer, const Note & note) override;
	private:
	u_freq getSetFreq(const Note & note);
	u_sample procKS(yzrilyzr_dsp::RingBufferSample & buffer, u_sample alpha, u_sample feedback, u_sample input, u_sample delayLen);
	void initBuffer(yzrilyzr_dsp::RingBufferSample & buffer, const Note & note);
	void initBurstRandom(yzrilyzr_dsp::RingBufferSample & buffer, const Note & note, u_sample len1);
	};
}