#include "SoftClipAmp.h"
#include "events/Note.h"
#include "yzrutil.h"
#include <cmath>

using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	SoftClipAmp::SoftClipAmp(NoteProcPtr a, double inputGain, double threshold, double outputGain)
		: AmpUnaryComposition(a){
		this->inputGain=abs(inputGain);
		this->threshold=abs(threshold);
		this->outputGain=abs(outputGain);
	}

	SoftClipAmp::SoftClipAmp()
		: SoftClipAmp(nullptr, 2.0, 0.5, 0.9){  // 默认：较强输入增益+中等阈值
		static double gainMin=0.1, gainMax=10.0;
		static double thresholdMin=0.1, thresholdMax=1.0;
		registerParam("InputGain", ParamType::Double, &inputGain, &gainMin, &gainMax);
		registerParam("Threshold", ParamType::Double, &threshold, &thresholdMin, &thresholdMax);
		registerParam("OutputGain", ParamType::Double, &outputGain, &gainMin, &gainMax);
	}

	u_sample SoftClipAmp::getAmp(Note & note){
		u_sample input=inputGain * a->getAmp(note);
		double in=static_cast<double>(input);
		double softClipped;

		// 软剪辑算法（三次函数平滑压缩）
		if(abs(in) <= threshold){
			softClipped=in;  // 低于阈值：无失真
		} else{
			// 超阈值部分：(3*threshold - in²/threshold)/2 （保证连续性和光滑性）
			double sign=1;//(in > 0)?1.0:-1.0;
			softClipped=sign * (3 * threshold - (threshold * threshold) / in) / 2.0;
		}

		return static_cast<u_sample>(softClipped * outputGain);
	}

	NoteProcPtr SoftClipAmp::clone(){
		return mksp<SoftClipAmp>(a->clone(), inputGain, threshold, outputGain);
	}

	String SoftClipAmp::toString() const{
		return StringFormat::object2string("SoftClipAmp", a, inputGain, threshold, outputGain);
	}
}