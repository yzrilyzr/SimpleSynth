#include "AmpQuantization.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	String AmpQuantization::toString() const{
		return StringFormat::object2string("AmpQuantization", a, quantization);
	}
}