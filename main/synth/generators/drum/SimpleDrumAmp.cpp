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
	SimpleDrumAmp::SimpleDrumAmp(u_freq startFreq, u_freq endFreq, u_time duration) : SimpleDrumAmp(std::make_shared<SineWave>(), startFreq, endFreq, duration){}
	SimpleDrumAmp::SimpleDrumAmp(NoteProcPtr osc, u_freq startFreq, u_freq endFreq, u_time duration) : SimpleDrumAmp(osc, startFreq, endFreq, duration, MODE_FIXED, Pow(-5)){}
	SimpleDrumAmp::SimpleDrumAmp(NoteProcPtr osc, u_freq startFreq, u_freq endFreq, u_time duration, int mode, std::shared_ptr<Interpolator> curve){
		this->src=osc;
		this->mode=mode;
		this->startFreq=startFreq;
		this->endFreq=endFreq;
		this->duration=duration;
		this->curve=curve;
	}
	SimpleDrumAmp::SimpleDrumAmp(NoteProcPtr osc, u_freq startFreq, u_freq endFreq, u_time duration, std::shared_ptr<Interpolator> curve) : SimpleDrumAmp(osc,
																																						  startFreq,
																																						  endFreq,
																																						  duration,
																																						  MODE_FIXED,
																																						  curve){}
	SimpleDrumAmp::SimpleDrumAmp() : SimpleDrumAmp(std::make_shared<SineWave>(), 200, 50, 0.3){
		registerParamFreq("StartFreq", &startFreq);
		registerParamFreq("EndFreq", &endFreq);
		registerParamTime("Duration", &duration);
		static int min=0, max=1;
		registerParam("Mode", ParamType::Int, &mode, &min, &max);
		registerParamSrc("Src", &src);
		registerParamInterpolator("Curve", &curve);
	}
	u_sample SimpleDrumAmp::getAmp(Note & note){
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
		note.freqSynth=freq;
		note.phaseSynth=data->freqTimeSynth;
		//
		u_sample s=src->getAmp(note);
		//
		note.freqSynth=origF;
		note.phaseSynth=origP;
		return s * note.velocitySynth;
	}
	void SimpleDrumAmp::init(ChannelConfig & cfg){
		NoteProcessor::init(cfg);
		if(src == nullptr)throw NullPointerException("src == null");
		src->init(cfg);
	}
	bool SimpleDrumAmp::noMoreData(Note & note){
		return note.passedTime > duration || note.closed(*note.cfg);
	}
	NoteProcPtr SimpleDrumAmp::clone(){
		return std::make_shared<SimpleDrumAmp>(src->clone(), startFreq, endFreq, duration, mode, curve);
	}
	SimpleDrumAmpKeyData * SimpleDrumAmp::init(SimpleDrumAmpKeyData * data, Note & note){
		if(data == nullptr) data=new SimpleDrumAmpKeyData();
		data->freqTimeSynth=0;
		return data;
	}
	String SimpleDrumAmp::toString()const{
		return StringFormat::object2string("SimpleDrumAmp", src, startFreq, endFreq, duration, mode, curve);
	}
}