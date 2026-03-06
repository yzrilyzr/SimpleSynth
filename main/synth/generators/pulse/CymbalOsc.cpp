#include "CymbalOsc.h"
#include "synth/source/AmplitudeSources.h"
#include "dsp/FastSin.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_array;
namespace yzrilyzr_simplesynth{
	void CymbalOsc::onRegisterParam(){
		Osc::onRegisterParam();
		static double mulMin=0, mulMax=10;
		registerParam("Multiply", yzrilyzr_util::ParamType::NoteSrc, &mul, &mulMin, &mulMax);
		registerParamIntArray("OscFreq", &osc);
		RegisterUtil::registerParamNormal01(*this, "NoiseMix", &mix);
	}
	CymbalOsc::CymbalOsc() : CymbalOsc(ConstAmp(1),0.12, IntArray(nullptr)){}
	CymbalOsc::CymbalOsc(NoteProcPtr mul, u_normal_01 mix) :CymbalOsc(nullptr, mul, mix, IntArray(nullptr)){}
	CymbalOsc::CymbalOsc(NoteProcPtr mul, u_normal_01 mix, const IntArray & arr) :CymbalOsc(nullptr, mul, mix, arr){}
	CymbalOsc::CymbalOsc(u_sp<PhaseSrc> freqSrc, NoteProcPtr mul, u_normal_01 mix, const IntArray & arr) : Osc(freqSrc), mul(mul), mix(mix){
		if(arr != nullptr)osc=Arrays::copyOf(arr, arr.length);
	}
	void CymbalOsc::init(ChannelConfig & cfg){
		if(mul == nullptr)mul=ConstAmp(1);
		if(osc == nullptr)osc=IntArray{305, 444, 558, 630, 794, 824, 1136};
	}
	u_sample CymbalOsc::getAmp(const Note & note){
		u_sample sum=0;
		u_sample mul1=mul->getAmp(note);
		u_time time=note.passedTime;
		for(int i=0, j=osc.length;i < j;i++){
			sum+=CymbalOsc::square(osc[i] * mul1, time);
		}
		sum*=(1.0 - mix) * 2.0 / osc.length;
		sum+=random.next() * mix;
		return sum * note.velocitySynth;
	}
	u_sample CymbalOsc::square(u_freq Hz, u_time time){
		s_phase ft=Hz * time;
		ft=ft - (int)ft;
		u_sample sp=ft < 0.5?1:-1;
		return sp - yzrilyzr_dsp::fast_sin(ft * Math::TAU, Hz);
	}
	NoteProcPtr CymbalOsc::clone(){
		return mksp<CymbalOsc>(getPhaseSource(), mul, mix, osc);
	}
	String CymbalOsc::toString()const{
		return StringFormat::object2string("CymbalOsc", getPhaseSource(), mul);
	}
}