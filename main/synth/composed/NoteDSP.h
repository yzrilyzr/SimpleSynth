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
	yzrilyzr_dsp::DSPPtr dsp=nullptr;
	};
	ECLASS(NoteDSP, public AmpUnaryComposition, NoteData<NoteDSPKeyData>){
	private:
	yzrilyzr_dsp::DSPPtr dsp=nullptr;
	public:
	~NoteDSP()=default;
	NoteDSP() :AmpUnaryComposition(nullptr){
	}
	NoteDSP(NoteProcPtr a, yzrilyzr_dsp::DSPPtr dsp);
	void init(ChannelConfig & cfg) override;
	u_sample getAmp(const Note & note) override;
	NoteProcPtr clone() override;
	void onRegisterParam()override;
	yzrilyzr_lang::String toString() const override;
	U_CLASS_INFO(NoteDSP)
	NoteDSPKeyData * init(NoteDSPKeyData * data, const Note & note) override;
	};
}