#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "AmpUnaryComposition.h"

namespace yzrilyzr_simplesynth{
	ECLASS(AmpQuantization, public AmpUnaryComposition){
	public:
	uint32_t quantization=32767;
	AmpQuantization() :AmpQuantization(nullptr, 0){}
	AmpQuantization(NoteProcPtr a, uint32_t quantization) :AmpUnaryComposition(a), quantization(quantization){}
	u_sample getAmp(const Note & note) override{
		u_sample src=a->getAmp(note);
		u_sample q=static_cast<double>(quantization);
		src=static_cast<int32_t>(src * q);
		src/=q;
		return src;
	}
	NoteProcPtr clone() override{
		return mksp<AmpQuantization>(a, quantization);
	}
	yzrilyzr_lang::String toString() const override;
	void onRegisterParam() override;

	U_CLASS_INFO(AmpQuantization);
	};
}