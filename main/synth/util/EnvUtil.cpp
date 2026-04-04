#include "EnvUtil.h"
#include "interpolator/Interpolator.h"
#include "interpolator/PowInterpolator.h"
#include "interpolator/LineInterpolator.h"
using namespace yzrilyzr_interpolator;
namespace yzrilyzr_simplesynth{
	u_sp<Interpolator> EnvUtil::Pow(float p){
		return mksp<PowInterpolator>(p);
	}
	u_sp<Interpolator> EnvUtil::Line(){
		return mksp<LineInterpolator>();
	}
	u_sp<Interpolator> Line(){
		return mksp<LineInterpolator>();
	}
	u_sp<Interpolator> Pow(float p){
		return mksp<PowInterpolator>(p);
	}
}