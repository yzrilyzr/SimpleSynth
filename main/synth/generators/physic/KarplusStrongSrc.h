#pragma once
#include "SynthUtil.h"
#include "synth/generators/Osc.h"
#include "interface/NoteProcessor.h"
#include "events/ChannelConfig.h"
#include "array/SampleProvider.h"
#include "dsp/RingBuffer.h"
#include "events/Note.h"
#include "events/NoteData.hpp"
#include "interpolator/Interpolator.h"
#include <memory>
#include <string>
#include "interface/PhaseSrc.h"
#include "yzrutil.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(KarplusStrongSrcKeyData){
	public:
	yzrilyzr_dsp::RingBufferSample buffer;
	u_normal_01 alpha=0.5;
	};
	ECLASS(KarplusStrongSrc, public Osc, NoteData<KarplusStrongSrcKeyData>){
	public:
	u_normal_01 alpha=0.5;
	std::shared_ptr<yzrilyzr_interpolator::Interpolator> alphaInterpolator;
	std::shared_ptr<yzrilyzr_array::SampleProvider> burstSampleData;
	NoteProcPtr burst;
	u_freq constFreq=0;
	FixedRandom random;
	KarplusStrongSrc();
	KarplusStrongSrc(std::shared_ptr<PhaseSrc> freq) :Osc(freq){}
	KarplusStrongSrc(std::shared_ptr<PhaseSrc> freq, u_normal_01 alpha,
					 std::shared_ptr<yzrilyzr_interpolator::Interpolator> alphaInterpolator,
					 std::shared_ptr<yzrilyzr_array::SampleProvider> burstSampleData,
					 NoteProcPtr burst,
					 u_freq constFreq) :Osc(freq),
		alpha(alpha),
		alphaInterpolator(alphaInterpolator),
		burstSampleData(burstSampleData),
		burst(burst),
		constFreq(constFreq){}
	KarplusStrongSrcKeyData * init(KarplusStrongSrcKeyData * buffer, Note & note) override;
	private:
	u_freq getInitSetFreq(Note & note);
	public:
	void init(ChannelConfig & cfg) override;
	NoteProcPtr clone() override;
	u_sample getAmp(Note & note) override;
	yzrilyzr_lang::String toString() const override;
	private:
	u_freq getSetFreq(Note & note);
	};
	EBCLASS(KarplusStrongBuilder){
	private:
	std::shared_ptr<KarplusStrongSrc> ks;
	public:
	KarplusStrongBuilder(){
		ks=std::make_shared<KarplusStrongSrc>();
	}
	KarplusStrongBuilder & alpha(u_normal_01 alpha){
		ks->alpha=alpha;
		return *this;
	}
	KarplusStrongBuilder & alpha(std::shared_ptr<yzrilyzr_interpolator::Interpolator> alpha){
		ks->alphaInterpolator=alpha;
		return *this;
	}
	KarplusStrongBuilder & freqSrc(std::shared_ptr<PhaseSrc> freqSrc){
		ks->setPhaseSource(freqSrc);
		return *this;
	}
	KarplusStrongBuilder & burst(NoteProcPtr burst){
		ks->burst=burst;
		return *this;
	}
	KarplusStrongBuilder & burst(std::shared_ptr<yzrilyzr_array::SampleProvider> burst){
		ks->burstSampleData=burst;
		return *this;
	}
	KarplusStrongBuilder & constFreq(u_freq constFreq){
		ks->constFreq=constFreq;
		return *this;
	}
	NoteProcPtr build(){
		return std::dynamic_pointer_cast<NoteProcessor>(ks);
	}
	};
}