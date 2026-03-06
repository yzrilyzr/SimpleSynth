#pragma once
#include "SimpleSynth.h"
#include "util/ClassRegister.h"

namespace yzrilyzr_simplesynth{
	class Note;
	ECLASS(PhaseSrc, public yzrilyzr_util::ClassRegister){
		public:
		virtual ~PhaseSrc()=default;
		virtual s_phase getPhase(const Note & note)=0;
		virtual void init(){}
		void registerParamPhaseSrc(const yzrilyzr_lang::String & name, u_sp<PhaseSrc> *value);
	};
}