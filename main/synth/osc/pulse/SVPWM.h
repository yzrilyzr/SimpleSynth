#pragma once
#include "interface/NoteProcessor.h"
#include "interface/PhaseSrc.h"

namespace yzrilyzr_simplesynth{
	ECLASS(SVPWM, public NoteProcessor){
	public:
	SVPWM();

	u_sample getAmp(const Note & note) override;

	// 参数设置方法
	void setSyncModulation(bool enable);
	void setSyncRatio(float ratio);
	void setAsyncCarrierFreq(float freq);
	void setVoltageRatio(float ratio);
	void setFrequency(float freq);
	void setModulationPhase(float phase);
	void setThirdHarmonicInjection(bool enable);
	void setCornerModulation(bool enable);

	
	bool syncModulation;        // 是否同步调制
	float syncRatio;            // 同步调制比
	float asyncCarrierFreq;     // 异步载波频率(Hz)
	float voltageRatio;         // 变压比(0-1)
	float frequency;            // 变频(Hz)
	float modulationPhase;      // 调制波初始相位(0-1)
	bool thirdHarmonicInjection; // 是否使用三次谐波注入
	bool cornerModulation;       // 是否使用折角调制

	yzrilyzr_lang::String toString() const override;
	U_CLASS_INFO(SVPWM)
	};
}
