#include "ClampAmp.h"
#include "util/Util.h"
#include "lang/StringFormat.hpp"
#include "dsp/DSP.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void ClampAmp::onRegisterParam(){
		AmpUnaryComposition::onRegisterParam();
		RegisterUtil::registerParamGain(*this, "InputGain", &inputGain);
		RegisterUtil::registerParamGain(*this, "Clamp", &clamp);
		RegisterUtil::registerParamGain(*this, "OutputGain", &outputGain);
	}
	ClampAmp::ClampAmp(NoteProcPtr a, u_sample inputGain, u_sample clamp, u_sample outputGain) : AmpUnaryComposition(a){
		this->inputGain=inputGain;
		this->outputGain=outputGain;
		this->clamp=abs(clamp);
	}
	ClampAmp::ClampAmp() :AmpUnaryComposition(nullptr){
		
	}
	u_sample ClampAmp::getAmp(const Note & note){
		return Util::clamp(inputGain * a->getAmp(note), -clamp, clamp) * outputGain;
	}
	NoteProcPtr ClampAmp::clone(){
		return mksp<ClampAmp>(a->clone(), inputGain, clamp, outputGain);
	}
	String ClampAmp::toString() const{
		return StringFormat::object2string("ClampAmp", a, inputGain, clamp, outputGain);
	}
}