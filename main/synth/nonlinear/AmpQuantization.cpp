#include "AmpQuantization.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_lang;
using namespace yzrilyzr_util;
namespace yzrilyzr_simplesynth{
	void AmpQuantization::onRegisterParam(){
		AmpUnaryComposition::onRegisterParam();
		static uint32_t min=1, max=65536;
		registerParam("Quantization", ParamType::UInt, &quantization, &min, &max);
	}
	String AmpQuantization::toString() const{
		return StringFormat::object2string("AmpQuantization", a, quantization);
	}
}