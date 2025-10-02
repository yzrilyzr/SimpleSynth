#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "AmpUnaryComposition.h"


namespace yzrilyzr_simplesynth{
	ECLASS(AutoMod, public AmpUnaryComposition){
	private:
	bool isMod=false;
	public:
	u_normal_01 modFreqDepth=0;
	u_normal_01 modAmpDepth=0;
	u_freq modRate=0;
	u_time modDelay=0;
	NoteProcPtr modShape=nullptr;
	~AutoMod()=default;
	AutoMod() :AmpUnaryComposition(nullptr){
		registerParamFreq("Freq", &modRate);
		registerParamNormal01("FreqDepth", &modFreqDepth);
		registerParamNormal01("AmpDepth", &modAmpDepth);
		registerParamTime("Delay", &modDelay);
		registerParamSrc("Shape", &modShape);
	}
	AutoMod(NoteProcPtr a, u_normal_01 modFreqDepth,u_normal_01 modAmpDepth, u_freq modRate, u_time modDelay, NoteProcPtr modShape=nullptr);
	u_sample getAmp(Note & note) override;
	bool noMoreData(Note & note) override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	};
}