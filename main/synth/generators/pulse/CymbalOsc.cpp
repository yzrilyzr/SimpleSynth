#include "CymbalOsc.h"
#include "synth/source/AmplitudeSources.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	CymbalOsc::CymbalOsc() : CymbalOsc(0){
		static double mulMin=0, mulMax=10;
		registerParam("Multiply", yzrilyzr_util::ParamType::Double, &mul, &mulMin, &mulMax);
	}
	CymbalOsc::CymbalOsc(double mul) : CymbalOsc(nullptr, ConstAmp(mul)){}
	CymbalOsc::CymbalOsc(NoteProcPtr mul) : CymbalOsc(nullptr, mul){}
	CymbalOsc::CymbalOsc(std::shared_ptr<PhaseSrc> freqSrc, NoteProcPtr mul) : Osc(freqSrc){
		this->mul=mul;
	}
	u_sample CymbalOsc::getAmp(Note & note){
		u_sample sum=0;
		u_sample mul1=mul->getAmp(note);
		u_time time=note.passedTime;
		sum+=CymbalOsc::square(1136 * mul1, time);
		sum+=CymbalOsc::square(794 * mul1, time);
		sum+=CymbalOsc::square(305 * mul1, time);
		sum+=CymbalOsc::square(444 * mul1, time);
		sum+=CymbalOsc::square(558 * mul1, time);
		sum+=CymbalOsc::square(824 * mul1, time);
		sum+=CymbalOsc::square(630 * mul1, time);
		static thread_local size_t randomIndex=0;
		sum+=random.next(&randomIndex)*0.7;
		return sum * note.velocitySynth * 0.15;
	}
	u_sample CymbalOsc::square(u_freq Hz, u_time time){
		s_phase ft=Hz * time;
		ft=ft - (int)ft;
		return ft > 0.5?1:-1;
	}
	NoteProcPtr CymbalOsc::clone(){
		return std::make_shared<CymbalOsc>(mul);
	}
	std::string CymbalOsc::toString()const{
		return StringFormat::object2string("CymbalOsc", getPhaseSource(), mul);
	}
}