#include "PostProcessDSP.h"
#include "dsp/DSP.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;

namespace yzrilyzr_simplesynth{
	PostProcessDSP::PostProcessDSP() :AmpUnaryComposition(nullptr){
		registerParamDSP("DSP", &dsp);
	}
	PostProcessDSP::PostProcessDSP(NoteProcPtr a, DSPPtr dsp) : AmpUnaryComposition(a){
		this->dsp=dsp;
	}
	void PostProcessDSP::init(ChannelConfig & cfg){
		AmpUnaryComposition::init(cfg);
		if(dsp == nullptr)throw NullPointerException("dsp == null");
		this->dsp->init(cfg.sampleRate);
		this->dsp->resetMemory();
	}
	u_sample PostProcessDSP::getAmp(Note & note){
		return a->getAmp(note);
	}
	u_sample PostProcessDSP::postProcess(u_sample output){
		return dsp->procDsp(a->postProcess(output));
	}
	NoteProcPtr PostProcessDSP::clone(){
		return std::make_shared<PostProcessDSP>(a->clone(), dsp->cloneDSP());
	}
	String PostProcessDSP::toString() const{
		return StringFormat::object2string("PostProcessDSP", a, dsp);
	}
}