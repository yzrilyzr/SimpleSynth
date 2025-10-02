#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "AmpUnaryComposition.h"
#include "array/Array.hpp"

namespace yzrilyzr_simplesynth{
	ECLASS(AmpWithCC, public AmpUnaryComposition){
	private:
	std::shared_ptr<yzrilyzr_array::IntArray> cc;
	public:
	~AmpWithCC();
	AmpWithCC();
	AmpWithCC(NoteProcPtr a, std::shared_ptr<yzrilyzr_array::IntArray> cc);
	u_sample getAmp(Note & note) override;
	NoteProcPtr clone() override;
	void init(ChannelConfig & cfg) override;
	yzrilyzr_lang::String toString() const override;
	};
}