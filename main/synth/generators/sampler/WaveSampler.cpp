#include "WaveSampler.h"
#include "SimpleSynth.h"
#include "array/SampleProvider.h"
#include <cstdint>
#include "events/Note.h"
#include <memory>
#include <string>
#include "synth/generators/Osc.h"
#include "interface/PhaseSrc.h"
#include "lang/StringFormat.hpp"
#include "dsp/InterpolateFunction.h"
#include "yzrutil.h"
using namespace yzrilyzr_array;
using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void WaveSampler::onRegisterParam(){
		Osc::onRegisterParam();
		static s_phase phase_min=0, phase_max=100;
		static int32_t loop_min=0, loop_max=1000000;
		static u_index len_min=0, len_max=1000000;
		RegisterUtil::registerParamSampleData(*this, "SampleData", &sampleData);
		registerParam("PhaseMul", ParamType::Double, &phaseMul, &phase_min, &phase_max);
		registerParam("SampleOffset", ParamType::Size, &sampleOffset, &len_min, &len_max);
		registerParam("SampleLength", ParamType::Size, &sampleLength, &len_min, &len_max);
		registerParam("LoopStart", ParamType::Int, &startLoopIndex, &loop_min, &loop_max);
		registerParam("LoopEnd", ParamType::Int, &endLoopIndex, &loop_min, &loop_max);
		static const char * enumNames[]={"LOOP_DISABLE", "LOOP_ONCE", "LOOP_LOOP", "LOOP_PING_PONG"};
		registerParamEnum("LoopType", &loopType, enumNames, 4);
	}
	WaveSampler::WaveSampler() :Osc(nullptr){}

	WaveSampler::WaveSampler(u_sp<PhaseSrc> freq, s_phase phaseMul, u_sp<SampleProvider> sampleData, u_index sampleOffset, u_index sampleLength,
							 int32_t startLoopIndex, int32_t endLoopIndex, int loopType) :
		Osc(freq),
		sampleData(sampleData),
		phaseMul(phaseMul),
		startLoopIndex(startLoopIndex),
		endLoopIndex(endLoopIndex),
		loopType(loopType),
		sampleOffset(sampleOffset),
		sampleLength(sampleLength){
		if(endLoopIndex < startLoopIndex || endLoopIndex - startLoopIndex == 0){
			this->loopType=LOOP_DISABLE;
		}
	}
	u_sample WaveSampler::getAmp(const Note & note){
		if(sampleData == nullptr) return 0;
		s_phase noteIndex=getPhase(note) * phaseMul;
		u_sample d=reSampleCubicSpline(*sampleData, static_cast<double>(noteIndex) + sampleOffset);
		return d * note.velocitySynth;
	}
	u_sample WaveSampler::getSample(SampleProvider & sample, int32_t index){
		index=clampToLoop(index);
		if(index < 0 || index >= sample.length) return 0;
		return sample[index];
	}
	bool WaveSampler::noMoreData(const Note & note){
		if(sampleData == nullptr) return true;
		switch(loopType){
			case LOOP_LOOP:
			case LOOP_PING_PONG:
				return note.closed(*note.cfg);
			case LOOP_DISABLE:
			case LOOP_ONCE:
			default:
				s_phase index=getPhase(note) * phaseMul;
				return index > sampleLength || note.closed(*note.cfg);
		}
	}
	u_sample WaveSampler::reSampleCubicSpline(SampleProvider & src, s_phase index){
		int32_t i=(int32_t)index;
		s_phase y0=getSample(src, i - 1);
		s_phase y1=getSample(src, i);
		s_phase y2=getSample(src, i + 1);
		s_phase y3=getSample(src, i + 2);
		return InterpolateFunction::cubicSpline(index, i, y0, y1, y2, y3);
	}
	int32_t WaveSampler::clampToLoop(int32_t index) const{
		switch(loopType){
			case LOOP_LOOP:
				if(index > endLoopIndex){
					index-=startLoopIndex;
					index%=endLoopIndex - startLoopIndex;
					return startLoopIndex + index;
				}
				return index;
			case LOOP_PING_PONG:
				if(index > endLoopIndex){
					index-=startLoopIndex;
					int32_t loopLength=endLoopIndex - startLoopIndex;
					index%=2 * loopLength;
					if(index > loopLength){
						index=2 * loopLength - index;
					}
					return startLoopIndex + index;
				}
				return index;
			//case LOOP_DISABLE:
			//case LOOP_ONCE:
			default:
				return index;
		}
	}
	String WaveSampler::toString() const{
		return StringFormat::object2string("WaveSampler", getPhaseSource(),
										   phaseMul,
										   sampleData,
										   sampleOffset,
										   sampleLength,
										   startLoopIndex,
										   endLoopIndex,
										   loopType);
	}
}