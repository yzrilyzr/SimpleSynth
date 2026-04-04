#include "BowedString.h"
#include "SynthUtil.h"
#include "array/Array.hpp"
#include "dsp/AllPassFilter.h"
#include "dsp/BufferDelayer.h"
#include "dsp/FilterPassType.h"
#include "dsp/IIRUtil.h"
#include "synth/util/AmplitudeSources.h"
#include "util/Util.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_array;
namespace yzrilyzr_simplesynth{
	void BowedString::onRegisterParam(){
		RegisterUtil::registerParamFreq(*this, "boxCombFreq", &boxCombFreq);
		RegisterUtil::registerParamFreq(*this, "boxBandFreq", &boxBandFreq);
		RegisterUtil::registerParamFreq(*this, "boxNotchFreq", &boxNotchFreq);
	}

	BowedString::~BowedString(){}
	void BowedString::init(ChannelConfig & cfg){
		boxReverb=mksp<AllPassFilter>(3, 0.5);
		boxReverb->init(cfg.sampleRate);
		IIRUtil::RBJ_biquad(boxLowBand, RBJParams{FilterPassType::BANDPASS, boxBandFreq, cfg.sampleRate, 0.7});
		IIRUtil::RBJ_biquad(boxNotch, RBJParams{FilterPassType::NOTCH, boxNotchFreq, cfg.sampleRate, 10});
		boxLowBand.init(cfg.sampleRate);
		boxNotch.init(cfg.sampleRate);
	}
	void BowedString::postProcess(u_sample * input, u_index length){
		static thread_local SampleArray temp;
		if(temp == nullptr || temp.length < length)temp=SampleArray(length);
		u_sample * data=temp.data();
		boxReverb->procBlock(input, data, length);
		for(u_index i=0; i < length; i++){
			data[i]*=0.5;
		}
		boxLowBand.procBlock(data, data, length);
		for(u_index i=0; i < length; i++){
			data[i]*=3.0;
		}
		boxNotch.procBlock(data, data, length);
		for(u_index i=0; i < length; i++){
			input[i]=data[i] + input[i] * 0.8;
		}
	}
	NoteProcPtr BowedString::clone(){
		return mksp<BowedString>(boxCombFreq, boxBandFreq, boxNotchFreq);
	}
	u_sample BowedString::getAmp(const Note & note){
		BowedStringKeyData & data=*getData(note);
		RingBufferSample & buffer=data.ringBuffer;
		u_freq freq2=getSetFreq(note);
		static thread_local FixedRandom random;
		u_sample input=random.next() * 0.1;
		s_phase time=note.phaseSynth;//. % 1;
		time=time - (int)time;
		input+=(time * 2 - 1) * 0.4;
		if(time > 0.3) input=0;
		input*=note.velocitySynth;
		input=data.filter->procDsp(input);
		u_sample alpha=0.4 + 0.4 * note.velocitySynth;
		return procKS(note.cfg->sampleRate,buffer, alpha, 0.95, input, freq2) * 0.2;
	}
	BowedStringKeyData * BowedString::init(BowedStringKeyData * data, const Note & note){
		if(data == nullptr){
			data=new BowedStringKeyData();
			data->filter=mksp<IIR>();
			IIRUtil::RBJ_biquad(*data->filter, RBJParams{FilterPassType::BANDPASS, 200, note.cfg->sampleRate, 0.7});
		}
		data->filter->resetMemory();
		data->filter->init(note.cfg->sampleRate);
		data->ringBuffer.fill(0);
		return data;
	}
	u_sample BowedString::procKS(u_sample_rate sampleRate,RingBufferSample & buffer, u_sample alpha, u_sample feedback, u_sample input, u_freq freq2){
		u_sample len=sampleRate / freq2;
		buffer.ensureCapacity(len);
		u_sample delayed=BufferDelayer::cubicSplineDelay(buffer, len - 1);
		u_sample newest=buffer.newest();
		u_sample alpha2=pow(alpha, len / 367.0);
		u_sample sum=newest * (1 - alpha2) + delayed * alpha2;
		sum+=input;
		buffer.write(sum * feedback);
		return sum;
	}
	u_freq BowedString::getSetFreq(const Note & note){
		return note.freqSynth;
	}
}