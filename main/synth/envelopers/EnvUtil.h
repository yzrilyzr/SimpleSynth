#pragma once
#include "SimpleSynth.h"
#include "interpolator/Interpolator.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(EnvUtil){
	public:
	static std::shared_ptr<yzrilyzr_interpolator::Interpolator> Pow(float p);
	static std::shared_ptr<yzrilyzr_interpolator::Interpolator> Line();
	};
	std::shared_ptr<yzrilyzr_interpolator::Interpolator> Pow(float p);
	std::shared_ptr<yzrilyzr_interpolator::Interpolator> Line();
}