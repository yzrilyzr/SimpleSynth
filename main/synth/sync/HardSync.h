#pragma once
#include "SimpleSynth.h"
#include "events/NoteData.hpp"
#include "interface/NoteProcessor.h"
#include "synth/composed/AmpUnaryComposition.h"


namespace yzrilyzr_simplesynth{
	EBCLASS(HardSyncKeyData){
	public:
	s_phase lastMasterPhase; // 记录上一帧的主振荡器相位
	s_phase phaseSynth;
	};
	ECLASS(HardSync, public AmpUnaryComposition, NoteData<HardSyncKeyData>){
	private:
	float slaveFreqRatio=2.0f;

	public:
	HardSync();
	HardSync(NoteProcPtr slave, float slaveFreqRatio);
	u_sample getAmp(const Note & note) override;
	NoteProcPtr clone() override;
	HardSyncKeyData * init(HardSyncKeyData * data, const Note & note) override;
	yzrilyzr_lang::String toString() const override;
	};
}