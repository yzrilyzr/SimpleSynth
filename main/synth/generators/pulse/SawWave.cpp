#include "SawWave.h"
#include "events/Note.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	u_sample SawWave::getAmp(const Note & note){
		s_phase ft=getPhase(note);
		ft=ft - (int)ft;
		ft=ft * 2.0 - 1.0;
		return ft * note.velocitySynth;
	}
	String SawWave::toString()const{
		return StringFormat::object2string("SawWave", getPhaseSource());
	}
}