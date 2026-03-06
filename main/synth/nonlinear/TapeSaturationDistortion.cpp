#include "TapeSaturationDistortion.h"
#include "events/Note.h"
#include "yzrutil.h"
#include <cmath>

using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	TapeSaturationDistortion::TapeSaturationDistortion(NoteProcPtr a, double inputGain, double drive, double bias, double outputGain)
		: AmpUnaryComposition(a){
		this->inputGain=abs(inputGain);
		this->drive=abs(drive);
		this->bias=bias;  // 偏置可正可负（模拟不同磁带特性）
		this->outputGain=abs(outputGain);
	}

	TapeSaturationDistortion::TapeSaturationDistortion()
		: TapeSaturationDistortion(nullptr, 1.2, 0.8, 0.1, 0.9){  // 默认：轻微驱动+小偏置
		static double gainMin=0.1, gainMax=5.0;    // 输入/输出增益范围：0.1~5
		static double driveMin=0.0, driveMax=2.0;  // 驱动范围：0（无饱和）~2（强饱和）
		static double biasMin=-0.5, biasMax=0.5;   // 偏置范围：-0.5~0.5
		registerParam("InputGain", ParamType::Double, &inputGain, &gainMin, &gainMax);
		registerParam("Drive", ParamType::Double, &drive, &driveMin, &driveMax);
		registerParam("Bias", ParamType::Double, &bias, &biasMin, &biasMax);
		registerParam("OutputGain", ParamType::Double, &outputGain, &gainMin, &gainMax);
	}

	u_sample TapeSaturationDistortion::getAmp(const Note & note){
		u_sample input=inputGain * a->getAmp(note);
		double in=static_cast<double>(input);

		// 磁带饱和算法（双曲正切+偏置模拟）
		double saturated=tanh((in + bias) * drive) / tanh(drive);  // 归一化到[-1,1]
		// 注：tanh(drive)用于补偿驱动带来的音量提升，保证阈值内信号线性

		return static_cast<u_sample>(saturated * outputGain);
	}

	NoteProcPtr TapeSaturationDistortion::clone(){
		return mksp<TapeSaturationDistortion>(a->clone(), inputGain, drive, bias, outputGain);
	}

	String TapeSaturationDistortion::toString() const{
		return StringFormat::object2string("TapeSaturationDistortion", a, inputGain, drive, bias, outputGain);
	}
}