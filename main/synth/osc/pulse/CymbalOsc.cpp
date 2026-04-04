#include "CymbalOsc.h"
#include "synth/util/AmplitudeSources.h"
#include "dsp/FastSin.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_array;
using namespace yzrilyzr_dsp;
namespace yzrilyzr_simplesynth{
	void CymbalOsc::onRegisterParam(){
		static double mulMin=0, mulMax=10;
		registerParam("Multiply", ParamType::NoteSrc, &mul, &mulMin, &mulMax);
		registerParamBool("RingMod", &ringMode);
		registerParamIntArray("OscFreq", &osc);
		RegisterUtil::registerParamNormal01(*this, "NoiseMix", &mix);
	}
	CymbalOsc::CymbalOsc() : CymbalOsc(ConstAmp(1), 0.12, IntArray(nullptr)){}
	CymbalOsc::CymbalOsc(NoteProcPtr mul, u_normal_01 mix) :CymbalOsc(mul, mix, IntArray(nullptr)){}
	CymbalOsc::CymbalOsc(NoteProcPtr mul, u_normal_01 mix, const IntArray & arr) : mul(mul), mix(mix){
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
		if(ringMode){
			for(int i=0, j=osc.length;i < j;i+=2){
				u_sample a=CymbalOsc::square(osc[i] * mul1, time);
				u_sample b=i + 1 >= j?1:CymbalOsc::square(osc[i + 1] * mul1, time);
				sum+=a * b;
			}
		} else{
			for(int i=0, j=osc.length;i < j;i++){
				sum+=CymbalOsc::square(osc[i] * mul1, time);
			}
		}

		sum*=(1.0 - mix) * 2.0 / osc.length;
		sum+=random.next() * mix;
		return sum * note.velocitySynth;
	}
	u_sample CymbalOsc::square(u_freq Hz, u_time time){
		s_phase ft=Hz * time;
		ft=ft - (int)ft;
		u_sample sp=ft < 0.5?1:-1;
		return ringMode?sp:sp - fast_sin(ft * Math::TAU, Hz);
	}
	NoteProcPtr CymbalOsc::clone(){
		return mksp<CymbalOsc>(mul, mix, osc);
	}
	String CymbalOsc::toString()const{
		return StringFormat::object2string("CymbalOsc", mul, mix, osc);
	}
}