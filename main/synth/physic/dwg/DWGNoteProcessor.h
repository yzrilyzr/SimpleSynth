#pragma once
#include "interface/NoteProcessor.h"
#include "DigitalWaveGuide.h"
#include "SimpleSynth.h"
#include "events/NoteData.hpp"


namespace yzrilyzr_simplesynth{
	ECLASS(DWGNoteProcessor, public NoteProcessor, public NoteData<DigitalWaveGuide>){
	public:
	NoteProcPtr left;//左DWG节点
	NoteProcPtr right;//右DWG节点
	NoteProcPtr load;//激励负载输入
	u_sample Z=1;//波导阻抗
	u_sample damper=1;//波导阻尼系数
	u_sp<PhaseSrc> delayFreq1;
	u_sp<PhaseSrc> delayFreq2;
	int loadAt=0;//取输入位置，左 0，右1
	int signalAt=0;//取输出位置，左 0，右1
	yzrilyzr_dsp::DSPPtr lowpass;
	yzrilyzr_collection::ArrayList<yzrilyzr_dsp::DSPPtr> dispersion;
	u_sp<PhaseSrc> fracDelay;
	DWGNoteProcessor()=default;
	void init(ChannelConfig & cfg)override;
	u_sample getAmp(const Note & note)override;
	void onRegisterParam()override;
	yzrilyzr_lang::String toString()const override;
	DigitalWaveGuide * init(DigitalWaveGuide * data, const Note & note)override;
	U_CLASS_INFO(DWGNoteProcessor)
	};
}