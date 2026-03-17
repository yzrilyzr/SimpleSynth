#include "ArctanDistortion.h"
#include "events/Note.h"
#include "yzrutil.h"
#include "dsp/DSP.h"
#include "lang/Math.h"

using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;

namespace yzrilyzr_simplesynth{
	void ArctanDistortion::onRegisterParam(){
		static double alphaMin=0, alphaMax=100;
		AmpUnaryComposition::onRegisterParam();
		RegisterUtil::registerParamGain(*this, "InputGain", &inputGain);
		registerParam("Alpha", ParamType::Double, &alpha, &alphaMin, &alphaMax);
		RegisterUtil::registerParamGain(*this, "OutputGain", &outputGain);
	}
	ArctanDistortion::ArctanDistortion(NoteProcPtr a, u_sample inputGain, double alpha, u_sample outputGain) : AmpUnaryComposition(a){
		this->inputGain=abs(inputGain);
		this->outputGain=abs(outputGain);
		this->alpha=abs(alpha);
	}
	ArctanDistortion::ArctanDistortion() :ArctanDistortion(nullptr, 1, 10, 1){}
	u_sample ArctanDistortion::getAmp(const Note & note){
		return (2 / Math::PI) * atan(inputGain * a->getAmp(note) * alpha) * outputGain;
	}
	NoteProcPtr ArctanDistortion::clone(){
		return mksp<ArctanDistortion>(a->clone(), inputGain, alpha, outputGain);
	}
	String ArctanDistortion::toString() const{
		return StringFormat::object2string("ArctanDistortion", a, inputGain, alpha, outputGain);
	}
}