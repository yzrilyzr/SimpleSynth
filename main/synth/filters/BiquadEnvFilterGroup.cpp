#include "BiquadEnvFilterGroup.h"
#include "dsp/DSPGroupBuilder.h"
#include "dsp/IIRUtil.h"
#include "lang/StringFormat.hpp"
#include "util/Util.h"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_util;
namespace yzrilyzr_simplesynth{
	BiquadEnvFilterGroup::BiquadEnvFilterGroup() :AmpUnaryComposition(nullptr){
		static const char * enums[2]={"Chain", "Parallel"};
		registerParamEnum("Type", &type, enums, 2);
	}
	BiquadEnvFilterGroup::BiquadEnvFilterGroup(NoteProcPtr src, int type, std::vector<BiquadEnvFilterGroupConfig> filtersCfg) :
		AmpUnaryComposition(src), type(type), filtersCfg(filtersCfg){}

	u_sample BiquadEnvFilterGroup::getAmp(Note & note){
		BiquadEnvFilterGroupKeyData & data=*getData(note);
		u_sample_rate sr=note.cfg->sampleRate;
		for(u_index i=0, j=filtersCfg.size();i < j;i++){
			BiquadEnvFilterGroupConfig & bcfg=filtersCfg[i];
			BiquadEnvFilterChange & change=data.changes[i];
			double fEnv=IIRUtil::limitFreq(sr, bcfg.freqEnv->getAmp(note));
			double qEnv=Util::clamp(bcfg.qEnv->getAmp(note), 0.001, 10000.0);
			double gEnv=Util::clamp(bcfg.gainEnv->getAmp(note), -1000.0, 1000.0);
			if(fEnv != change.freq || qEnv != change.q || gEnv != change.gain){
				change.freq=fEnv;
				change.q=qEnv;
				change.gain=gEnv;
				BiquadIIR & iir=static_cast<BiquadIIR &>(data.filters->get(i));
				IIRUtil::biquad(iir, fEnv, sr, qEnv, bcfg.type, gEnv);
			}
		}
		u_sample y=a->getAmp(note);
		y=data.filters->procDsp(y);
		return y;
	}
	NoteProcPtr BiquadEnvFilterGroup::clone(){
		return std::make_shared< BiquadEnvFilterGroup>(a, type, filtersCfg);
	}
	void BiquadEnvFilterGroup::init(ChannelConfig & cfg){
		for(BiquadEnvFilterGroupConfig & bcfg : filtersCfg){
			bcfg.freqEnv->init(cfg);
			bcfg.qEnv->init(cfg);
			bcfg.gainEnv->init(cfg);
		}
	}
	BiquadEnvFilterGroupKeyData * BiquadEnvFilterGroup::init(BiquadEnvFilterGroupKeyData * data, Note & note){
		if(data == nullptr)data=new BiquadEnvFilterGroupKeyData();
		if(data->filters == nullptr || data->filters->size() != filtersCfg.size()){
			DSPGroupBuilder builder;
			builder.begin(type);
			data->changes.clear();
			for(auto & cf : filtersCfg){
				builder.add(std::make_shared<BiquadIIR>());
				data->changes.emplace_back(BiquadEnvFilterChange());
			}
			data->filters=builder.build();
		}
		data->filters->resetMemory();
		return data;
	}
	String BiquadEnvFilterGroup::toString() const{
		return StringFormat::object2string("BiquadEnvFilterGroup", a, type, filtersCfg);
	}
}