#include "BiquadFilterSrc.h"
#include "SimpleSynth.h"
#include "dsp/BiquadIIR.h"
#include "dsp/FilterPassType.h"
#include "dsp/IIRUtil.h"
#include "events/ChannelConfig.h"
#include "events/ChannelEvent.h"
#include "events/Note.h"
#include "interface/NoteProcessor.h"
#include "interface/NoteTuning.h"
#include "lang/Exception.h"
#include "lang/StringFormat.hpp"
#include "util/Util.h"
#include "yzrutil.h"
#include <memory>
#include <string>
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	BiquadFilterSrc::BiquadFilterSrc(NoteProcPtr src, NoteProcPtr freqEnv, NoteProcPtr qEnv, FilterPassType type) : src(src),
		freqEnv(freqEnv),
		qEnv(qEnv),
		type(type){}
	BiquadFilterSrc::BiquadFilterSrc(NoteProcPtr src, NoteProcPtr freqEnv, NoteProcPtr qEnv, NoteProcPtr gainEnv, FilterPassType type) : src(src),
		freqEnv(freqEnv),
		qEnv(qEnv),
		gainEnv(gainEnv),
		type(type){}
	BiquadFilterSrc::BiquadFilterSrc() :BiquadFilterSrc(nullptr, nullptr, nullptr, FilterPassType::LOWPASS){
		registerParamSrc("Src", &src);
		registerParamSrc("FreqEnv", &freqEnv);
		registerParamSrc("Q Env", &qEnv);
		registerParamSrc("Gain Env", &gainEnv);
		static const char * type_to_name[9]={"LOWPASS", "HIGHPASS", "BANDPASS", "BANDSTOP", "NOTCH", "LOWSHELF", "HIGHSHELF", "BELL", "ALLPASS"};
		registerParamEnum("Type", (int *)&type, type_to_name, 9);
	}
	void BiquadFilterSrc::init(ChannelConfig & cfg){
		this->cfg=&cfg;
		sampleRate=cfg.sampleRate;
		if(src == nullptr)throw NullPointerException("src == null");
		if(freqEnv == nullptr)throw NullPointerException("freqEnv == null");
		if(qEnv == nullptr)throw NullPointerException("qEnv == null");
		src->init(cfg);
		freqEnv->init(cfg);
		qEnv->init(cfg);
		if(gainEnv != nullptr)gainEnv->init(cfg);
	}
	u_sample BiquadFilterSrc::postProcess(u_sample output){
		return src->postProcess(output);
	}
	void BiquadFilterSrc::cc(ChannelConfig & cfg, ChannelControl & cc){
		src->cc(cfg, cc);
		freqEnv->cc(cfg, cc);
		qEnv->cc(cfg, cc);
		if(gainEnv != nullptr)gainEnv->cc(cfg,cc);
	}
	u_sample BiquadFilterSrc::getAmp(Note & note){
		s_note_id freqID=static_cast<s_note_id>(freqEnv->getAmp(note));
		u_freq freqValue=cfg->tuning->getFrequencyByID(freqID);
		freqValue=Util::clamp(freqValue, static_cast<u_freq>(0.0), static_cast<u_freq>(sampleRate / 2.1));
		u_sample qValue=qEnv->getAmp(note);
		u_sample gainValue=0;
		if(gainEnv != nullptr)gainValue=gainEnv->getAmp(note);
		u_sample y=src->getAmp(note);
		BiquadFilterSrcKeyData * data=getData(note);
		BiquadIIR & iir=data->filter;
		IIRUtil::biquad(iir, freqValue, sampleRate, qValue, type, gainValue);
		y=iir.procDsp(y);
		return y;
	}
	bool BiquadFilterSrc::noMoreData(Note & note){
		return src->noMoreData(note);
	}
	NoteProcPtr BiquadFilterSrc::clone(){
		return mksp<BiquadFilterSrc>(src->clone(), freqEnv->clone(), qEnv->clone(), type);
	}
	BiquadFilterSrcKeyData * BiquadFilterSrc::init(BiquadFilterSrcKeyData * data, Note & note){
		if(data == nullptr){
			data=new BiquadFilterSrcKeyData();
		}
		data->filter.init(sampleRate);
		data->filter.resetMemory();
		return data;
	}
	String BiquadFilterSrc::toString()const{
		return StringFormat::object2string("BiquadFilterSrc", src, freqEnv, qEnv, (int)type);
	}
}