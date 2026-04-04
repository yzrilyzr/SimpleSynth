#include "NoteDSP.h"
#include "dsp/DSP.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;

namespace yzrilyzr_simplesynth{
	NoteDSP::NoteDSP(NoteProcPtr a, DSPPtr dsp) : AmpUnaryComposition(a){
		this->dsp=dsp;
	}
	void NoteDSP::getAmpBlock(const Note * noteSnapshots, u_sample * output, u_index length){
		auto & data=*getData(noteSnapshots[0]);
		a->getAmpBlock(noteSnapshots, output, length);
		data.dsp->procBlock(output, length);
	}

	void NoteDSP::init(ChannelConfig & cfg){
		AmpUnaryComposition::init(cfg);
		if(dsp == nullptr)throw NullPointerException("dsp == null");
		this->dsp->init(cfg.sampleRate);
		this->dsp->resetMemory();
	}
	u_sample NoteDSP::getAmp(const Note & note){
		return getData(note)->dsp->procDsp(a->getAmp(note));
	}
	NoteProcPtr NoteDSP::clone(){
		return mksp<NoteDSP>(a->clone(), dsp->clone());
	}
	NoteDSPKeyData * NoteDSP::init(NoteDSPKeyData * data, const Note & note){
		if(data == nullptr){
			data=new NoteDSPKeyData();
			data->dsp=dsp->newInstance();
		}
		data->dsp->cloneParam(dsp.get());
		data->dsp->init(note.cfg->sampleRate);
		data->dsp->resetMemory();
		return data;
	}
	String NoteDSP::toString() const{
		return StringFormat::object2string("NoteDSP", a, dsp);
	}

	void NoteDSP::onRegisterParam(){
		AmpUnaryComposition::onRegisterParam();
		RegisterUtil::registerParamDSP(*this, "DSP", &dsp);
	}
}