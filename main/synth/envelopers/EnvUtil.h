#pragma once
#include "SimpleSynth.h"
#include "interpolator/Interpolator.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(EnvUtil){
	public:
	static u_sp<yzrilyzr_interpolator::Interpolator> Pow(float p);
	static u_sp<yzrilyzr_interpolator::Interpolator> Line();
	};
	U_EXPORT_API u_sp<yzrilyzr_interpolator::Interpolator> Pow(float p);
	U_EXPORT_API u_sp<yzrilyzr_interpolator::Interpolator> Line();
}