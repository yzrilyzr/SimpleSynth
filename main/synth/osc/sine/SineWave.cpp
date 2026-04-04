#include "SineWave.h"
#include "lang/StringFormat.hpp"
#include "dsp/FastSin.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_dsp;
namespace yzrilyzr_simplesynth{
	u_sample SineWave::getAmp(const Note & note){
		return fast_sin(note.phaseSynth * Math::TAU, note.freqSynth) * note.velocitySynth;
	}
	void SineWave::getAmpBlock(const Note * note, u_sample * output, u_index length){
		for(u_index i=0;i < length;i++){
			output[i]=fast_sin(note[i].phaseSynth * Math::TAU, note[i].freqSynth) * note[i].velocitySynth;
		}
	}
	String SineWave::toString()const{
		return StringFormat::object2string("SineWave");
	}
}