#include "ClampWithVelocityAmp.h"
#include "events/Note.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	ClampWithVelocityAmp::ClampWithVelocityAmp(NoteProcPtr a, u_sample inputGain, u_sample clamp, u_sample outputGain) : AmpUnaryComposition(a){
		this->inputGain=inputGain;
		this->outputGain=outputGain;
		this->clamp=abs(clamp);
	}
	ClampWithVelocityAmp::ClampWithVelocityAmp() :AmpUnaryComposition(nullptr){
		registerParamGain("InputGain", &inputGain);
		registerParamGain("Clamp", &clamp);
		registerParamGain("OutputGain", &outputGain);
	}
	u_sample ClampWithVelocityAmp::getAmp(Note & note){
		u_sample cl=clamp * note.velocitySynth;
		return Util::clamp(inputGain * a->getAmp(note), -cl, cl) * outputGain;
	}
	NoteProcPtr ClampWithVelocityAmp::clone(){
		return std::make_shared<ClampWithVelocityAmp>(a->clone(), inputGain, clamp, outputGain);
	}
	String ClampWithVelocityAmp::toString() const{
		return StringFormat::object2string("ClampWithVelocityAmp", a, inputGain, clamp, outputGain);
	}
}