#pragma once
#include "SimpleSynth.h"
#include "events/Note.h"
#include "events/NoteData.hpp"
#include "interface/NoteProcessor.h"
#include "synth/envelopers/Enveloper.h"
#include "interpolator/Interpolator.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(AHDSREnvelopKeyData){
	public:
	u_time releaseTime=-1;
	u_sample releaseVolume=0;
	u_time attackTime=0;
	};
	ECLASS(AHDSREnvelop, public NoteProcessor, public Enveloper, NoteData<AHDSREnvelopKeyData>){
	public:
	u_time forceReleaseTime=0;
	u_time delayTime=0;
	u_time attackTime=0;
	u_time holdTime=0;
	u_time decayTime=0;
	bool canSustain=false;
	u_normal_01 sustainVolume=0;
	u_time releaseTime=0;
	u_sp<yzrilyzr_interpolator::Interpolator> aCurve=nullptr;
	u_sp<yzrilyzr_interpolator::Interpolator> dCurve=nullptr;
	u_sp<yzrilyzr_interpolator::Interpolator> rCurve=nullptr;
	AHDSREnvelop();
	/**
	 * 瞬时乐器：A D(敲击后非人为干预的自由振动) R(手动停止)
	 * 瞬时乐器：A D(衰减为1不变) R(无需手动停止)
	 * 持续乐器：A D(达峰后衰减) S(持续) R(手动停止)
	 *
	 * @param attackTime       (0,+∞)单位：毫秒ms
	 * @param decayTime        [0,+∞)单位：毫秒ms
	 * @param sustainVolume    [0-1],可以持续发声，敲击乐器等不需要此(0)
	 * @param releaseTime      (0,+∞)单位：毫秒ms
	 * @param forceReleaseTime 强制释放，例如连续击剑，(0,+∞)单位：毫秒ms
	 */
	AHDSREnvelop(u_time_ms delayTime, u_time_ms attackTime, u_time_ms holdTime,
				 u_time_ms decayTime, u_normal_01 sustainVolume, bool canSustain,
				 u_time_ms releaseTime, u_time_ms forceReleaseTime,
				 u_sp<yzrilyzr_interpolator::Interpolator> aCurve,
				 u_sp<yzrilyzr_interpolator::Interpolator> dCurve,
				 u_sp<yzrilyzr_interpolator::Interpolator> rCurve);
	bool noMoreData(Note & note) override;
	NoteProcPtr clone() override;
	void init(ChannelConfig & cfg) override;
	u_sample getAmp(Note & note) override;
	yzrilyzr_lang::String toString() const override;
	void cc(ChannelConfig & cfg, ChannelControl & cc)override;
	AHDSREnvelopKeyData * init(AHDSREnvelopKeyData * data, Note & note) override;
	private:
	bool isNoteNotReleased(Note & n) const;
	};
}
