#include "Sitar.h"
#include "dsp/BufferDelayer.h"
#include "dsp/FilterPassType.h"
#include "dsp/IIRUtil.h"
#include "interface/NoteTuning.h"
#include "util/Util.h"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
using namespace yzrilyzr_array;
namespace yzrilyzr_simplesynth{
	Sitar::~Sitar(){}
	Sitar::Sitar(){}
	void Sitar::init(ChannelConfig & cfg){
		this->sampleRate = sampleRate;
		resonanceStrings=yzrilyzr_array::Array<u_sp<RingBufferSample>>(resonanceStringsCount);
		resonanceStringDelays=yzrilyzr_array::Array<u_sp<RingBufferSample>>(resonanceStringsCount);
		resonanceStringFreq=DoubleArray(resonanceStringsCount);
		int initI=Note::C4;
		IntArray ids{initI, initI + 2, initI + 4, initI + 5, initI + 7, initI + 9, initI + 11, initI + 12, initI + 14, initI + 16, initI + 17, initI + 19, initI + 21, initI + 23};
		static thread_local FixedRandom random;
		for(u_index i=0;i < resonanceStringsCount;i++){
			resonanceStrings[i]=mksp<RingBufferSample>();
			resonanceStringDelays[i]=mksp<RingBufferSample>();
			resonanceStringFreq[i]=cfg.tuning->getFrequency(ids[i] + random.next() * 0.01 / 12.0);
		}
		boxFilter=mksp<IIR>();
		filter=mksp<IIR>();
		IIRUtil::RBJ_biquad(*boxFilter, RBJParams{FilterPassType::BANDPASS, 300, cfg.sampleRate, 4});
		IIRUtil::RBJ_biquad(*filter, RBJParams{FilterPassType::BANDPASS, 400, cfg.sampleRate, 0.2});
		boxFilter->init(cfg.sampleRate);
		filter->init(cfg.sampleRate);
	}
	void Sitar::postProcess(u_sample * input, u_index length){
		for(u_index i=0;i < length;i++){
			u_sample output=input[i];
			u_sample resonSum=0;
			for(u_index i=0;i < resonanceStringsCount;i++){
				u_sample delayLen=(i + 1) * 5 / 1000.0 * 44100;
				resonanceStringDelays[i]->ensureCapacity(delayLen);
				resonanceStringDelays[i]->write(output);
				u_sample resonanceDelayOut=BufferDelayer::cubicSplineDelay(*resonanceStringDelays[i], delayLen) * 0.2;
				u_sample resonanceStringOut=procKS(sampleRate,*resonanceStrings[i], 0.99, 0.99, resonanceDelayOut, resonanceStringFreq[i]);
				resonSum+=Util::clamp(resonanceStringOut, static_cast<u_sample>(-1.0), static_cast<u_sample>(1.0));
			}
			resonSum/=resonanceStringsCount;
			u_sample out=output / 2.0 + resonSum;
			out+=boxFilter->procDsp(out) / 2;
			out=filter->procDsp(out);
			input[i]=out;
		}
	}
	NoteProcPtr Sitar::clone(){
		return mksp<Sitar>();
	}
	u_sample Sitar::getAmp(const Note & note){
		RingBufferSample & buffer=*getData(note);
		u_freq freq2=getSetFreq(note);
		u_sample len=RingBufferUtil::freq2delayIndex(freq2, note.cfg->sampleRate);
		u_sample string1=procKS(note.cfg->sampleRate, buffer, 0.75, 1, 0, len);
		string1=Util::clamp(string1 * 2, static_cast<u_sample>(-1.0), static_cast<u_sample>(1.0));
		return string1;
	}
	RingBufferSample * Sitar::init(RingBufferSample * buffer, const Note & note){
		if(buffer == nullptr) buffer=new RingBufferSample();
		initBuffer(*buffer, note);
		return buffer;
	}
	u_freq Sitar::getSetFreq(const Note & note){
		return note.freqSynth;
	}
	u_sample Sitar::procKS(u_sample_rate sampleRate, RingBufferSample & buffer, u_sample alpha, u_sample feedback, u_sample input, u_sample delayLen){
		buffer.ensureCapacity(delayLen);
		u_sample delayed=BufferDelayer::cubicSplineDelay(buffer, delayLen);
		u_sample newest=buffer.newest();
		u_sample alpha2=pow(alpha, delayLen / 367.0);
		u_sample sum=newest * (1 - alpha2) + delayed * alpha2;
		sum+=input;
		buffer.write(sum * feedback);
		return sum;
	}
	void Sitar::initBuffer(RingBufferSample & buffer, const Note & note){
		buffer.reset();
		u_freq freq=note.cfg->tuning->getFrequencyByID(note.id);
		u_sample len=RingBufferUtil::freq2delayIndex(freq, note.cfg->sampleRate);
		buffer.ensureCapacity(len);
		initBurstRandom(buffer, note, len);
	}
	void Sitar::initBurstRandom(RingBufferSample & buffer, const Note & note, u_sample delayIndex){
		static thread_local FixedRandom random;
		for(u_index i=0, j=delayIndex + 3;i < j;i++){
			u_sample x=(u_sample)i / delayIndex;
			u_sample r1=(x * 2 - 1) / 4;
			//r1+=(pwm(x,0.4,0.3,0.3,0)+1)/8;
			r1+=random.next();
			buffer.write(r1 * note.velocitySynth);
		}
	}
}