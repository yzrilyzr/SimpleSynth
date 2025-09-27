#pragma once
#include "SimpleSynth.h"
#include "util/ParamRegister.h"

namespace yzrilyzr_simplesynth{
	class Note;
	ECLASS(PhaseSrc, public yzrilyzr_util::ParamRegister){
		public:
		virtual ~PhaseSrc()=default;
		virtual s_phase getPhase(Note & note)=0;
		virtual void init(){}
		void registerParamPhaseSrc(const std::string & name, std::shared_ptr<PhaseSrc> *value);
	};
}