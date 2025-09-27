#pragma once
#include "synth/generators/Osc.h"
#include "interface/NoteProcessor.h"
#include "synth/envelopers/AHDSREnvelop.h"
#include "SimpleSynth.h"
#include "SynthUtil.h"
#include "dsp/RingBuffer.h"
#include "dsp/IIR.h"
#include "dsp/BiquadIIR.h"


namespace yzrilyzr_simplesynth{
	EBCLASS(SakuraExciterKeyData){
	public:
	double noiseRateCounter=0;
	u_sample lastNoiseValue=0;
	};
	ECLASS(SakuraExciter, public Osc, NoteData<SakuraExciterKeyData>){
	public:
	float noiseMixRatio=0.0f;
	float noiseRate=1.0f;
	FixedRandom random;
	SakuraExciter();
	~SakuraExciter(){}
	void init(ChannelConfig & cfg) override;
	//u_sample postProcess(u_sample output) override;
	NoteProcPtr clone() override;
	u_sample getAmp(Note & note) override;
	//bool noMoreData(Note & note)override;
	SakuraExciterKeyData * init(SakuraExciterKeyData * buffer, Note & note) override;
	static u_sample exciteClickFunc(s_phase mod);

	};
}