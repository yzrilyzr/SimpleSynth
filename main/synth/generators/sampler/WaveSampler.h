#pragma once
#include "events/Note.h"
#include "synth/generators/Osc.h"
#include "SimpleSynth.h"
#include "array/SampleProvider.h"
#include "array/ByteArray.h"
#include "array/FloatArray.h"
#include "array/SampleArray.h"
#include "array/ShortArray.h"
#include <cstdint>
#include <memory>
#include <string>
#include "interface/PhaseSrc.h"
#include "yzrutil.h"

namespace yzrilyzr_simplesynth{
	ECLASS(WaveSampler, public Osc){
	public:
	static constexpr int const LOOP_DISABLE=0;
	static constexpr int const LOOP_ONCE=1;
	static constexpr int const LOOP_LOOP=2;
	static constexpr int const LOOP_PING_PONG=3;
	private:
	std::shared_ptr<yzrilyzr_array::SampleProvider> sampleData;
	s_phase phaseMul=1;
	int32_t startLoopIndex=0;
	int32_t endLoopIndex=0;
	int loopType=0;
	size_t sampleOffset=0;
	size_t sampleLength=0;
	public:
	WaveSampler();
	WaveSampler(std::shared_ptr<PhaseSrc> freq, s_phase phaseMul, std::shared_ptr<yzrilyzr_array::SampleProvider> sampleData, size_t sampleOffset, size_t sampleLength,
				int32_t startLoopIndex, int32_t endLoopIndex, int loopType);
	u_sample getAmp(Note & note) override;
	u_sample getSample(yzrilyzr_array::SampleProvider & sample, int32_t index);
	bool noMoreData(Note & note) override;
	private:
	u_sample reSampleCubicSpline(yzrilyzr_array::SampleProvider & src, s_phase index);
	int32_t clampToLoop(int32_t index) const;
	std::string toString() const override;
	};
	EBCLASS(WaveSamplerBuilder){
		std::shared_ptr<PhaseSrc> freq=nullptr;
		s_phase phaseMul=1;
		std::shared_ptr<yzrilyzr_array::SampleProvider> sampleData=nullptr;
		size_t sampleOffset=0;
		size_t sampleLength=0;
		int32_t startLoopIndex=0;
		int32_t endLoopIndex=0;
		uint8_t loopType=WaveSampler::LOOP_DISABLE;
	public:
	WaveSamplerBuilder(){}
	WaveSamplerBuilder & freqSrc(std::shared_ptr<PhaseSrc> freq){
		this->freq=freq;
		return *this;
	}
	WaveSamplerBuilder & loop(int32_t startLoopIndex, int32_t endLoopIndex, uint8_t loopType){
		this->startLoopIndex=startLoopIndex;
		this->endLoopIndex=endLoopIndex;
		this->loopType=loopType;
		return *this;
	}
	WaveSamplerBuilder & sampleFreq(s_phase phaseMul){
		this->phaseMul=phaseMul;
		return *this;
	}
	/**
	* sampleDataFreq:采样数据的原始频率
	* sampleDataRate:采样数据的采样率
	*/
	WaveSamplerBuilder & sampleFreq(u_freq sampleDataFreq, u_sample_rate sampleDataRate){
		this->phaseMul=static_cast<s_phase>(sampleDataRate) / sampleDataFreq;
		return *this;
	}
	WaveSamplerBuilder & sampleOffsetLength(size_t sampleOffset, size_t sampleLength){
		this->sampleOffset=sampleOffset;
		this->sampleLength=sampleLength;
		return *this;
	}
	WaveSamplerBuilder & sample(std::shared_ptr<yzrilyzr_array::SampleProvider> sampleData){
		this->sampleData=sampleData;
		sampleOffsetLength(0, sampleData->length);
		return *this;
	}
	WaveSamplerBuilder & sample(std::shared_ptr<yzrilyzr_array::SampleArray> array){
		sample(std::make_shared<yzrilyzr_array::SampleArrayProvider>(array));
		return *this;
	}
	WaveSamplerBuilder & sample(std::shared_ptr<yzrilyzr_array::FloatArray> array){
		sample(std::make_shared<yzrilyzr_array::FloatArrayProvider>(array));
		return *this;
	}
	WaveSamplerBuilder & sample(std::shared_ptr<yzrilyzr_array::ShortArray> array){
		sample(std::make_shared<yzrilyzr_array::ShortArrayProvider>(array));
		return *this;
	}
	WaveSamplerBuilder & sample(std::shared_ptr<yzrilyzr_array::ByteArray> array){
		sample(std::make_shared<yzrilyzr_array::ByteArrayProvider>(array));
		return *this;
	}
	std::shared_ptr<WaveSampler> build(){
		return std::make_shared<WaveSampler>(freq, phaseMul, sampleData, sampleOffset, sampleLength,
											 startLoopIndex, endLoopIndex, loopType);
	}
	};
}