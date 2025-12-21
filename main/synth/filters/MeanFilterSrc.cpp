#include "MeanFilterSrc.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	MeanFilterSrc::MeanFilterSrc(NoteProcPtr src, NoteProcPtr env, double envMulti){
		this->src=src;
		this->env=env;
		this->envMulti=envMulti;
	}
	MeanFilterSrc::MeanFilterSrc() :MeanFilterSrc(nullptr, nullptr, 0){
		static double gainMin=0, gainMax=500;
		registerParamSrc("Src", &src);
		registerParamSrc("Env", &env);
		registerParam("Multi", ParamType::Double, &envMulti, &gainMin, &gainMax);
	}
	u_sample MeanFilterSrc::getAmp(Note & note){
		u_sample src=this->src->getAmp(note);
		u_sample env=this->env->getAmp(note);
		MeanFilterSrcKeyData * data=getData(note);
		u_sample last=data->last;
		env*=envMulti;
		u_sample out=(src + env * last) / (1 + env);
		data->last=out;
		return out;
	}
	bool MeanFilterSrc::noMoreData(Note & note){
		bool nmd=this->src->noMoreData(note);
		if(nmd){
			this->env->noMoreData(note);
		}
		return nmd;
	}
	NoteProcPtr MeanFilterSrc::clone(){
		return mksp<MeanFilterSrc>(src->clone(), env->clone(), envMulti);
	}
	MeanFilterSrcKeyData * MeanFilterSrc::init(MeanFilterSrcKeyData * data, Note & note){
		if(data == nullptr) data=new MeanFilterSrcKeyData();
		data->last=0;
		return data;
	}
	String MeanFilterSrc::toString()const{
		return StringFormat::object2string("MeanFilterSrc", src, env, envMulti);
	}
}