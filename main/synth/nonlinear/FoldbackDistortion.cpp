#include "FoldbackDistortion.h"
#include "events/Note.h"
#include "yzrutil.h"
#include <cmath>

using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	FoldbackDistortion::FoldbackDistortion(NoteProcPtr a, double inputGain, double threshold, double foldRatio, double outputGain)
		: AmpUnaryComposition(a){
		this->inputGain=abs(inputGain);
		this->threshold=abs(threshold);
		this->foldRatio=abs(foldRatio);
		this->outputGain=abs(outputGain);
	}

	FoldbackDistortion::FoldbackDistortion()
		: FoldbackDistortion(nullptr, 3.0, 0.6, 0.8, 0.7){  // 默认：强输入增益+中等折叠
		static double gainMin=0.1, gainMax=10.0;
		static double thresholdMin=0.1, thresholdMax=1.0;
		static double ratioMin=0.1, ratioMax=1.0;  // 折叠比率：0.1（弱折叠）~1（强折叠）
		registerParam("InputGain", ParamType::Double, &inputGain, &gainMin, &gainMax);
		registerParam("Threshold", ParamType::Double, &threshold, &thresholdMin, &thresholdMax);
		registerParam("FoldRatio", ParamType::Double, &foldRatio, &ratioMin, &ratioMax);
		registerParam("OutputGain", ParamType::Double, &outputGain, &gainMin, &gainMax);
	}

	u_sample FoldbackDistortion::getAmp(Note & note){
		u_sample input=inputGain * a->getAmp(note);
		double in=static_cast<double>(input);
		double folded;

		if(abs(in) <= threshold){
			folded=in;  // 低于阈值：无失真
		} else{
			// 折叠算法：超阈值部分 = 阈值 - (超阈值量 * 折叠比率)，保持符号
			double excess=abs(in) - threshold;
			folded=(in > 0)?(threshold - excess * foldRatio):(-threshold + excess * foldRatio);
			// 二次折叠（若折叠后仍超阈值，递归处理）
			if(abs(folded) > threshold){
				excess=abs(folded) - threshold;
				folded=(folded > 0)?(threshold - excess * foldRatio):(-threshold + excess * foldRatio);
			}
		}

		return static_cast<u_sample>(folded * outputGain);
	}

	NoteProcPtr FoldbackDistortion::clone(){
		return std::make_shared<FoldbackDistortion>(a->clone(), inputGain, threshold, foldRatio, outputGain);
	}

	std::string FoldbackDistortion::toString() const{
		return StringFormat::object2string("FoldbackDistortion", a, inputGain, threshold, foldRatio, outputGain);
	}
}