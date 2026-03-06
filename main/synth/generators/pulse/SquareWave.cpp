#include "SquareWave.h"
#include "events/Note.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	u_sample SquareWave::getAmp(const Note & note){
		s_phase ft=getPhase(note);
		ft=ft - (int)ft;
		ft=ft > 0.5?1:-1;
		return ft * note.velocitySynth;
	}
	String SquareWave::toString() const{
		return StringFormat::object2string("SquareWave", getPhaseSource());
	}
}