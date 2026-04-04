#include "BiquadEnvFilterGroup.h"
#include "dsp/DSPGroupBuilder.h"
#include "dsp/IIR.h"
#include "dsp/IIRUtil.h"
#include "lang/StringFormat.hpp"
#include "util/Util.h"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_util;
namespace yzrilyzr_simplesynth{
	void BiquadEnvFilterGroup::onRegisterParam(){
		AmpUnaryComposition::onRegisterParam();
		static const char * enums[2]={"Chain", "Parallel"};
		registerParamEnum("Type", &type, enums, 2);
	}
	BiquadEnvFilterGroup::BiquadEnvFilterGroup() :AmpUnaryComposition(nullptr){

	}
	BiquadEnvFilterGroup::BiquadEnvFilterGroup(NoteProcPtr src, int type, std::vector<BiquadEnvFilterGroupConfig> filtersCfg) :
		AmpUnaryComposition(src), type(type), filtersCfg(filtersCfg){}

	u_sample BiquadEnvFilterGroup::getAmp(const Note & note){
		BiquadEnvFilterGroupKeyData & data=*getData(note);
		u_sample_rate sr=note.cfg->sampleRate;
		for(u_index i=0, j=filtersCfg.size();i < j;i++){
			BiquadEnvFilterGroupConfig & bcfg=filtersCfg[i];
			BiquadEnvFilterChange & change=data.changes[i];
			double fEnv=IIRUtil::limitFreq(sr, bcfg.freqEnv->getAmp(note));
			double qEnv=Util::clamp(bcfg.qEnv->getAmp(note), static_cast<u_sample>(0.001), static_cast<u_sample>(10000.0));
			double gEnv=Util::clamp(bcfg.gainEnv->getAmp(note), static_cast<u_sample>(-1000.0), static_cast<u_sample>(1000.0));
			if(fEnv != change.freq || qEnv != change.q || gEnv != change.gain){
				change.freq=fEnv;
				change.q=qEnv;
				change.gain=gEnv;
				IIR & iir=static_cast<IIR &>(data.filters->get(i));
				IIRUtil::RBJ_biquad(iir, RBJParams{bcfg.type, fEnv, sr, qEnv, gEnv});
			}
		}
		u_sample y=a->getAmp(note);
		y=data.filters->procDsp(y);
		return y;
	}
	NoteProcPtr BiquadEnvFilterGroup::clone(){
		return mksp< BiquadEnvFilterGroup>(a, type, filtersCfg);
	}
	void BiquadEnvFilterGroup::init(ChannelConfig & cfg){
		for(BiquadEnvFilterGroupConfig & bcfg : filtersCfg){
			bcfg.freqEnv->init(cfg);
			bcfg.qEnv->init(cfg);
			bcfg.gainEnv->init(cfg);
		}
	}
	BiquadEnvFilterGroupKeyData * BiquadEnvFilterGroup::init(BiquadEnvFilterGroupKeyData * data, const Note & note){
		if(data == nullptr)data=new BiquadEnvFilterGroupKeyData();
		if(data->filters == nullptr || data->filters->size() != filtersCfg.size()){
			DSPGroupBuilder builder;
			builder.begin(type);
			data->changes.clear();
			for(auto & cf : filtersCfg){
				auto iir=mksp<IIR>();
				IIRUtil::RBJ_biquad(*iir, RBJParams{FilterPassType::ALLPASS, 1000, note.cfg->sampleRate});
				builder.add(iir);
				data->changes.emplace_back(BiquadEnvFilterChange());
			}
			data->filters=builder.build();
		}

		data->filters->init(note.cfg->sampleRate);
		data->filters->resetMemory();
		return data;
	}
	String BiquadEnvFilterGroup::toString() const{
		return StringFormat::object2string("BiquadEnvFilterGroup", a, type, filtersCfg);
	}
}