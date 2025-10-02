#pragma once
#include "SynthUtil.h"
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "synth/generators/Osc.h"
#include "synth/composed/AmpBinaryComposition.h"
#include "dsp/BiquadIIR.h"
#include "dsp/FilterPassType.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(BiquadFilterSrcKeyData){
	public:
	yzrilyzr_dsp::BiquadIIR filter;
	};
	ECLASS(BiquadFilterSrc, public NoteProcessor, NoteData<BiquadFilterSrcKeyData>){
	private:
	yzrilyzr_dsp::FilterPassType type;
	NoteProcPtr src=nullptr;
	NoteProcPtr freqEnv=nullptr;
	NoteProcPtr qEnv=nullptr;
	NoteProcPtr gainEnv=nullptr;
	u_sample_rate sampleRate=0;
	ChannelConfig * cfg=nullptr;
	public:
		/*
		* @param freqEnv 滤波器的特征频率，以note id为标准，若freqEnv->getAmp()返回69，频率则为440Hz
		* @param qEnv 品质因素，参考IIRUtil::biquad
		* @param type 滤波器种类
		*/
	BiquadFilterSrc();
	BiquadFilterSrc(NoteProcPtr src, NoteProcPtr freqEnv, NoteProcPtr qEnv, yzrilyzr_dsp::FilterPassType type);
	BiquadFilterSrc(NoteProcPtr src, NoteProcPtr freqEnv, NoteProcPtr qEnv, NoteProcPtr gainEnv, yzrilyzr_dsp::FilterPassType type);
	void init(ChannelConfig & cfg) override;
	u_sample postProcess(u_sample output) override;
	void cc(ChannelConfig & cfg, ChannelControl & cc) override;
	u_sample getAmp(Note & note) override;
	bool noMoreData(Note & note) override;
	NoteProcPtr clone() override;
	BiquadFilterSrcKeyData * init(BiquadFilterSrcKeyData * data, Note & note) override;
	yzrilyzr_lang::String toString() const override;
	};
}