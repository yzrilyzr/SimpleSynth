#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "AmpUnaryComposition.h"
#include "events/NoteData.hpp"

namespace yzrilyzr_dsp{
	class DSP;
}
namespace yzrilyzr_simplesynth{
	EBCLASS(NoteDSPKeyData){
	public:
	u_sp<yzrilyzr_dsp::DSP> dsp=nullptr;
	};
	ECLASS(NoteDSP, public AmpUnaryComposition, NoteData<NoteDSPKeyData>){
	private:
	u_sp<yzrilyzr_dsp::DSP> dsp=nullptr;
	public:
	~NoteDSP()=default;
	NoteDSP() :AmpUnaryComposition(nullptr){
		registerParamDSP("DSP", &dsp);
	}
	NoteDSP(NoteProcPtr a, u_sp<yzrilyzr_dsp::DSP> dsp);
	void init(ChannelConfig & cfg) override;
	u_sample getAmp(Note & note) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	NoteDSPKeyData * init(NoteDSPKeyData * data, Note & note) override;
	};
}