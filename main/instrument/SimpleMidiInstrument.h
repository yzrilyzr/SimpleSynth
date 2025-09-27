#pragma once
#include "interface/InstrumentProvider.h"

namespace yzrilyzr_simplesynth{
	ECLASS(SimpleMidiInstrument, public InstrumentProvider){
	public:
	NoteProcPtr get(s_bank_id bank, s_program_id program, u_sample_rate sampleRate) override;
	NoteProcPtr getDrumSet(s_bank_id bank, u_sample_rate sampleRate) override;
	};
}