#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"

#include "events/NoteData.hpp"
#include "events/Note.h"
#include "interface/PhaseSrc.h"

namespace yzrilyzr_simplesynth{
	ECLASS(Osc, public NoteProcessor){
	private:
	std::shared_ptr<PhaseSrc> _freq;
	NoteProcPtr _pm;
	NoteProcPtr _lpm;
	double pmAmp=0;
	double lpmAmp=0;
	public:
	Osc(std::shared_ptr<PhaseSrc> freq);
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
	std::shared_ptr<PhaseSrc> getPhaseSource()const;
	void setPhaseSource(std::shared_ptr<PhaseSrc> freq);
	/**
	 * pm调制
	 *
	 * @param pmSrc     调制波源
	 * @param pmAmp     载波相位改变量
	 * @param noteRatio pm调制波频率与载波（音符）频率之比
	 */
	void pm(std::shared_ptr<Osc> pmSrc, double pmAmp, double noteRatio);
	void pm(NoteProcPtr pmSrc,double pmAmp);
	/**
	 * lpm 低频相位调制
	 *
	 * @param lpmSrc 调制波源
	 * @param lpmAmp 载波相位改变量
	 * @param lpmHz  调制波频率
	 */
	void lpm(std::shared_ptr<Osc> lpmSrc, double lpmAmp, u_freq lpmHz);
	void lpm(NoteProcPtr lpmSrc, double lpmAmp);
	std::string toString() const override;
	};
}