#include "EnvUtil.h"
#include "interpolator/Interpolator.h"
#include "interpolator/PowInterpolator.h"
#include "interpolator/LineInterpolator.h"
using namespace yzrilyzr_interpolator;
namespace yzrilyzr_simplesynth{
	std::shared_ptr<Interpolator> EnvUtil::Pow(float p){
		return std::make_shared<PowInterpolator>(p);
	}
	std::shared_ptr<Interpolator> EnvUtil::Line(){
		return std::make_shared<LineInterpolator>();
	}
	std::shared_ptr<Interpolator> Line(){
		return std::make_shared<LineInterpolator>();
	}
	std::shared_ptr<Interpolator> Pow(float p){
		return std::make_shared<PowInterpolator>(p);
	}
}