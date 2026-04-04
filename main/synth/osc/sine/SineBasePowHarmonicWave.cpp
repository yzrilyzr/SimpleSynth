#include "SineBasePowHarmonicWave.h"
#include "dsp/FastSin.h"
#include "lang/Math.h"
using namespace yzrilyzr_array;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	SineBasePowHarmonicWave::~SineBasePowHarmonicWave(){}
	SineBasePowHarmonicWave::SineBasePowHarmonicWave(const DoubleArray & freqAndAmp){
		this->aa=freqAndAmp;
	}
	u_sample SineBasePowHarmonicWave::getAmp(const Note & note){
		return a(note.phaseSynth * Math::TAU, note.id) * note.velocitySynth;
	}
	u_sample SineBasePowHarmonicWave::a(double x, int id){
		u_sample y=0;
		for(u_index i=0;i < aa.length;i++){
			if(aa[i] == 0) continue;
			if(id + i * 12 > 127) continue;
			double ii=pow(2, i);
			y+=aa[i] * fast_sin(x * ii, ii);
		}
		return y;
	}
}