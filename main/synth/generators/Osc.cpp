#include "Osc.h"
#include "synth/source/AmplitudeSources.h"

using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	Osc::Osc(){
		registerParamPhaseSrc("PhaseSrc", &_freq);
		registerParamSrc("PM", &_pm);
		registerParamSrc("LPM", &_lpm);
		static double pmMin=0, pmMax=10;
		registerParam("PM Amp", ParamType::Double, &pmAmp, &pmMin, &pmMax);
		registerParam("LPM Amp", ParamType::Double, &lpmAmp, &pmMin, &pmMax);
	}
	Osc::Osc(std::shared_ptr<PhaseSrc> freq) :Osc(){
		setPhaseSource(freq == nullptr?NotePhase:freq);
	}
	std::shared_ptr<PhaseSrc> Osc::getPhaseSource()const{
		return this->_freq;
	}
	void Osc::init(ChannelConfig & cfg){
		if(_freq == nullptr)throw NullPointerException("freq == null");
		_freq->init();
		if(_pm!=nullptr)_pm->init(cfg);
		if(_lpm!=nullptr)_lpm->init(cfg);
	}

	void Osc::setPhaseSource(std::shared_ptr<PhaseSrc> freq){
		this->_freq=freq;
	}
	void Osc::pm(std::shared_ptr<Osc> pmSrc, double pmAmp, double noteRatio){
		pm(pmSrc, pmAmp);
		pmSrc->setPhaseSource(MulPhase(this->_freq, noteRatio));
	}
	void Osc::pm(NoteProcPtr pmSrc, double pmAmp){
		this->_pm=pmSrc;
		this->pmAmp=pmAmp;
	}
	void Osc::lpm(std::shared_ptr<Osc> lpmSrc, double lpmAmp, u_freq lpmHz){
		lpm(lpmSrc, lpmAmp);
		lpmSrc->setPhaseSource(ConstPhase(lpmHz));
	}
	void Osc::lpm(NoteProcPtr lpmSrc, double lpmAmp){
		this->_lpm=lpmSrc;
		this->lpmAmp=pmAmp;
	}
	std::string Osc::toString()const{
		return StringFormat::format("Osc(%s)", _freq);
	}
}