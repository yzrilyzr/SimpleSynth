#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "synth/modulation/NoteModulation.h"

namespace yzrilyzr_simplesynth{
	ECLASS(AutoMod, public NoteModulation){
	private:
	mutable bool isMod=false;
	public:
	u_normal_01 modFreqDepth=0;
	u_normal_01 modAmpDepth=0;
	u_freq modRate=0;
	u_time modDelay=0;
	NoteProcPtr modShape=nullptr;
	~AutoMod()=default;
	AutoMod() :NoteModulation(nullptr){}
	AutoMod(NoteProcPtr a, u_normal_01 modFreqDepth, u_normal_01 modAmpDepth, u_freq modRate, u_time modDelay, NoteProcPtr modShape=nullptr);
	void applyMod(Note & note)override;
	//u_sample getAmp(const Note & note) override;
	bool noMoreData(const Note & note) const override;
	NoteProcPtr clone() override;
	yzrilyzr_lang::String toString() const override;
	void onRegisterParam() override;
	U_CLASS_INFO(AutoMod)
	};
}