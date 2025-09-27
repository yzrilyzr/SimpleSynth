#pragma once
#include "SynthUtil.h"
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "synth/generators/Osc.h"
#include "synth/composed/AmpUnaryComposition.h"
#include "synth/source/AmplitudeSources.h"
#include "dsp/BiquadIIR.h"
#include "dsp/FilterPassType.h"
#include "dsp/DSPGroup.h"
#include <memory>
#include <vector>
namespace yzrilyzr_simplesynth{
	struct BiquadEnvFilterGroupConfig{
		yzrilyzr_dsp::FilterPassType type;
		NoteProcPtr freqEnv;
		NoteProcPtr qEnv;
		NoteProcPtr gainEnv;
		BiquadEnvFilterGroupConfig(yzrilyzr_dsp::FilterPassType t, NoteProcPtr freq, NoteProcPtr q, NoteProcPtr gain)
			: type(t), freqEnv(std::move(freq)), qEnv(std::move(q)), gainEnv(std::move(gain)){}

		BiquadEnvFilterGroupConfig(yzrilyzr_dsp::FilterPassType t, NoteProcPtr freq, double q, double gain)
			: type(t), freqEnv(std::move(freq)), qEnv(ConstAmp(q)), gainEnv(ConstAmp(gain)){}

		BiquadEnvFilterGroupConfig(yzrilyzr_dsp::FilterPassType t, double freq, double q, double gain)
			: type(t), freqEnv(ConstAmp(freq)), qEnv(ConstAmp(q)), gainEnv(ConstAmp(gain)){}
	};
	struct BiquadEnvFilterChange{
		double freq=0;
		double q=0;
		double gain=0;
	};
	EBCLASS(BiquadEnvFilterGroupKeyData){
		public:
		std::shared_ptr<yzrilyzr_dsp::DSPGroup> filters;
		std::vector< BiquadEnvFilterChange> changes;
	};
	ECLASS(BiquadEnvFilterGroup, public AmpUnaryComposition, NoteData<BiquadEnvFilterGroupKeyData>){
		private:
		std::vector<BiquadEnvFilterGroupConfig> filtersCfg;
		int type;
		public:
		BiquadEnvFilterGroup();
		BiquadEnvFilterGroup(NoteProcPtr src, int type, std::vector<BiquadEnvFilterGroupConfig> filtersCfg);
		void init(ChannelConfig & cfg)override;
		u_sample getAmp(Note & note) override;
		NoteProcPtr clone() override;
		BiquadEnvFilterGroupKeyData * init(BiquadEnvFilterGroupKeyData * data, Note & note) override;
		std::string toString() const override;
	};
}