#include "SineWave.h"
#include "lang/StringFormat.hpp"
#include "dsp/FastSin.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_dsp;
namespace yzrilyzr_simplesynth{
	u_sample SineWave::getAmp(const Note & note){
		return fast_sin(getPhase(note) *  Math::TAU, note.freqSynth) * note.velocitySynth;
	}
	String SineWave::toString()const{
		return StringFormat::object2string("SineWave", getPhaseSource());
	}
}