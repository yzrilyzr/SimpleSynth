#include "SimpleDrumAmp.h"
#include "synth/generators/sine/SineWave.h"
#include "events/Note.h"
#include "util/Util.h"
#include "interface/NoteTuning.h"
#include "synth/envelopers/EnvUtil.h"

using namespace yzrilyzr_util;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void SimpleDrumAmp::onRegisterParam(){
		RegisterUtil::registerParamFreq(*this, "StartFreq", &startFreq);
		RegisterUtil::registerParamFreq(*this, "EndFreq", &endFreq);
		RegisterUtil::registerParamTime(*this, "Duration", &duration);
		static int min=0, max=1;
		registerParam("Mode", ParamType::Int, &mode, &min, &max);
		RegisterUtil::registerParamSrc(*this, "Src", &src);
		registerParamInterpolator("Curve", &curve);
	}
	SimpleDrumAmp::SimpleDrumAmp(u_freq startFreq, u_freq endFreq, u_time duration) : SimpleDrumAmp(mksp<SineWave>(), startFreq, endFreq, duration){}
	SimpleDrumAmp::SimpleDrumAmp(NoteProcPtr osc, u_freq startFreq, u_freq endFreq, u_time duration) : SimpleDrumAmp(osc, startFreq, endFreq, duration, MODE_FIXED, Pow(-5)){}
	SimpleDrumAmp::SimpleDrumAmp(NoteProcPtr osc, u_freq startFreq, u_freq endFreq, u_time duration, int mode, u_sp<Interpolator> curve){
		this->src=osc;
		this->mode=mode;
		this->startFreq=startFreq;
		this->endFreq=endFreq;
		this->duration=duration;
		this->curve=curve;
	}
	SimpleDrumAmp::SimpleDrumAmp(NoteProcPtr osc, u_freq startFreq, u_freq endFreq, u_time duration, u_sp<Interpolator> curve) : SimpleDrumAmp(osc,
																																						  startFreq,
																																						  endFreq,
																																						  duration,
																																						  MODE_FIXED,
																																						  curve){}
	SimpleDrumAmp::SimpleDrumAmp() : SimpleDrumAmp(mksp<SineWave>(), 200, 50, 0.3){}
	u_sample SimpleDrumAmp::getAmp(const Note & note){
		u_time timePassed=note.passedTime;
		if(note.passedTime > duration) return 0;
		u_time x=Util::clamp01(timePassed / duration);
		x=curve->y(x);
		u_freq freq=0;
		if(mode == MODE_FIXED) freq=startFreq - x * (startFreq - endFreq);
		else if(mode == MODE_NOTE_RATIO){
			u_freq nf=note.cfg->tuning->getFrequencyByID(note.id);
			freq=startFreq * nf - x * (startFreq * nf - endFreq * nf);
		}
		SimpleDrumAmpKeyData * data=getData(note);
		data->freqTimeSynth+=freq * note.cfg->deltaTime;
		//
		s_phase origP=note.phaseSynth;
		u_freq origF=note.freqSynth;
		//
		auto & mut_note=const_cast<Note &>(note);
		mut_note.freqSynth=freq;
		mut_note.phaseSynth=data->freqTimeSynth;
		//
		u_sample s=src->getAmp(mut_note);
		//
		mut_note.freqSynth=origF;
		mut_note.phaseSynth=origP;
		return s * mut_note.velocitySynth;
	}
	void SimpleDrumAmp::init(ChannelConfig & cfg){
		NoteProcessor::init(cfg);
		if(src == nullptr)throw NullPointerException("src == null");
		src->init(cfg);
	}
	bool SimpleDrumAmp::noMoreData(const Note & note){
		return note.passedTime > duration || note.closed(*note.cfg);
	}
	NoteProcPtr SimpleDrumAmp::clone(){
		return mksp<SimpleDrumAmp>(src->clone(), startFreq, endFreq, duration, mode, curve);
	}
	SimpleDrumAmpKeyData * SimpleDrumAmp::init(SimpleDrumAmpKeyData * data, const Note & note){
		if(data == nullptr) data=new SimpleDrumAmpKeyData();
		data->freqTimeSynth=0;
		return data;
	}
	String SimpleDrumAmp::toString()const{
		return StringFormat::object2string("SimpleDrumAmp", src, startFreq, endFreq, duration, mode, curve);
	}
}