#include "AmpAdder.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	std::string AmpAdder::toString() const{
		return StringFormat::object2string("AmpAdder", a, b);
	}
}