#pragma once
#include "synth/composed/NonInterpolateAmpSet.h"
#include "interface/NoteProcessor.h"
#include "events/Note.h"
#include "synth/source/AmpBuilder.h"
#include "interpolator/Interpolator.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(SimpleDrumAmpKeyData){
	public:
	s_phase freqTimeSynth=0;
	};
	ECLASS(SimpleDrumAmp, public NoteProcessor), NoteData<SimpleDrumAmpKeyData>{
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
	std::shared_ptr<yzrilyzr_interpolator::Interpolator> curve;
	NoteProcPtr src;
	int key=-1;
	int mode=MODE_FIXED;
	public:
	SimpleDrumAmp(u_freq startFreq, u_freq endFreq, u_time duration);
	SimpleDrumAmp(NoteProcPtr osc, u_freq startFreq, u_freq endFreq, u_time duration);
	SimpleDrumAmp(NoteProcPtr osc, u_freq startFreq, u_freq endFreq, u_time duration, int mode, std::shared_ptr<yzrilyzr_interpolator::Interpolator> curve);
	SimpleDrumAmp(NoteProcPtr osc, u_freq startFreq, u_freq endFreq, u_time duration, std::shared_ptr<yzrilyzr_interpolator::Interpolator> curve);
	SimpleDrumAmp();
	u_sample getAmp(Note & note) override;
	void init(ChannelConfig & cfg) override;
	bool noMoreData(Note & note) override;
	NoteProcPtr clone() override;
	SimpleDrumAmpKeyData * init(SimpleDrumAmpKeyData * data, Note & note) override;
	yzrilyzr_lang::String toString()const override;
	};
}