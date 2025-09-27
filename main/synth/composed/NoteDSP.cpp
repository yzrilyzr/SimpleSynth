#include "NoteDSP.h"
#include "dsp/DSP.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;

namespace yzrilyzr_simplesynth{
	NoteDSP::NoteDSP(NoteProcPtr a, std::shared_ptr<DSP> dsp) : AmpUnaryComposition(a){
		this->dsp=dsp;
	}
	void NoteDSP::init(ChannelConfig & cfg){
		AmpUnaryComposition::init(cfg);
		if(dsp == nullptr)throw NullPointerException("dsp == null");
		this->dsp->init(cfg.sampleRate);
		this->dsp->resetMemory();
	}
	u_sample NoteDSP::getAmp(Note & note){
		return getData(note)->dsp->procDsp(a->getAmp(note));
	}
	NoteProcPtr NoteDSP::clone(){
		return std::make_shared<NoteDSP>(a->clone(), dsp);
	}
	NoteDSPKeyData * NoteDSP::init(NoteDSPKeyData * data, Note & note){
		if(data == nullptr){
			data=new NoteDSPKeyData();
			data->dsp=dsp->cloneDSP();
		}
		//std::cout << (int)note.uniqueID << std::endl;
		data->dsp->cloneParam(dsp.get());
		data->dsp->init(note.cfg->sampleRate);
		data->dsp->resetMemory();
		return data;
	}
	std::string NoteDSP::toString() const{
		return StringFormat::object2string("DSP", a, dsp);
	}
}