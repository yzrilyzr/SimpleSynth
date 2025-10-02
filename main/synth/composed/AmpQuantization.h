#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "AmpUnaryComposition.h"

namespace yzrilyzr_simplesynth{
	ECLASS(AmpQuantization, public AmpUnaryComposition){
	public:
	uint32_t quantization=32767;
	AmpQuantization() :AmpQuantization(nullptr, 0){
		static uint32_t min=0, max=65536;
		registerParam("Quantization", yzrilyzr_util::ParamType::UInt, &quantization, &min, &max);
	}
	AmpQuantization(NoteProcPtr a, uint32_t quantization) :AmpUnaryComposition(a), quantization(quantization){}
	u_sample getAmp(Note & note) override{
		u_sample src=a->getAmp(note);
		u_sample q=static_cast<double>(quantization);
		src=static_cast<int32_t>(src * q);
		src/=q;
		return src;
	}
	NoteProcPtr clone() override{
		return std::make_shared<AmpQuantization>(a, quantization);
	}
	yzrilyzr_lang::String toString() const override;
	};
}