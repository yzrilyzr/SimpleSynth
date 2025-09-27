#include "AmpMultiplier.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	std::string AmpMultiplier::toString() const{
		return StringFormat::object2string("AmpMultiplier", a, b);
	}
}