#pragma once
#include "events/Note.h"
#include "interface/NoteProcessor.h"
#include "synth/composed/NonInterpolateAmpSet.h"
#include "synth/generators/sine/SineWaveTable.h"

namespace yzrilyzr_simplesynth{
	ECLASS(SimpleDrumSet, public NonInterpolateAmpSet){
	public:
		NoteProcPtr (*bassDrum)()=nullptr;
		SimpleDrumSet();
		static NoteProcPtr kickBassDistortion();
		static NoteProcPtr kickBassRaw();
		static NoteProcPtr kickPower();
		static NoteProcPtr kickBassTrap();
		static NoteProcPtr tom(u_sample_rate sampleRate, double scale);
		static NoteProcPtr tom(u_sample_rate sampleRate);
		static u_sp<SineWaveTable> risset();
		void setBassDrumType(NoteProcPtr (*bd)()){
			bassDrum=bd;
		}
		void init(ChannelConfig & cfg) override;
	};
}
