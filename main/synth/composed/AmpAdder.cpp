#include "AmpAdder.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	String AmpAdder::toString() const{
		return StringFormat::object2string("AmpAdder", a, b);
	}
}