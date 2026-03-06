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
using namespace yzrilyzr_array;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void Sakura::onRegisterParam(){
		Osc::onRegisterParam();
		static u_sample q_min=0.01;
		static u_sample q_max=10;
		registerParam("Exciter", ParamType::NoteSrc, &exciter, nullptr, nullptr);
		//
		RegisterUtil::registerParamFreq(*this, "ExHiCutF", &exciterHiCutFreq);
		registerParam("ExHiCutQ", ParamType::Sample, &exciterHiCutQ, &q_min, &q_max);
		//
		registerSub("ExHiCutEnv", &exciterHiCutEnv);
		//
		RegisterUtil::registerParamFreq(*this, "ExLoCutF", &exciterLowCutFreq);
		registerParam("ExLoCutQ", ParamType::Sample, &exciterLowCutQ, &q_min, &q_max);
		//
		static u_sample f_mul_min=1.0 / 5.0;
		static u_sample f_mul_max=5.0;
		registerParam("StringFMul1", ParamType::Sample, &stringFMul1, &f_mul_min, &f_mul_max);
		RegisterUtil::registerParamNormal11(*this, "StringVFeedback1", &stringVFeedback1);
		RegisterUtil::registerParamNormal01(*this, "StringVAlpha1", &stringVAlpha1);
		//
		registerParam("StringFMul2", ParamType::Sample, &stringFMul2, &f_mul_min, &f_mul_max);
		RegisterUtil::registerParamNormal11(*this, "StringVFeedback2", &stringVFeedback2);
		RegisterUtil::registerParamNormal01(*this, "StringVAlpha2", &stringVAlpha2);
		//
		RegisterUtil::registerParamNormal01(*this, "CombPosition", &combPosition);
		RegisterUtil::registerParamNormal11(*this, "CombFeedback1", &combFeedback1);
		RegisterUtil::registerParamNormal11(*this, "CombFeedback2", &combFeedback2);
		RegisterUtil::registerParamGain(*this, "CombOutputLevel", &combOutputLevel);
		//
		RegisterUtil::registerParamNormal11(*this, "StringMix", &stringMix);
		//
		registerSub("StringEnv", &stringEnv);
		//
		RegisterUtil::registerParamGain(*this, "StringOutputLevel", &stringOutputLevel);
		//
		for(u_index i=0;i < 8;i++){
			RegisterUtil::registerParamFreq(*this, String("ResonatorFreq") + i, &resonatorFreq[i]);
			RegisterUtil::registerParamNormal11(*this, String("ResonatorFeedback") + i, &resonatorFeedback[i]);
			registerParam(String("ResonatorEnable") + i, ParamType::Bool, &resonatorEnabled[i], nullptr, nullptr);
		}
		//
		RegisterUtil::registerParamGain(*this, "ResonatorOutputLevel", &resonatorOutputLevel);

	}
	Sakura::Sakura() :Osc(nullptr){
		resonators=Array<RingBufferSample*>(8);
		for(u_index i=0;i < 8;i++){
			resonators[i]=new RingBufferSample(256);
		}
		exciterHiCutEnv.delayTime=0;
		exciterHiCutEnv.attackTime=0.1;
		exciterHiCutEnv.holdTime=0.1;
		exciterHiCutEnv.decayTime=0.1;
		exciterHiCutEnv.sustainVolume=0.5;
		exciterHiCutEnv.canSustain=true;
		exciterHiCutEnv.releaseTime=0.1;
		exciterHiCutEnv.aCurve=Pow(-5);
		exciterHiCutEnv.dCurve=Pow(5);
		exciterHiCutEnv.rCurve=Pow(5);
		//
		stringEnv.delayTime=0;
		stringEnv.attackTime=0.1;
		stringEnv.holdTime=0.1;
		stringEnv.decayTime=0.1;
		stringEnv.sustainVolume=0.5;
		stringEnv.canSustain=true;
		stringEnv.releaseTime=0.1;
		stringEnv.aCurve=Pow(-5);
		stringEnv.dCurve=Pow(5);
		stringEnv.rCurve=Pow(5);
	}
	Sakura::~Sakura(){
		for(u_index i=0;i < 8;i++){
			delete resonators[i];
		}
	}
	void Sakura::init(ChannelConfig & cfg){
		this->sampleRate=cfg.sampleRate;
		if(exciter == nullptr)exciter=mksp<SakuraExciter>();
		exciter->init(cfg);
	}
	u_sample Sakura::postProcess(u_sample output){
		u_sample sum=output;
		for(u_index i=0;i < 8;i++){
			bool enabled=resonatorEnabled[i];
			if(!enabled)continue;
			RingBufferSample & re=*resonators[i];
			u_freq freq=resonatorFreq[i];
			u_sample fdbk=resonatorFeedback[i];
			u_sample delayIndex=RingBufferUtil::freq2delayIndex(freq, sampleRate);
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
	u_sample Sakura::getAmp(const Note & note){
		SakuraKeyData & data=*getData(note);
		u_sample sumExcite=exciter->getAmp(note);
		u_freq hcFreq=note.cfg->tuning->getFrequencyByID(127.0 * exciterHiCutFreq * exciterHiCutEnv.getAmp(note));
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
		sumString*=stringEnv.getAmp(note) * static_cast<u_sample>(stringOutputLevel);
		return sumString;
	}
	u_sample Sakura::procString(RingBufferSample & stringBuf, RingBufferSample & combBuf, const Note & note, u_sample input, u_sample sampleRate, u_sample vAlpha, u_sample vFeedback, u_sample fMul, u_sample combFeedback){
		u_freq noteFreq=note.freqSynth * fMul;
		u_freq combFreq=noteFreq / combPosition;
		u_sample delayIndex1=RingBufferUtil::freq2delayIndex(noteFreq, sampleRate);
		u_sample delayIndexComb=RingBufferUtil::freq2delayIndex(combFreq, sampleRate);
		stringBuf.ensureCapacity(delayIndex1);
		combBuf.ensureCapacity(delayIndexComb);
		u_sample delayed=BufferDelayer::cubicSplineDelay(stringBuf, delayIndex1) * RingBufferUtil::feedbackCoeff(vFeedback, noteFreq);
		u_sample delayedComb=BufferDelayer::cubicSplineDelay(combBuf, delayIndexComb) * RingBufferUtil::feedbackCoeff(combFeedback, combFreq);
		u_sample alpha2=pow(vAlpha, Util::clamp(delayIndex1, static_cast<u_sample>(0.0), static_cast<u_sample>(800.0)) / 367.0);
		delayed=stringBuf.newest() * (1 - alpha2) + delayed * alpha2;
		stringBuf.write(input + delayed);
		combBuf.write(input + delayedComb);
		return delayed + delayedComb * static_cast<u_sample>(combOutputLevel);
	}

	SakuraKeyData * Sakura::init(SakuraKeyData * data, const Note & note){
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
	bool Sakura::noMoreData(const Note & note){
		if(exciter != nullptr)exciter->noMoreData(note);
		return stringEnv.noMoreData(note);
	}
}