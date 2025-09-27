#pragma once
#include "array/SampleProvider.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(SimpleWaveTable){
	public:
	static std::shared_ptr<yzrilyzr_array::SampleProvider> Piano_Wave;
	};
}