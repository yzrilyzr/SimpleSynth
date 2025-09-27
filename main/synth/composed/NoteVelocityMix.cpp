#include "NoteVelocityMix.h"
#include "events/Note.h"

using namespace yzrilyzr_util;
namespace yzrilyzr_simplesynth{
	NoteVelocityMix::NoteVelocityMix():AmpUnaryComposition(nullptr){
		static double min=0, max=1;
		registerParam("OverrideVel",ParamType::Double, &overrideKeyVel,&min,&max);
		registerParamNormal01("Mix", &mix);
	}
	NoteVelocityMix::NoteVelocityMix(NoteProcPtr a, u_normal_01 mix):NoteVelocityMix(a,1,mix){}
	NoteVelocityMix::NoteVelocityMix(NoteProcPtr a, s_note_vel ovrd, u_normal_01 mix) :
		overrideKeyVel(ovrd), mix(mix),AmpUnaryComposition(a){}
	u_sample NoteVelocityMix::getAmp(Note & note){
		s_note_vel orig=note.velocitySynth;
		note.velocitySynth=mix * overrideKeyVel + (1 - mix) * orig;
		u_sample val=a->getAmp(note);
		note.velocitySynth=orig;
		return val;
	}
	std::string NoteVelocityMix::toString() const{
		return yzrilyzr_lang::StringFormat::format("NoteVelocityMix(%s,%.2f,%.2f)", a, overrideKeyVel,mix);
	}
}