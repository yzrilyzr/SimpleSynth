#include "SVPWM.h"
#include "events/Note.h"
#include "lang/StringFormat.hpp"
#include "lang/Math.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	SVPWM::SVPWM(){
		// 初始化默认参数
		syncModulation=true;
		syncRatio=1.0f;
		asyncCarrierFreq=1000.0f;
		voltageRatio=1.0f;
		frequency=50.0f;
		modulationPhase=0.0f;
		thirdHarmonicInjection=false;
		cornerModulation=false;
	}

	u_sample SVPWM::getAmp(const Note & note){
		// 获取当前时间（秒）
		float t=note.phaseSynth;

		// 计算调制波频率和电压比
		float modFreq=frequency;
		float modVoltage=voltageRatio;

		// 计算载波频率
		float carrierFreq=syncModulation?modFreq * syncRatio:asyncCarrierFreq;

		// 计算调制波相位（考虑初始相位）
		float modPhase= Math::TAU * (modFreq * t + modulationPhase);

		// 计算基本调制波
		float modulationWave=Math::sin(modPhase) * modVoltage;

		// 三次谐波注入
		if(thirdHarmonicInjection){
			modulationWave+=Math::sin(3.0f * modPhase) * modVoltage / 6.0f;
		}

		// 折角调制处理
		if(cornerModulation){
			float maxMod=1.0f / sqrt(3.0f);  // 最大线性调制比
			if(fabs(modulationWave) > maxMod){
				modulationWave=modulationWave > 0?maxMod:-maxMod;
			}
		}

		// 计算载波（三角波）
		float carrierPhase=fmod(carrierFreq * t, 1.0f);
		float carrierWave=2.0f * (carrierPhase < 0.5f?carrierPhase:1.0f - carrierPhase) - 0.5f;

		// SVPWM比较生成输出（0或1）
		u_sample output=(modulationWave > carrierWave)?1.0f:0.0f;

		return output * note.velocitySynth;
	}

	void SVPWM::setSyncModulation(bool enable){
		syncModulation=enable;
	}

	void SVPWM::setSyncRatio(float ratio){
		syncRatio=ratio;
	}

	void SVPWM::setAsyncCarrierFreq(float freq){
		asyncCarrierFreq=freq;
	}

	void SVPWM::setVoltageRatio(float ratio){
		voltageRatio=ratio;
	}

	void SVPWM::setFrequency(float freq){
		frequency=freq;
	}

	void SVPWM::setModulationPhase(float phase){
		modulationPhase=phase;
	}

	void SVPWM::setThirdHarmonicInjection(bool enable){
		thirdHarmonicInjection=enable;
	}

	void SVPWM::setCornerModulation(bool enable){
		cornerModulation=enable;
	}
	String SVPWM::toString() const{
		return StringFormat::object2string("SVPWM");
	}
}