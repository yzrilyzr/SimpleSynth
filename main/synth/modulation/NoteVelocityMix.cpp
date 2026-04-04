#include "NoteVelocityMix.h"
#include "events/Note.h"
#include "dsp/DSP.h"
using namespace yzrilyzr_lang;
using namespace yzrilyzr_util;
namespace yzrilyzr_simplesynth{
	NoteVelocityMix::NoteVelocityMix() :NoteModulation(nullptr){}
	NoteVelocityMix::NoteVelocityMix(NoteProcPtr a, u_normal_01 mix) :NoteVelocityMix(a, 1, mix){}
	NoteVelocityMix::NoteVelocityMix(NoteProcPtr a, s_note_vel ovrd, u_normal_01 mix) :
		overrideKeyVel(ovrd), mix(mix), NoteModulation(a){}
	void NoteVelocityMix::onRegisterParam(){
		static double min=0, max=1;
		registerParam("OverrideVel", ParamType::Double, &overrideKeyVel, &min, &max);
		RegisterUtil::registerParamNormal01(*this, "Mix", &mix);
	}
	void NoteVelocityMix::applyMod(Note & note){
		note.velocitySynth=mix * overrideKeyVel + (1 - mix) * note.velocitySynth;
	}
	String NoteVelocityMix::toString() const{
		return StringFormat::format("NoteVelocityMix(%s,%.2f,%.2f)", a, overrideKeyVel, mix);
	}
}