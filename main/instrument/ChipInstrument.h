#pragma once
#include "interface/InstrumentProvider.h"

namespace yzrilyzr_simplesynth{
	ECLASS(ChipInstrument, public InstrumentProvider){
	public:
	static constexpr const int PULSE_12D5=0;
	static constexpr const int PULSE_25=1;
	static constexpr const int PULSE_50=2;
	static constexpr const int PULSE_75=3;
	static constexpr const int TRI=4;
	static constexpr const int SAW=5;
	static constexpr const int NOISE_SHORT=6;
	static constexpr const int NOISE_LONG=7;
	NoteProcPtr get(s_bank_id bank, s_program_id program, u_sample_rate sampleRate) override;
	NoteProcPtr getDrumSet(s_bank_id bank, u_sample_rate sampleRate) override;
	};
}