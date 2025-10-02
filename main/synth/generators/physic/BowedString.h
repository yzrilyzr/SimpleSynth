#pragma once
#include "SimpleSynth.h"
#include "SynthUtil.h"
#include "interface/NoteProcessor.h"
#include "synth/source/AmplitudeSources.h"
#include "synth/generators/Osc.h"
#include "array/Array.hpp"
#include "dsp/RingBuffer.h"
#include "util/Util.h"
#include "dsp/BufferDelayer.h"
#include "dsp/IIR.h"
#include "dsp/BiquadIIR.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(BowedStringKeyData){
	public:
	yzrilyzr_dsp::RingBufferSample ringBuffer;
	std::shared_ptr<yzrilyzr_dsp::IIR> filter=nullptr;
	};
	ECLASS(BowedString, public Osc), NoteData<BowedStringKeyData>{
	private:
	FixedRandom * random=new FixedRandom();
	u_sample_rate sampleRate=0;
	yzrilyzr_dsp::DSP * boxReverb=nullptr;
	yzrilyzr_dsp::BiquadIIR boxLowBand;
	yzrilyzr_dsp::BiquadIIR boxNotch;
	u_freq boxCombFreq;
	u_freq boxBandFreq;
	u_freq boxNotchFreq;
	public:
	~BowedString();
	BowedString() :BowedString(100, 100, 100){
		registerParamFreq("boxCombFreq", &boxCombFreq);
		registerParamFreq("boxBandFreq", &boxBandFreq);
		registerParamFreq("boxNotchFreq", &boxNotchFreq);
	}
	BowedString(u_freq boxCombFreq, u_freq boxBandFreq, u_freq boxNotchFreq) :
		boxCombFreq(boxCombFreq),
		boxBandFreq(boxBandFreq),
		boxNotchFreq(boxNotchFreq),
		Osc(nullptr){}
	void init(ChannelConfig & cfg) override;
	u_sample postProcess(u_sample output) override;
	NoteProcPtr clone() override;
	u_sample getAmp(Note & note) override;
	BowedStringKeyData * init(BowedStringKeyData * data, Note & note) override;
	private:
	u_sample procKS(yzrilyzr_dsp::RingBufferSample & buffer, double alpha, double feedback, u_sample input, u_freq freq2);
	u_freq getSetFreq(Note & note);
	};
}