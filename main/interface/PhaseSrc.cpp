#include "interface/PhaseSrc.h"
#include "util/ParamRegister.h"
#include "NoteProcessor.h"
using namespace yzrilyzr_util;
namespace yzrilyzr_simplesynth{
	void PhaseSrc::registerParamPhaseSrc(const std::string & name, std::shared_ptr<PhaseSrc> * value){
		registerParam(name, ParamType::PhaseSrc, value, nullptr, nullptr);
	}
}