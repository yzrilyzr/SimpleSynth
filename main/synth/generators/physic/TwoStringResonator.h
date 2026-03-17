#pragma once
#include "synth/generators/Osc.h"
#include "interface/NoteProcessor.h"
#include "synth/envelopers/AHDSREnvelop.h"
#include "SimpleSynth.h"
#include "TwoStringResonatorExciter.h"
#include "SynthUtil.h"
#include "dsp/RingBuffer.h"
#include "dsp/IIR.h"
#include "dsp/BiquadIIR.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(TwoStringResonatorKeyData){
	public:
	yzrilyzr_dsp::BiquadIIR hiCut;
	yzrilyzr_dsp::BiquadIIR lowCut;
	yzrilyzr_dsp::RingBufferSample string1;
	yzrilyzr_dsp::RingBufferSample string2;
	yzrilyzr_dsp::RingBufferSample comb1;
	yzrilyzr_dsp::RingBufferSample comb2;
	};
	ECLASS(TwoStringResonator, public Osc, NoteData<TwoStringResonatorKeyData>){
	public:
	//exciter
	NoteProcPtr exciter;
	u_freq exciterHiCutFreq=1.0f;
	u_sample exciterHiCutQ=1.0f;
	u_freq exciterLowCutFreq=0.01f;
	u_sample exciterLowCutQ=1.0f;
	AHDSREnvelop exciterHiCutEnv;
	//string
	u_normal_11 stringVFeedback1=1.0f;
	u_normal_11 stringVFeedback2=1.0f;
	u_normal_01 stringVAlpha1=1.0f;
	u_normal_01 stringVAlpha2=1.0f;
	u_sample stringFMul1=1.0f;
	u_sample stringFMul2=1.0f;
	u_normal_11 stringMix=0.0f;
	u_sample stringOutputLevel=1.0f;
	AHDSREnvelop stringEnv;
	//comb
	u_normal_01 combPosition=1;
	u_normal_11 combFeedback1=1;
	u_normal_11 combFeedback2=-1;
	u_sample combOutputLevel=1;
	//resonator
	u_normal_11 resonatorFeedback[8]={0};
	u_freq resonatorFreq[8]={1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0};
	bool resonatorEnabled[8]={false};
	u_sample resonatorOutputLevel=1;
	//
	yzrilyzr_array::Array<yzrilyzr_dsp::RingBufferSample*> resonators;
	u_sample sampleRate=0;
	TwoStringResonator();
	~TwoStringResonator();
	void init(ChannelConfig & cfg) override;
	u_sample postProcess(u_sample output) override;
	NoteProcPtr clone() override;
	u_sample getAmp(const Note & note) override;
	bool noMoreData(const Note & note)override;
	TwoStringResonatorKeyData * init(TwoStringResonatorKeyData * buffer, const Note & note) override;
	//static u_sample exciteClickFunc(s_phase mod);
	u_sample procString(yzrilyzr_dsp::RingBufferSample & string, yzrilyzr_dsp::RingBufferSample & comb, const Note & note, u_sample input, u_sample sampleRate, u_sample vAlpha, u_sample vFeedback, u_sample fMul, u_sample combFeedback);
	void onRegisterParam() override;
	U_CLASS_INFO(TwoStringResonator)
	};
	EBCLASS(TwoStringResonatorBuilder){
	private:
	u_sp<TwoStringResonator> twostringresonator;
	public:
	TwoStringResonatorBuilder(){
		twostringresonator=mksp<TwoStringResonator>();
	}
	TwoStringResonatorBuilder & exciter(NoteProcPtr paramRegPtr){
		twostringresonator->exciter=paramRegPtr;
		return *this;
	}
	TwoStringResonatorBuilder & exciter(u_sample noiseMix, u_sample noiseRate){
		auto ex=mksp<TwoStringResonatorExciter>();
		ex->noiseMixRatio=noiseMix;
		ex->noiseRate=noiseRate;
		exciter(ex);
		return *this;
	}
	TwoStringResonatorBuilder & exciterHiCut(u_sample freq, u_sample q){
		twostringresonator->exciterHiCutFreq=freq;
		twostringresonator->exciterHiCutQ=q;
		return *this;
	}
	TwoStringResonatorBuilder & exciterHiCutEnv(u_time_ms aTime, u_time_ms hTime, u_time_ms dTime, u_sample sLevel){
		twostringresonator->exciterHiCutEnv.attackTime=aTime / 1000.0;
		twostringresonator->exciterHiCutEnv.holdTime=hTime / 1000.0;
		twostringresonator->exciterHiCutEnv.decayTime=dTime / 1000.0;
		twostringresonator->exciterHiCutEnv.sustainVolume=sLevel;
		return *this;
	}
	TwoStringResonatorBuilder & exciterLowCut(u_freq freq, u_sample q){
		twostringresonator->exciterLowCutFreq=freq;
		twostringresonator->exciterLowCutQ=q;
		return *this;
	}
	TwoStringResonatorBuilder & string1(u_sample freqMul, u_sample feedback, u_sample alpha){
		twostringresonator->stringFMul1=freqMul;
		twostringresonator->stringVFeedback1=feedback;
		twostringresonator->stringVAlpha1=alpha;
		return *this;
	}
	TwoStringResonatorBuilder & string2(u_sample freqMul, u_sample feedback, u_sample alpha){
		twostringresonator->stringFMul2=freqMul;
		twostringresonator->stringVFeedback2=feedback;
		twostringresonator->stringVAlpha2=alpha;
		return *this;
	}
	TwoStringResonatorBuilder & stringMix(u_sample mix){
		twostringresonator->stringMix=mix;
		return *this;
	}
	TwoStringResonatorBuilder & stringEnv(u_time_ms aTime, u_time_ms hTime, u_time_ms dTime, bool sustainable, u_normal_01 sLevel, u_time_ms rTime){
		twostringresonator->stringEnv.attackTime=aTime / 1000.0;
		twostringresonator->stringEnv.holdTime=hTime / 1000.0;
		twostringresonator->stringEnv.decayTime=dTime / 1000.0;
		twostringresonator->stringEnv.canSustain=sustainable;
		twostringresonator->stringEnv.sustainVolume=sLevel;
		twostringresonator->stringEnv.releaseTime=rTime / 1000.0;
		return *this;
	}
	TwoStringResonatorBuilder & stringLevel(u_sample mul){
		twostringresonator->stringOutputLevel=mul;
		return *this;
	}
	TwoStringResonatorBuilder & comb(u_sample pos, u_sample feedback1, u_sample feedback2, u_sample outputLevel){
		twostringresonator->combPosition=pos;
		twostringresonator->combFeedback1=feedback1;
		twostringresonator->combFeedback2=feedback2;
		twostringresonator->combOutputLevel=outputLevel;
		return *this;
	}
	TwoStringResonatorBuilder & resonator(int which, u_sample freq, u_sample feedback){
		twostringresonator->resonatorEnabled[which]=true;
		twostringresonator->resonatorFreq[which]=freq;
		twostringresonator->resonatorFeedback[which]=feedback;
		return *this;
	}
	TwoStringResonatorBuilder & resonatorLevel(u_sample mul){
		twostringresonator->resonatorOutputLevel=mul;
		return *this;
	}
	NoteProcPtr build(){
		return spsc<NoteProcessor>(twostringresonator);
	}
	};
}