#include "BowedString.h"
#include "SynthUtil.h"
#include "dsp/AllPassFilter.h"
#include "dsp/FilterPassType.h"
#include "dsp/IIRUtil.h"

using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
namespace yzrilyzr_simplesynth{
	BowedString::~BowedString(){
		delete random;
		delete boxReverb;
	}
	void BowedString::init(ChannelConfig & cfg){
		Osc::init(cfg);
		this->sampleRate=cfg.sampleRate;
		boxReverb=new AllPassFilter(3, 0.5);
		boxReverb->init(cfg.sampleRate);
		IIRUtil::biquad(boxLowBand, boxBandFreq, sampleRate, 0.7, FilterPassType::BANDPASS);
		IIRUtil::biquad(boxNotch, boxNotchFreq, sampleRate, 10, FilterPassType::NOTCH);
	}
	u_sample BowedString::postProcess(u_sample output){
		u_sample string1=Osc::postProcess(output);
		u_sample sum1=boxReverb->procDsp(string1) * 0.5;
		sum1=boxLowBand.procDsp(sum1) * 3.0;
		sum1=boxNotch.procDsp(sum1);
		return sum1 + string1 * 0.8;
	}
	NoteProcPtr BowedString::clone(){
		return std::make_shared<BowedString>(boxCombFreq, boxBandFreq, boxNotchFreq);
	}
	u_sample BowedString::getAmp(Note & note){
		BowedStringKeyData & data=*getData(note);
		RingBufferSample & buffer=data.ringBuffer;
		u_freq freq2=getSetFreq(note);
		static thread_local u_index randomIndex=0;
		u_sample input=random->next(&randomIndex) * 0.1;
		s_phase time=getPhase(note);//. % 1;
		time=time - (int)time;
		input+=(time * 2 - 1) * 0.4;
		if(time > 0.3) input=0;
		input*=note.velocitySynth;
		input=data.filter->procDsp(input);
		double alpha=0.4 + 0.4 * note.velocitySynth;
		return procKS(buffer, alpha, 0.95, input, freq2) * 0.2;
	}
	BowedStringKeyData * BowedString::init(BowedStringKeyData * data, Note & note){
		if(data == nullptr){
			data=new BowedStringKeyData();
			data->filter=IIRUtil::newButterworthIIRFilter(sampleRate, FilterPassType::BANDPASS, 1, 20, 5000);
		}
		data->filter->resetMemory();
		data->filter->init(note.cfg->sampleRate);
		data->ringBuffer.fill(0);
		return data;
	}
	u_sample BowedString::procKS(RingBufferSample & buffer, double alpha, double feedback, u_sample input, u_freq freq2){
		double len=sampleRate / freq2;
		buffer.ensureCapacity(len);
		u_sample delayed=BufferDelayer::cubicSplineDelay(buffer, len - 1);
		u_sample newest=buffer.newest();
		double alpha2=pow(alpha, len / 367.0);
		u_sample sum=newest * (1 - alpha2) + delayed * alpha2;
		sum+=input;
		buffer.write(sum * feedback);
		return sum;
	}
	u_freq BowedString::getSetFreq(Note & note){
		return note.freqSynth;
	}
}