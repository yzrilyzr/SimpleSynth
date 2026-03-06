#include "TimeEnvelop.h"
#include "dsp/DSP.h"
using namespace yzrilyzr_util;

namespace yzrilyzr_simplesynth{
	void TimeEnvelop::onRegisterParam(){
		RegisterUtil::registerParamTimeMs(*this, "Time(ms)", &duration);
	}

}