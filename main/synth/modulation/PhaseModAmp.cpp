#include "PhaseModAmp.h"
#include "synth/util/AmplitudeSources.h"

using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void PhaseModAmp::onRegisterParam(){
		RegisterUtil::registerParamPhaseSrc(*this, "PhaseSrc", &_freq);
		RegisterUtil::registerParamSrc(*this, "PM", &_pm);
		RegisterUtil::registerParamSrc(*this, "LPM", &_lpm);
		static double pmMin=0, pmMax=10;
		registerParam("PM Amp", ParamType::Double, &pmAmp, &pmMin, &pmMax);
		registerParam("LPM Amp", ParamType::Double, &lpmAmp, &pmMin, &pmMax);
	}
	PhaseModAmp::PhaseModAmp(){}
	PhaseModAmp::PhaseModAmp(NoteProcPtr a, u_sp<PhaseSrc> freq) :NoteModulation(a), _freq(freq){}
	u_sp<PhaseSrc> PhaseModAmp::getPhaseSource()const{
		return this->_freq;
	}
	void PhaseModAmp::applyMod(Note & note){
		s_phase t=_freq->getPhase(note);
		if(_pm != nullptr){
			t+=(s_phase)(_pm->getAmp(note) * pmAmp);
		}
		if(_lpm != nullptr){
			t+=(s_phase)(_lpm->getAmp(note) * lpmAmp);
		}
		note.phaseSynth=t;
	}

	void PhaseModAmp::init(ChannelConfig & cfg){
		if(_freq == nullptr)_freq=NotePhase;
		_freq->init();
		if(_pm != nullptr)_pm->init(cfg);
		if(_lpm != nullptr)_lpm->init(cfg);
	}

	void PhaseModAmp::setPhaseSource(u_sp<PhaseSrc> freq){
		this->_freq=freq;
	}
	void PhaseModAmp::pm(NoteProcPtr pmSrc, double pmAmp){
		this->_pm=pmSrc;
		this->pmAmp=pmAmp;
	}
	void PhaseModAmp::lpm(NoteProcPtr lpmSrc, double lpmAmp){
		this->_lpm=lpmSrc;
		this->lpmAmp=pmAmp;
	}
	String PhaseModAmp::toString()const{
		return StringFormat::format("PhaseModAmp(%s)", _freq);
	}
}