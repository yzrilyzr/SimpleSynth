#include "ClampAmp.h"
#include "util/Util.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	ClampAmp::ClampAmp(NoteProcPtr a, u_sample inputGain, u_sample clamp, u_sample outputGain) : AmpUnaryComposition(a){
		this->inputGain=inputGain;
		this->outputGain=outputGain;
		this->clamp=abs(clamp);
	}
	ClampAmp::ClampAmp() :AmpUnaryComposition(nullptr){
		registerParamGain("InputGain", &inputGain);
		registerParamGain("Clamp", &clamp);
		registerParamGain("OutputGain", &outputGain);
	}
	u_sample ClampAmp::getAmp(Note & note){
		return Util::clamp(inputGain * a->getAmp(note), -clamp, clamp) * outputGain;
	}
	NoteProcPtr ClampAmp::clone(){
		return std::make_shared<ClampAmp>(a->clone(), inputGain, clamp, outputGain);
	}
	String ClampAmp::toString() const{
		return StringFormat::object2string("ClampAmp", a, inputGain, clamp, outputGain);
	}
}