#include "ArctanDistortion.h"
#include "events/Note.h"
#include "yzrutil.h"

using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	ArctanDistortion::ArctanDistortion(NoteProcPtr a, double inputGain, double alpha, double outputGain) : AmpUnaryComposition(a){
		this->inputGain=abs(inputGain);
		this->outputGain=abs(outputGain);
		this->alpha=abs(alpha);
	}
	ArctanDistortion::ArctanDistortion() :ArctanDistortion(nullptr, 1, 10, 1){
		static double alphaMin=0, alphaMax=100;
		registerParamGain("InputGain", &inputGain);
		registerParam("Alpha", ParamType::Double, &alpha, &alphaMin, &alphaMax);
		registerParamGain("OutputGain", &outputGain);
	}
	u_sample ArctanDistortion::getAmp(Note & note){
		return (2 / PI) * atan(inputGain * a->getAmp(note) * alpha) * outputGain;
	}
	NoteProcPtr ArctanDistortion::clone(){
		return std::make_shared<ArctanDistortion>(a->clone(), inputGain, alpha, outputGain);
	}
	String ArctanDistortion::toString() const{
		return StringFormat::object2string("ArctanDistortion", a, inputGain, alpha, outputGain);
	}
}