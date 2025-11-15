#include "SineHarmonicWave.h"
#include "dsp/FastSin.h"
#include "lang/Math.h"
using namespace yzrilyzr_array;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	SineHarmonicWave::~SineHarmonicWave(){
		
	}
	SineHarmonicWave::SineHarmonicWave(const DoubleArray & freqAndAmp): SineHarmonicWave(nullptr, freqAndAmp){}
	SineHarmonicWave::SineHarmonicWave(std::shared_ptr<PhaseSrc> freq, const DoubleArray & freqAndAmp): Osc(freq){
		this->aa=freqAndAmp;
	}
	u_sample SineHarmonicWave::getAmp(Note &note){
		return a(getPhase(note)* Math::TAU)*note.velocitySynth;
	}
	u_sample SineHarmonicWave::a(double x){
		u_sample y=0;
		for(u_index i=0;i<aa.length;i++){
			if(aa[i]==0) continue;
			y+=aa[i]*fast_sin(x*(i+1.0),i);
		}
		return y;
	}
}