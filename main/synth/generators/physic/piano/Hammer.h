#pragma once
#include "SimpleSynth.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(Hammer){
	public:
		/**
		 * @param sampleRate 采样率
		 * @param m          质量，影响回到0的衰减时间，越大越长）
		 * @param K          力度，影响上升过程的斜率，越大斜率越高(线性)
		 * @param p          刚度，影响曲线隆起柔和程度，越小隆起越尖更快速斜率越大，越大隆起越平缓延迟更多斜率越小(指数)
		 * @param Z          阻抗，不影响上升斜率，同时影响上升的最大值和衰减时间，越小则峰值越小衰减时间更长，越大则峰值越大但是更容易衰减
		 * @param alpha
		 */
	virtual ~Hammer()=default;
	virtual void init(double sampleRate, double m, double K, double p, double Z, double alpha)=0;
	virtual double load(double in)=0;
	virtual void trigger(double v)=0;
	std::string toString()const override{ return "Hammer"; }
	};
}
