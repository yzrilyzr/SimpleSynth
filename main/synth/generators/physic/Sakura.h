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
	EBCLASS(SakuraKeyData){
	public:
	yzrilyzr_dsp::BiquadIIR hiCut;
	yzrilyzr_dsp::BiquadIIR lowCut;
	yzrilyzr_dsp::RingBufferSample string1;
	yzrilyzr_dsp::RingBufferSample string2;
	yzrilyzr_dsp::RingBufferSample comb1;
	yzrilyzr_dsp::RingBufferSample comb2;
	};
	ECLASS(Sakura, public Osc, NoteData<SakuraKeyData>){
	public:
	//exciter
	NoteProcPtr exciter;
	float noiseMixRatio=0.0f;
	float noiseRate=1.0f;
	float exciterHiCutFreq=1.0f;
	float exciterHiCutQ=1.0f;
	float exciterLowCutFreq=0.01f;
	float exciterLowCutQ=1.0f;
	AHDSREnvelop * exciterHiCutEnv=nullptr;
	//string
	float stringVFeedback1=1.0f;
	float stringVFeedback2=1.0f;
	float stringVAlpha1=1.0f;
	float stringVAlpha2=1.0f;
	float stringFMul1=1.0f;
	float stringFMul2=1.0f;
	float stringMix=0.0f;
	float stringOutputLevel=1.0f;
	AHDSREnvelop * stringEnv=nullptr;
	//comb
	float combPosition=1;
	float combFeedback1=1;
	float combFeedback2=-1;
	float combOutputLevel=1;
	//resonator
	float resonatorFeedback[8]={0};
	u_sample resonatorFreq[8]={1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0};
	bool resonatorEnabled[8]={false};
	float resonatorOutputLevel=1;
	//
	yzrilyzr_dsp::RingBufferSample * resonators[8]={nullptr};
	u_sample sampleRate=0;
	Sakura();
	~Sakura();
	void init(ChannelConfig & cfg) override;
	u_sample postProcess(u_sample output) override;
	NoteProcPtr clone() override;
	u_sample getAmp(Note & note) override;
	bool noMoreData(Note & note)override;
	SakuraKeyData * init(SakuraKeyData * buffer, Note & note) override;
	//static u_sample exciteClickFunc(s_phase mod);
	u_sample procString(yzrilyzr_dsp::RingBufferSample & string, yzrilyzr_dsp::RingBufferSample & comb, Note & note, u_sample input, u_sample sampleRate, u_sample vAlpha, u_sample vFeedback, u_sample fMul, u_sample combFeedback);
	};
	EBCLASS(SakuraBuilder){
	private:
	u_sp<Sakura> sakura;
	public:
	SakuraBuilder(){
		sakura=mksp<Sakura>();
	}
	SakuraBuilder & exciter(NoteProcPtr paramRegPtr){
		sakura->exciter=paramRegPtr;
		return *this;
	}
	SakuraBuilder & exciter(u_sample noiseMix, u_sample noiseRate){
		sakura->noiseMixRatio=noiseMix;
		sakura->noiseRate=noiseRate;
		return *this;
	}
	SakuraBuilder & exciterHiCut(u_sample freq, u_sample q){
		sakura->exciterHiCutFreq=freq;
		sakura->exciterHiCutQ=q;
		return *this;
	}
	SakuraBuilder & exciterHiCutEnv(u_time_ms aTime, u_time_ms hTime, u_time_ms dTime, u_sample sLevel){
		sakura->exciterHiCutEnv->attackTime=aTime / 1000.0;
		sakura->exciterHiCutEnv->holdTime=hTime / 1000.0;
		sakura->exciterHiCutEnv->decayTime=dTime / 1000.0;
		sakura->exciterHiCutEnv->sustainVolume=sLevel;
		return *this;
	}
	SakuraBuilder & exciterLowCut(u_freq freq, u_sample q){
		sakura->exciterLowCutFreq=freq;
		sakura->exciterLowCutQ=q;
		return *this;
	}
	SakuraBuilder & string1(u_sample freqMul, u_sample feedback, u_sample alpha){
		sakura->stringFMul1=freqMul;
		sakura->stringVFeedback1=feedback;
		sakura->stringVAlpha1=alpha;
		return *this;
	}
	SakuraBuilder & string2(u_sample freqMul, u_sample feedback, u_sample alpha){
		sakura->stringFMul2=freqMul;
		sakura->stringVFeedback2=feedback;
		sakura->stringVAlpha2=alpha;
		return *this;
	}
	SakuraBuilder & stringMix(u_sample mix){
		sakura->stringMix=mix;
		return *this;
	}
	SakuraBuilder & stringEnv(u_time_ms aTime, u_time_ms hTime, u_time_ms dTime, bool sustainable, u_normal_01 sLevel, u_time_ms rTime){
		sakura->stringEnv->attackTime=aTime / 1000.0;
		sakura->stringEnv->holdTime=hTime / 1000.0;
		sakura->stringEnv->decayTime=dTime / 1000.0;
		sakura->stringEnv->canSustain=sustainable;
		sakura->stringEnv->sustainVolume=sLevel;
		sakura->stringEnv->releaseTime=rTime / 1000.0;
		return *this;
	}
	SakuraBuilder & stringLevel(u_sample mul){
		sakura->stringOutputLevel=mul;
		return *this;
	}
	SakuraBuilder & comb(u_sample pos, u_sample feedback1, u_sample feedback2, u_sample outputLevel){
		sakura->combPosition=pos;
		sakura->combFeedback1=feedback1;
		sakura->combFeedback2=feedback2;
		sakura->combOutputLevel=outputLevel;
		return *this;
	}
	SakuraBuilder & resonator(int which, u_sample freq, u_sample feedback){
		sakura->resonatorEnabled[which]=true;
		sakura->resonatorFreq[which]=freq;
		sakura->resonatorFeedback[which]=feedback;
		return *this;
	}
	SakuraBuilder & resonatorLevel(u_sample mul){
		sakura->resonatorOutputLevel=mul;
		return *this;
	}
	NoteProcPtr build(){
		return std::dynamic_pointer_cast<NoteProcessor>(sakura);
	}
	};
}