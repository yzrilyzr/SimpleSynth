#include "AutoMod.h"
#include "synth/generators/sine/SineWave.h"
#include "events/Note.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	AutoMod::AutoMod(NoteProcPtr a, u_normal_01 modFreqDepth, u_normal_01 modAmpDepth, u_freq modRate, u_time modDelay, NoteProcPtr modShape) :
		AmpUnaryComposition(a),
		modFreqDepth(modFreqDepth),
		modAmpDepth(modAmpDepth),
		modRate(modRate),
		modDelay(modDelay),
		modShape(modShape){}
	u_sample AutoMod::getAmp(Note & note){
		if(modShape == nullptr){
			modShape=std::make_shared<SineWave>();
		}
		u_time passedTime=note.passedTime;
		u_sample mod=0;
		if(passedTime > modDelay || isMod){
			Note tmp(note.uniqueID);
			tmp.set(note);
			tmp.velocitySynth=1;
			u_freq minRate=modRate-3;
			u_freq maxRate=modRate+3;
			s_note_id_i idMin=0, idMax=127;
			u_freq rate=Util::linearMap(idMin, idMax, minRate, maxRate, Util::clamp(note.id, idMin, idMax));
			rate=Util::clamp(rate, 1.0, 15.0);
			tmp.phaseSynth=(passedTime - modDelay) *rate;
			mod=modShape->getAmp(tmp);
			isMod=true;
		} else mod=0;
		note.phaseSynth+=static_cast<s_phase>(mod * note.freqSynth * note.cfg->deltaTime * modFreqDepth * 0.05017);
		note.velocitySynth+=note.velocitySynth * mod * modAmpDepth;
		return a->getAmp(note);
	}
	NoteProcPtr AutoMod::clone(){
		return std::make_shared<AutoMod>(a->clone(), modFreqDepth, modAmpDepth, modRate, modDelay, modShape->clone());
	}
	bool AutoMod::noMoreData(Note & note){
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