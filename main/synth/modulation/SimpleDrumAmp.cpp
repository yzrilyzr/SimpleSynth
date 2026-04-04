#include "SimpleDrumAmp.h"
#include "dsp/DSP.h"
#include "events/Note.h"
#include "interface/NoteTuning.h"
#include "synth/osc/sine/SineWave.h"
#include "synth/util/EnvUtil.h"
#include "util/Util.h"

using namespace yzrilyzr_util;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void SimpleDrumAmp::onRegisterParam(){
		NoteModulation::onRegisterParam();
		RegisterUtil::registerParamFreq(*this, "StartFreq", &startFreq);
		RegisterUtil::registerParamFreq(*this, "EndFreq", &endFreq);
		RegisterUtil::registerParamTime(*this, "Duration", &duration);
		static int min=0, max=1;
		registerParam("Mode", ParamType::Int, &mode, &min, &max);
		registerParamInterpolator("Curve", &curve);
	}
	SimpleDrumAmp::SimpleDrumAmp(u_freq startFreq, u_freq endFreq, u_time duration) : SimpleDrumAmp(mksp<SineWave>(), startFreq, endFreq, duration){}
	SimpleDrumAmp::SimpleDrumAmp(NoteProcPtr osc, u_freq startFreq, u_freq endFreq, u_time duration) : SimpleDrumAmp(osc, startFreq, endFreq, duration, MODE_FIXED, Pow(-5)){}
	SimpleDrumAmp::SimpleDrumAmp(NoteProcPtr osc, u_freq startFreq, u_freq endFreq, u_time duration, int mode, u_sp<Interpolator> curve) :NoteModulation(osc){
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
	void SimpleDrumAmp::applyMod(Note & note){
		u_time timePassed=note.passedTime;
		if(note.passedTime > duration) return;
		u_time x=Util::clamp01(timePassed / duration);
		x=curve->y(x);
		u_freq freq=0;
		if(mode == MODE_FIXED) freq=startFreq - x * (startFreq - endFreq);
		else if(mode == MODE_NOTE_RATIO){
			u_freq nf=note.cfg->tuning->getFrequencyByID(note.id);
			freq=startFreq * nf - x * (startFreq * nf - endFreq * nf);
		}
		SimpleDrumAmpKeyData * data=getData(note);
		note.freqSynth=freq;		
		data->phase+=freq * note.cfg->deltaTime;
		note.phaseSynth=data->phase;
	}
	bool SimpleDrumAmp::noMoreData(const Note & note)const{
		return note.passedTime > duration || note.closed(*note.cfg);
	}
	NoteProcPtr SimpleDrumAmp::clone(){
		return mksp<SimpleDrumAmp>(a->clone(), startFreq, endFreq, duration, mode, curve);
	}
	SimpleDrumAmpKeyData * SimpleDrumAmp::init(SimpleDrumAmpKeyData * data, const Note & note){
		if(data == nullptr) data=new SimpleDrumAmpKeyData();
		data->phase=0;
		return data;
	}
	String SimpleDrumAmp::toString()const{
		return StringFormat::object2string("SimpleDrumAmp", a, startFreq, endFreq, duration, mode, curve);
	}
}