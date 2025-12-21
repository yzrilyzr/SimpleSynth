#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"

#include "events/NoteData.hpp"
#include "events/Note.h"
#include "interface/PhaseSrc.h"

namespace yzrilyzr_simplesynth{
	ECLASS(Osc, public NoteProcessor){
	private:
	u_sp<PhaseSrc> _freq;
	NoteProcPtr _pm;
	NoteProcPtr _lpm;
	double pmAmp=0;
	double lpmAmp=0;
	public:
	Osc(u_sp<PhaseSrc> freq);
	Osc();
	s_phase getPhase(Note & note){
		s_phase t=_freq->getPhase(note);
		if(_pm != nullptr){
			t+=(s_phase)(_pm->getAmp(note) * pmAmp);
		}
		if(_lpm != nullptr){
			t+=(s_phase)(_lpm->getAmp(note) * lpmAmp);
		}
		return t;
	}
	void init(ChannelConfig & cfg)override;
	u_sp<PhaseSrc> getPhaseSource()const;
	void setPhaseSource(u_sp<PhaseSrc> freq);
	/**
	 * pm调制
	 *
	 * @param pmSrc     调制波源
	 * @param pmAmp     载波相位改变量
	 * @param noteRatio pm调制波频率与载波（音符）频率之比
	 */
	void pm(u_sp<Osc> pmSrc, double pmAmp, double noteRatio);
	void pm(NoteProcPtr pmSrc,double pmAmp);
	/**
	 * lpm 低频相位调制
	 *
	 * @param lpmSrc 调制波源
	 * @param lpmAmp 载波相位改变量
	 * @param lpmHz  调制波频率
	 */
	void lpm(u_sp<Osc> lpmSrc, double lpmAmp, u_freq lpmHz);
	void lpm(NoteProcPtr lpmSrc, double lpmAmp);
	yzrilyzr_lang::String toString() const override;
	};
}