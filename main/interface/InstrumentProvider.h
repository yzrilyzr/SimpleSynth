#pragma once
#include "SimpleSynth.h"

namespace yzrilyzr_simplesynth{
	class NoteProcessor;
	typedef u_sp<NoteProcessor> NoteProcPtr;
	EBCLASS(InstrumentProvider){
		public:
		virtual ~InstrumentProvider()=default;
		virtual NoteProcPtr get(s_bank_id bank, s_program_id program, u_sample_rate sampleRate)=0;
		virtual NoteProcPtr getDrumSet(s_bank_id bank, u_sample_rate sampleRate)=0;
	};
}