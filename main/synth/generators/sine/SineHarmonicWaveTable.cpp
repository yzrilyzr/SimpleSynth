#include "SineHarmonicWaveTable.h"
#include "events/Note.h"
#include "interface/NoteTuning.h"
#include "interpolator/LineInterpolator.h"
#include "lang/Exception.h"
#include "dsp/FastSin.h"
#include "array/Arrays.h"
using namespace yzrilyzr_array;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_dsp;
namespace yzrilyzr_simplesynth{
	SineHarmonicWaveTable::~SineHarmonicWaveTable(){
		
	}
	SineHarmonicWaveTable::SineHarmonicWaveTable(const Array<DoubleArray> &aa) : SineHarmonicWaveTable(nullptr, aa){}
	SineHarmonicWaveTable::SineHarmonicWaveTable(std::shared_ptr<PhaseSrc> freq,const Array<DoubleArray> &naa) : Osc(freq){
		this->aa=aa;
		this->interpolator=std::make_shared<LineInterpolator>();
	}
	DoubleArray SineHarmonicWaveTable::dBToAmp(double gain,const DoubleArray& line){
		DoubleArray ampArr=Arrays::copyOf(line, line.length);
		for(u_index i=3;i < line.length;i++){
			ampArr[i]=gain * pow(10, line[i] / 20.0);
		}
		return ampArr;
	}
	u_sample SineHarmonicWaveTable::getAmp(Note & note){
		return a(note, getPhase(note) * _2PI) * note.velocitySynth;
	}
	u_sample SineHarmonicWaveTable::a(Note & note, double x){
		u_sample y=0;
		for(u_index harmonicOrder=0;harmonicOrder < aa.length;harmonicOrder++){
			auto & ampLine=aa[harmonicOrder];
			y+=getInterpolation(note, ampLine) * fast_sin(x * (harmonicOrder + 1.0), harmonicOrder);
		}
		return y;
	}
	double SineHarmonicWaveTable::getInterpolation(Note & note,const DoubleArray & ampLine){
		if(ampLine.length == 1) return ampLine[0];
		if(ampLine.length < 5) throw Exception("WaveTable err");
		u_index points=ampLine.length - 3;
		auto tuning=note.cfg->tuning;
		s_note_id id1=tuning->getIDByFrequency(ampLine[1]);
		s_note_id id2=tuning->getIDByFrequency(ampLine[2]);
		double xAtAllRange=(static_cast<s_note_id>(note.id) - id1) / (id2 - id1);
		if(xAtAllRange < 0 || xAtAllRange>1) return 0;//超出范围
		int32_t index=(int32_t)((points - 1) * xAtAllRange);
		if(index < 0 || index + 4 >= ampLine.length){
			throw Exception();
		}
		double y1=ampLine[index + 3];
		double y2=ampLine[index + 4];
		double delta=y2 - y1;
		double xWidthBetweenPoints=1.0f / points;
		double interpolationX=(xAtAllRange - index * xWidthBetweenPoints) / xWidthBetweenPoints;
		return y1 + delta * interpolator->y(interpolationX);
	}
}