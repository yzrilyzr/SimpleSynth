#pragma once
#include "SimpleSynth.h"
#include "SynthUtil.h"
#include "dsp/IIR.h"
#include "dsp/RingBuffer.h"
#include "interface/NoteProcessor.h"
#include "events/NoteData.hpp"

namespace yzrilyzr_simplesynth{
	EBCLASS(BowedStringKeyData){
	public:
	yzrilyzr_dsp::RingBufferSample ringBuffer;
	u_sp<yzrilyzr_dsp::IIR> filter=nullptr;
	};
	ECLASS(BowedString, public NoteProcessor), NoteData<BowedStringKeyData>{
	private:
	yzrilyzr_dsp::DSPPtr boxReverb;
	yzrilyzr_dsp::IIR boxLowBand;
	yzrilyzr_dsp::IIR boxNotch;
	u_freq boxCombFreq;
	u_freq boxBandFreq;
	u_freq boxNotchFreq;
	public:
	~BowedString();
	BowedString() :BowedString(100, 100, 100){}
	BowedString(u_freq boxCombFreq, u_freq boxBandFreq, u_freq boxNotchFreq) :
		boxCombFreq(boxCombFreq),
		boxBandFreq(boxBandFreq),
		boxNotchFreq(boxNotchFreq){}
	void init(ChannelConfig & cfg) override;
	void postProcess(u_sample * input, u_index length) override;
	NoteProcPtr clone() override;
	u_sample getAmp(const Note & note) override;
	BowedStringKeyData * init(BowedStringKeyData * data, const Note & note) override;
	private:
	u_sample procKS(u_sample_rate sampleRate, yzrilyzr_dsp::RingBufferSample & buffer, u_sample alpha, u_sample feedback, u_sample input, u_freq freq2);
	u_freq getSetFreq(const Note & note);
	void onRegisterParam() override;
	};
}