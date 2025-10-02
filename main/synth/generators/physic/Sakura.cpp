#include "events/Note.h"
#include "interface/NoteTuning.h"
#include "Sakura.h"
#include "SakuraExciter.h"
#include "synth/envelopers/EnvUtil.h"
#include "dsp/BiquadIIR.h"
#include "dsp/BufferDelayer.h"
#include "dsp/IIRUtil.h"
#include "util/Util.h"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
namespace yzrilyzr_simplesynth{
	Sakura::Sakura() :Osc(nullptr){
		registerParam("Exciter", ParamType::NoteSrc, &exciter, nullptr, nullptr);
		for(u_index i=0;i < 8;i++){
			resonators[i]=new RingBufferSample(1024);
		}
		exciterHiCutEnv=new AHDSREnvelop(0, 100, 100, 100, 0.5, true, 100, 100, Pow(-5), Pow(5), Pow(5));
		stringEnv=new AHDSREnvelop(0, 100, 100, 100, 0.5, true, 100, 100, Pow(-5), Pow(5), Pow(5));
	}
	Sakura::~Sakura(){
		for(u_index i=0;i < 8;i++){
			delete resonators[i];
		}
		delete exciterHiCutEnv;
		delete stringEnv;
	}
	void Sakura::init(ChannelConfig & cfg){
		this->sampleRate=cfg.sampleRate;
		if(exciter == nullptr)exciter=std::make_shared<SakuraExciter>();
		if(auto exciter1=std::dynamic_pointer_cast<SakuraExciter>(exciter)){
			exciter1->noiseMixRatio=noiseMixRatio;
			exciter1->noiseRate=noiseRate;
		}
		exciter->init(cfg);
	}
	u_sample Sakura::postProcess(u_sample output){
		u_sample sum=output;
		for(u_index i=0;i < 8;i++){
			bool enabled=resonatorEnabled[i];
			if(!enabled)continue;
			RingBufferSample & re=*resonators[i];
			u_freq freq=resonatorFreq[i];
			double fdbk=resonatorFeedback[i];
			double delayIndex=RingBufferUtil::freq2delayIndex(freq, sampleRate);
			re.ensureCapacity(delayIndex);
			u_sample delayed=BufferDelayer::cubicSplineDelay(re, delayIndex);
			re.write(output + delayed * RingBufferUtil::feedbackCoeff(fdbk, freq));
			sum+=delayed * static_cast<u_sample>(resonatorOutputLevel);
		}
		return sum;
	}
	NoteProcPtr Sakura::clone(){
		return nullptr;
	}
	u_sample Sakura::getAmp(Note & note){
		SakuraKeyData & data=*getData(note);
		u_sample sumExcite=exciter->getAmp(note);
		u_freq hcFreq=note.cfg->tuning->getFrequencyByID(127.0 * exciterHiCutFreq * exciterHiCutEnv->getAmp(note));
		u_freq lcFreq=note.cfg->tuning->getFrequencyByID(127.0 * exciterLowCutFreq);
		IIRUtil::biquad(data.hiCut, IIRUtil::limitFreq(sampleRate, hcFreq), sampleRate, exciterHiCutQ, FilterPassType::LOWPASS);
		IIRUtil::biquad(data.lowCut, IIRUtil::limitFreq(sampleRate, lcFreq), sampleRate, exciterLowCutQ, FilterPassType::HIGHPASS);
		sumExcite=data.hiCut.procDsp(sumExcite);
		sumExcite=data.lowCut.procDsp(sumExcite);
		if(isnan(sumExcite)){
			data.hiCut.resetMemory();
			data.lowCut.resetMemory();
		}
		u_sample sumString=0;
		u_sample str1Out=procString(data.string1, data.comb1, note, sumExcite, sampleRate, stringVAlpha1, stringVFeedback1, stringFMul1, combFeedback1);
		u_sample str2Out=procString(data.string2, data.comb2, note, sumExcite, sampleRate, stringVAlpha2, stringVFeedback2, stringFMul2, combFeedback2);
		sumString+=str1Out * (-(stringMix - 1.0) / 2.0);
		sumString+=str2Out * ((stringMix + 1.0) / 2.0);
		sumString*=stringEnv->getAmp(note) * static_cast<u_sample>(stringOutputLevel);
		return sumString;
	}
	u_sample Sakura::procString(RingBufferSample & stringBuf, RingBufferSample & combBuf, Note & note, u_sample input, double sampleRate, double vAlpha, double vFeedback, double fMul, double combFeedback){
		u_freq noteFreq=note.freqSynth * fMul;
		u_freq combFreq=noteFreq / combPosition;
		double delayIndex1=RingBufferUtil::freq2delayIndex(noteFreq, sampleRate);
		double delayIndexComb=RingBufferUtil::freq2delayIndex(combFreq, sampleRate);
		stringBuf.ensureCapacity(delayIndex1);
		combBuf.ensureCapacity(delayIndexComb);
		u_sample delayed=BufferDelayer::cubicSplineDelay(stringBuf, delayIndex1) * RingBufferUtil::feedbackCoeff(vFeedback, noteFreq);
		u_sample delayedComb=BufferDelayer::cubicSplineDelay(combBuf, delayIndexComb) * RingBufferUtil::feedbackCoeff(combFeedback, combFreq);
		double alpha2=pow(vAlpha, Util::clamp(delayIndex1, 0.0, 800.0) / 367.0);
		delayed=stringBuf.newest() * (1 - alpha2) + delayed * alpha2;
		stringBuf.write(input + delayed);
		combBuf.write(input + delayedComb);
		return delayed + delayedComb * static_cast<u_sample>(combOutputLevel);
	}

	SakuraKeyData * Sakura::init(SakuraKeyData * data, Note & note){
		if(data == nullptr){
			data=new SakuraKeyData();
		}
		data->string1.fill(0);
		data->string2.fill(0);
		data->comb1.fill(0);
		data->comb2.fill(0);
		data->hiCut.resetMemory();
		data->lowCut.resetMemory();
		return data;
	}
	bool Sakura::noMoreData(Note & note){
		if(exciter != nullptr)exciter->noMoreData(note);
		return stringEnv->noMoreData(note);
	}
}