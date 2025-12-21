#include "SineWaveTable.h"
#include "events/Note.h"
#include "events/ChannelConfig.h"
#include "interface/NoteTuning.h"
#include "dsp/FastSin.h"
using namespace yzrilyzr_array;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	SineWaveTable::~SineWaveTable(){
		
	}
	SineWaveTable::SineWaveTable(double baseFreq,const yzrilyzr_array::DoubleArray& freqAndAmp): SineWaveTable(nullptr, baseFreq, freqAndAmp){
	}
	SineWaveTable::SineWaveTable(u_sp<PhaseSrc> freq, double baseFreq,const yzrilyzr_array::DoubleArray &freqAndAmp): Osc(freq){
		this->aa=freqAndAmp;
		this->baseFreq=baseFreq;
	}
	u_sample SineWaveTable::getAmp(Note &note){
		return a(getPhase(note)* Math::TAU, note.cfg->tuning->getFrequencyByID(note.id))*note.velocitySynth;
	}
	u_sample SineWaveTable::a(double x, u_freq notef){
		u_sample y=0;
		for(u_index i=0;i<aa.length;i+=2){
			double f=aa[i];
			double a=aa[i+1];
			f=f/baseFreq;
			u_freq ff=f * notef;
			if(ff >20000) continue;
			y+=a*fast_sin(x*f,ff);
		}
		return y;
	}
}