#include "ClampWithVelocityAmp.h"
#include "events/Note.h"
#include "lang/StringFormat.hpp"
#include "dsp/DSP.h"
#include "util/Util.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void ClampWithVelocityAmp::onRegisterParam(){
		AmpUnaryComposition::onRegisterParam();
		RegisterUtil::registerParamGain(*this, "InputGain", &inputGain);
		RegisterUtil::registerParamGain(*this, "Clamp", &clamp);
		RegisterUtil::registerParamGain(*this, "OutputGain", &outputGain);
	}
	ClampWithVelocityAmp::ClampWithVelocityAmp(NoteProcPtr a, u_sample inputGain, u_sample clamp, u_sample outputGain) : AmpUnaryComposition(a){
		this->inputGain=inputGain;
		this->outputGain=outputGain;
		this->clamp=abs(clamp);
	}
	ClampWithVelocityAmp::ClampWithVelocityAmp() :AmpUnaryComposition(nullptr){	}
	u_sample ClampWithVelocityAmp::getAmp(const Note & note){
		u_sample cl=clamp * note.velocitySynth;
		return Util::clamp(inputGain * a->getAmp(note), -cl, cl) * outputGain;
	}
	NoteProcPtr ClampWithVelocityAmp::clone(){
		return mksp<ClampWithVelocityAmp>(a->clone(), inputGain, clamp, outputGain);
	}
	String ClampWithVelocityAmp::toString() const{
		return StringFormat::object2string("ClampWithVelocityAmp", a, inputGain, clamp, outputGain);
	}
}