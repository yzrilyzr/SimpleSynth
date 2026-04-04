#pragma once
#include "synth/set/NonInterpolateAmpSet.h"

namespace yzrilyzr_simplesynth{
	ECLASS(TR808DrumSet, public NonInterpolateAmpSet){
	public:
	TR808DrumSet()=default;
	void init(ChannelConfig & cfg) override;
	};
}
