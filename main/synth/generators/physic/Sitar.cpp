#include "Sitar.h"
#include "interface/NoteTuning.h"
#include "util/Util.h"
#include "dsp/BufferDelayer.h"
#include "dsp/IIRUtil.h"
#include "dsp/FilterPassType.h"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
using namespace yzrilyzr_array;
namespace yzrilyzr_simplesynth{
	Sitar::~Sitar(){
		delete[] resonanceStrings;
		delete[] resonanceStringDelays;
	}
	Sitar::Sitar() : Osc(nullptr){}
	void Sitar::init(ChannelConfig & cfg){
		resonanceStrings=new RingBufferSample[resonanceStringsCount];
		resonanceStringDelays=new RingBufferSample[resonanceStringsCount];
		resonanceStringFreq=std::make_shared<DoubleArray>(resonanceStringsCount);
		Osc::init(cfg);
		int initI=Note::C4;
		int * ids=new int[14]{initI, initI + 2, initI + 4, initI + 5, initI + 7, initI + 9, initI + 11, initI + 12, initI + 14, initI + 16, initI + 17, initI + 19, initI + 21, initI + 23};
		static thread_local u_index randomIndex=0;
		for(u_index i=0;i < resonanceStringsCount;i++){
			(*resonanceStringFreq)[i]=cfg.tuning->getFrequency(ids[i] + random.next(&randomIndex) * 0.01 / 12.0);
		}
		delete[] ids;
		boxFilter=IIRUtil::newButterworthIIRFilter(cfg.sampleRate, FilterPassType::BANDPASS, 1, 100, 500);
		filter=IIRUtil::newButterworthIIRFilter(cfg.sampleRate, FilterPassType::BANDPASS, 2, 20, 20000);
		boxFilter->init(cfg.sampleRate);
		filter->init(cfg.sampleRate);
	}
	u_sample Sitar::postProcess(u_sample output){
		u_sample allActiveString=Osc::postProcess(output);
		u_sample resonSum=0;
		DoubleArray & l_resonanceStringFreq=*resonanceStringFreq;
		for(u_index i=0;i < resonanceStringsCount;i++){
			u_sample delayLen=(i + 1) * 5 / 1000.0 * 44100;
			resonanceStringDelays[i].ensureCapacity(delayLen);
			resonanceStringDelays[i].write(allActiveString);
			u_sample resonanceDelayOut=BufferDelayer::cubicSplineDelay(resonanceStringDelays[i], delayLen) * 0.2;
			u_sample resonanceStringOut=procKS(resonanceStrings[i], 0.99, 0.99, resonanceDelayOut, l_resonanceStringFreq[i]);
			resonSum+=Util::clamp(resonanceStringOut, static_cast<u_sample>(-1.0), static_cast<u_sample>(1.0));
		}
		resonSum/=resonanceStringsCount;
		u_sample out=allActiveString / 2.0 + resonSum;
		out+=boxFilter->procDsp(out) / 2;
		out=filter->procDsp(out);
		return out;
	}
	NoteProcPtr Sitar::clone(){
		return std::make_shared<Sitar>();
	}
	u_sample Sitar::getAmp(Note & note){
		RingBufferSample & buffer=*getData(note);
		u_freq freq2=getSetFreq(note);
		u_sample len=RingBufferUtil::freq2delayIndex(freq2, note.cfg->sampleRate);
		u_sample string1=procKS(buffer, 0.75, 1, 0, len);
		string1=Util::clamp(string1 * 2, static_cast<u_sample>(-1.0), static_cast<u_sample>(1.0));
		return string1;
	}
	RingBufferSample * Sitar::init(RingBufferSample * buffer, Note & note){
		if(buffer == nullptr) buffer=new RingBufferSample();
		initBuffer(*buffer, note);
		return buffer;
	}
	u_freq Sitar::getSetFreq(Note & note){
		return note.freqSynth;
	}
	u_sample Sitar::procKS(RingBufferSample & buffer, u_sample alpha, u_sample feedback, u_sample input, u_sample delayLen){
		buffer.ensureCapacity(delayLen);
		u_sample delayed=BufferDelayer::cubicSplineDelay(buffer, delayLen);
		u_sample newest=buffer.newest();
		u_sample alpha2=pow(alpha, delayLen / 367.0);
		u_sample sum=newest * (1 - alpha2) + delayed * alpha2;
		sum+=input;
		buffer.write(sum * feedback);
		return sum;
	}
	void Sitar::initBuffer(RingBufferSample & buffer, Note & note){
		buffer.reset();
		u_freq freq=note.cfg->tuning->getFrequencyByID(note.id);
		u_sample len=RingBufferUtil::freq2delayIndex(freq, note.cfg->sampleRate);
		buffer.ensureCapacity(len);
		initBurstRandom(buffer, note, len);
	}
	void Sitar::initBurstRandom(RingBufferSample & buffer, Note & note, u_sample delayIndex){
		static thread_local u_index randomIndex=0;
		for(u_index i=0, j=delayIndex + 3;i < j;i++){
			u_sample x=(u_sample)i / delayIndex;
			u_sample r1=(x * 2 - 1) / 4;
			//r1+=(pwm(x,0.4,0.3,0.3,0)+1)/8;
			r1+=random.next(&randomIndex);
			buffer.write(r1 * note.velocitySynth);
		}
	}
}