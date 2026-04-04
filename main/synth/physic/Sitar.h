#pragma once
#include "SimpleSynth.h"
#include "SynthUtil.h"
#include "array/Array.hpp"
#include "dsp/IIR.h"
#include "dsp/RingBuffer.h"
#include "events/NoteData.hpp"
#include "interface/NoteProcessor.h"

namespace yzrilyzr_simplesynth{
	ECLASS(Sitar, public NoteProcessor, NoteData<yzrilyzr_dsp::RingBufferSample>){
	private:
	yzrilyzr_array::Array<u_sp<yzrilyzr_dsp::RingBufferSample>> resonanceStrings;
	yzrilyzr_array::Array<u_sp<yzrilyzr_dsp::RingBufferSample>> resonanceStringDelays;
	int resonanceStringsCount=13;
	yzrilyzr_array::DoubleArray resonanceStringFreq;
	u_sp<yzrilyzr_dsp::IIR> boxFilter=nullptr;
	u_sp<yzrilyzr_dsp::IIR> filter=nullptr;
	u_sample_rate sampleRate;
	public:
	~Sitar();
	Sitar();
	void init(ChannelConfig & cfg) override;
	void postProcess(u_sample * input, u_index length) override;
	NoteProcPtr clone() override;
	u_sample getAmp(const Note & note) override;
	yzrilyzr_dsp::RingBufferSample * init(yzrilyzr_dsp::RingBufferSample * buffer, const Note & note) override;
	private:
	u_freq getSetFreq(const Note & note);
	u_sample procKS(u_sample_rate sampleRate, yzrilyzr_dsp::RingBufferSample & buffer, u_sample alpha, u_sample feedback, u_sample input, u_sample delayLen);
	void initBuffer(yzrilyzr_dsp::RingBufferSample & buffer, const Note & note);
	void initBurstRandom(yzrilyzr_dsp::RingBufferSample & buffer, const Note & note, u_sample len1);
	};
}