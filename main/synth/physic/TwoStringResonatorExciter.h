#pragma once
#include "SimpleSynth.h"
#include "SynthUtil.h"
#include "interface/NoteProcessor.h"
#include "events/NoteData.hpp"

namespace yzrilyzr_simplesynth{
	EBCLASS(TwoStringResonatorExciterKeyData){
	public:
	double noiseRateCounter=0;
	u_sample lastNoiseValue=0;
	};
	ECLASS(TwoStringResonatorExciter, public NoteProcessor, NoteData<TwoStringResonatorExciterKeyData>){
	public:
	float noiseMixRatio=0.0f;
	float noiseRate=1.0f;
	TwoStringResonatorExciter();
	~TwoStringResonatorExciter(){}
	void init(ChannelConfig & cfg) override;
	NoteProcPtr clone() override;
	u_sample getAmp(const Note & note) override;
	TwoStringResonatorExciterKeyData * init(TwoStringResonatorExciterKeyData * data, const Note & note) override;
	static u_sample exciteClickFunc(s_phase mod);
	void onRegisterParam() override;
	U_CLASS_INFO(TwoStringResonatorExciter)

	};
}