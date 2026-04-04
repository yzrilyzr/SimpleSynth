#pragma once
#include "SimpleSynth.h"
#include "SynthUtil.h"
#include "dsp/IIR.h"
#include "dsp/FilterPassType.h"
#include "events/NoteData.hpp"
#include "interface/NoteProcessor.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(BiquadEnvFilterSrcKeyData){
	public:
	yzrilyzr_dsp::IIR filter;
	};
	ECLASS(BiquadEnvFilterSrc, public NoteProcessor, NoteData<BiquadEnvFilterSrcKeyData>){
	private:
	yzrilyzr_dsp::FilterPassType type;
	NoteProcPtr src=nullptr;
	NoteProcPtr freqEnv=nullptr;
	NoteProcPtr qEnv=nullptr;
	NoteProcPtr gainEnv=nullptr;
	public:
		/*
		* @param freqEnv 滤波器的特征频率，以note id为标准，若freqEnv->getAmp()返回69，频率则为440Hz
		* @param qEnv 品质因素，参考IIRUtil::biquad
		* @param type 滤波器种类
		*/
	BiquadEnvFilterSrc();
	BiquadEnvFilterSrc(NoteProcPtr src, NoteProcPtr freqEnv, NoteProcPtr qEnv, yzrilyzr_dsp::FilterPassType type);
	BiquadEnvFilterSrc(NoteProcPtr src, NoteProcPtr freqEnv, NoteProcPtr qEnv, NoteProcPtr gainEnv, yzrilyzr_dsp::FilterPassType type);
	void init(ChannelConfig & cfg) override;
	void postProcess(u_sample * input, u_index length) override;
	void cc(ChannelConfig & cfg, ChannelControl & cc) override;
	u_sample getAmp(const Note & note) override;
	bool noMoreData(const Note & note) const override;
	NoteProcPtr clone() override;
	BiquadEnvFilterSrcKeyData * init(BiquadEnvFilterSrcKeyData * data, const Note & note) override;
	yzrilyzr_lang::String toString() const override;
	void onRegisterParam() override;
	U_CLASS_INFO(BiquadEnvFilterSrc);

	};
}