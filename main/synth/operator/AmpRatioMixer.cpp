#include "AmpRatioMixer.h"
#include "lang/StringFormat.hpp"
#include "dsp/DSP.h"
using namespace yzrilyzr_lang;
using namespace yzrilyzr_util;
namespace yzrilyzr_simplesynth{
	void AmpRatioMixer::onRegisterParam(){
		RegisterUtil::registerParamNormal11(*this, "Ratio", &ratio);
	}
	String AmpRatioMixer::toString() const{
		return StringFormat::object2string("AmpRatioMixer", a, b, ratio);
	}
}