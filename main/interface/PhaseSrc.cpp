#include "interface/PhaseSrc.h"
#include "util/ParamRegister.h"
#include "NoteProcessor.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void PhaseSrc::registerParamPhaseSrc(const String & name, u_sp<PhaseSrc> * value){
		registerParam(name, ParamType::PhaseSrc, value, nullptr, nullptr);
	}
}