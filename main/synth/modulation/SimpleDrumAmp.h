#pragma once
#include "synth/modulation/NoteModulation.h"
#include "interpolator/Interpolator.h"
#include "events/NoteData.hpp"

namespace yzrilyzr_simplesynth{
	EBCLASS(SimpleDrumAmpKeyData){
	public:
	s_phase phase=0;
	};
	ECLASS(SimpleDrumAmp, public NoteModulation, NoteData<SimpleDrumAmpKeyData>){
	public:
		/**
		 * 固定频率
		 */
	static constexpr int const MODE_FIXED=0;
	/**
	 * 音符频率倍数
	 */
	static constexpr int const MODE_NOTE_RATIO=1;
	private:
	u_freq startFreq;
	u_freq endFreq;
	u_time duration;
	u_sp<yzrilyzr_interpolator::Interpolator> curve;
	int key=-1;
	int mode=MODE_FIXED;
	public:
	SimpleDrumAmp(u_freq startFreq, u_freq endFreq, u_time duration);
	SimpleDrumAmp(NoteProcPtr osc, u_freq startFreq, u_freq endFreq, u_time duration);
	SimpleDrumAmp(NoteProcPtr osc, u_freq startFreq, u_freq endFreq, u_time duration, int mode, u_sp<yzrilyzr_interpolator::Interpolator> curve);
	SimpleDrumAmp(NoteProcPtr osc, u_freq startFreq, u_freq endFreq, u_time duration, u_sp<yzrilyzr_interpolator::Interpolator> curve);
	SimpleDrumAmp();
	void applyMod(Note & note)override;
	bool noMoreData(const Note & note) const override;
	NoteProcPtr clone() override;
	SimpleDrumAmpKeyData * init(SimpleDrumAmpKeyData * data, const Note & note) override;
	yzrilyzr_lang::String toString()const override;
	void onRegisterParam() override;
	U_CLASS_INFO(SimpleDrumAmp);
	};
}