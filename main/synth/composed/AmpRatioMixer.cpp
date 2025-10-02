#include "AmpRatioMixer.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	String AmpRatioMixer::toString() const{
		return StringFormat::object2string("AmpRatioMixer", a, b, ratio);
	}
}