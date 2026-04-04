#pragma once
#include "SimpleSynth.h"
#include "synth/modulation/NoteModulation.h"
#include "events/NoteData.hpp"
#include "events/Note.h"
#include "interface/PhaseSrc.h"

namespace yzrilyzr_simplesynth{
	ECLASS(PhaseModAmp, public NoteModulation){
	private:
	u_sp<PhaseSrc> _freq;
	NoteProcPtr _pm;
	NoteProcPtr _lpm;
	double pmAmp=0;
	double lpmAmp=0;
	public:
	PhaseModAmp(NoteProcPtr a, u_sp<PhaseSrc> freq);
	PhaseModAmp();
	void init(ChannelConfig & cfg)override;
	u_sp<PhaseSrc> getPhaseSource()const;
	void setPhaseSource(u_sp<PhaseSrc> freq);
	/**
	 * pm调制
	 *
	 * @param pmSrc     调制波源
	 * @param pmAmp     载波相位改变量
	 */
	void pm(NoteProcPtr pmSrc, double pmAmp);
	/**
	 * lpm 低频相位调制
	 *
	 * @param lpmSrc 调制波源
	 * @param lpmAmp 载波相位改变量
	 */
	void lpm(NoteProcPtr lpmSrc, double lpmAmp);
	void applyMod(Note & note)override;
	yzrilyzr_lang::String toString() const override;
	void onRegisterParam() override;
	U_CLASS_INFO(PhaseModAmp);
	};
}