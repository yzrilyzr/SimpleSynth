#pragma once
#include "SimpleSynth.h"
#include "events/NoteData.hpp"
#include "interface/NoteProcessor.h"
#include "AmpUnaryComposition.h"
#include "interpolator/Interpolator.h"

namespace yzrilyzr_simplesynth{
	// 每个Note的同步数据（扩展插值器支持）
	EBCLASS(SoftSyncKeyData){
	public:
	s_phase lastMasterPhase=0;  // 上一帧主振荡器相位
	s_phase slavePhase=0;       // 从振荡器相位
	s_phase syncStartPhase=0;   // 同步起始相位（用于插值）
	float syncProgress=0.0f; // 同步进度[0,1]
	};

	// 软同步处理器
	ECLASS(SoftSync, public AmpUnaryComposition, public NoteData<SoftSyncKeyData>){
	private:
	float slaveFreqRatio=2.0f;
	std::shared_ptr<yzrilyzr_interpolator::Interpolator> syncInterp; // 同步相位插值器
	public:
	SoftSync();
	SoftSync(NoteProcPtr slave, float ratio, std::shared_ptr<yzrilyzr_interpolator::Interpolator> interp);
	u_sample getAmp(Note & note) override;
	static inline s_phase lerpPhase(s_phase start, s_phase end, s_phase t){
		return start + (end - start) * t; // t∈[0,1]
	}
	// 设置同步插值器（如Bezier、Pow等）
	void setSyncInterpolator(std::shared_ptr<yzrilyzr_interpolator::Interpolator> interp);
	NoteProcPtr clone() override;
	SoftSyncKeyData * init(SoftSyncKeyData * data, Note & note) override;
	std::string toString() const override;
	};
}