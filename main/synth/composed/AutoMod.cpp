#include "AutoMod.h"
#include "synth/generators/sine/SineWave.h"
#include "events/Note.h"
#include "dsp/DSP.h"
#include "util/Util.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void AutoMod::onRegisterParam(){
		NoteModulation::onRegisterParam();
		RegisterUtil::registerParamFreq(*this, "Freq", &modRate);
		RegisterUtil::registerParamNormal01(*this, "FreqDepth", &modFreqDepth);
		RegisterUtil::registerParamNormal01(*this, "AmpDepth", &modAmpDepth);
		RegisterUtil::registerParamTime(*this, "Delay", &modDelay);
		RegisterUtil::registerParamSrc(*this,"Shape", &modShape);
	}
	AutoMod::AutoMod(NoteProcPtr a, u_normal_01 modFreqDepth, u_normal_01 modAmpDepth, u_freq modRate, u_time modDelay, NoteProcPtr modShape) :
		NoteModulation(a),
		modFreqDepth(modFreqDepth),
		modAmpDepth(modAmpDepth),
		modRate(modRate),
		modDelay(modDelay),
		modShape(modShape){}
	void AutoMod::applyMod(Note & note){
		if(modShape == nullptr){
			modShape=mksp<SineWave>();
		}
		u_time passedTime=note.passedTime;
		u_sample mod=0;
		if(passedTime > modDelay || isMod){
			static thread_local Note tmp;
			tmp.set(note);
			tmp.uniqueID=note.uniqueID;
			tmp.velocitySynth=1;
			u_freq minRate=modRate - 1;
			u_freq maxRate=modRate + 1;
			s_note_id_i idMin=0, idMax=127;
			u_freq rate=Util::linearMap(idMin, idMax, minRate, maxRate, Util::clamp(note.id, idMin, idMax));
			rate=Util::clamp(rate, 1.0, 15.0);
			tmp.phaseSynth=(passedTime - modDelay) * rate;
			mod=modShape->getAmp(tmp);
			isMod=true;
		} else mod=0;
		s_phase deltaMod=static_cast<s_phase>(mod * note.freqSynth * note.cfg->deltaTime * modFreqDepth);
		note.phaseSynth+=deltaMod*50;
		note.velocitySynth+=note.velocitySynth * mod * modAmpDepth;		
	}
	
	NoteProcPtr AutoMod::clone(){
		return mksp<AutoMod>(a->clone(), modFreqDepth, modAmpDepth, modRate, modDelay, modShape->clone());
	}
	bool AutoMod::noMoreData(const Note & note){
		bool nmd=a->noMoreData(note);
		if(nmd){
			u_time t=note.passedTime;
			if(t < modDelay){
				isMod=false;
			}
		}
		return nmd;
	}
	String AutoMod::toString() const{
		return StringFormat::object2string("AutoMod", a, modFreqDepth, modAmpDepth, modRate, modDelay, modShape);
	}
}