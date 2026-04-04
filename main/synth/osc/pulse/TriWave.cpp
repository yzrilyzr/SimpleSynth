#include "TriWave.h"
#include "events/Note.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	u_sample TriWave::getAmp(const Note & note){
		s_phase ft=note.phaseSynth;
		ft=ft - (int)ft;
		ft=ft > 0.5?((1 - ft) * 4 - 1):(ft * 4 - 1);
		return ft * note.velocitySynth;
	}
	String TriWave::toString() const{
		return StringFormat::object2string("TriWave");
	}
}