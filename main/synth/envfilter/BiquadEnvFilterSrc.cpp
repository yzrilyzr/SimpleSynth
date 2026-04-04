#include "BiquadEnvFilterSrc.h"
#include "SimpleSynth.h"
#include "dsp/IIR.h"
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
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void BiquadEnvFilterSrc::onRegisterParam(){
		RegisterUtil::registerParamSrc(*this, "Src", &src);
		RegisterUtil::registerParamSrc(*this, "FreqEnv", &freqEnv);
		RegisterUtil::registerParamSrc(*this, "Q Env", &qEnv);
		RegisterUtil::registerParamSrc(*this, "Gain Env", &gainEnv);
		static const char * type_to_name[9]={"LOWPASS", "HIGHPASS", "BANDPASS", "BANDSTOP", "NOTCH", "LOWSHELF", "HIGHSHELF", "BELL", "ALLPASS"};
		registerParamEnum("Type", (int *)&type, type_to_name, 9);
	}
	BiquadEnvFilterSrc::BiquadEnvFilterSrc(NoteProcPtr src, NoteProcPtr freqEnv, NoteProcPtr qEnv, FilterPassType type) : src(src),
		freqEnv(freqEnv),
		qEnv(qEnv),
		type(type){}
	BiquadEnvFilterSrc::BiquadEnvFilterSrc(NoteProcPtr src, NoteProcPtr freqEnv, NoteProcPtr qEnv, NoteProcPtr gainEnv, FilterPassType type) : src(src),
		freqEnv(freqEnv),
		qEnv(qEnv),
		gainEnv(gainEnv),
		type(type){}
	BiquadEnvFilterSrc::BiquadEnvFilterSrc() :BiquadEnvFilterSrc(nullptr, nullptr, nullptr, FilterPassType::LOWPASS){

	}
	void BiquadEnvFilterSrc::init(ChannelConfig & cfg){
		if(src == nullptr)throw NullPointerException("src == null");
		if(freqEnv == nullptr)throw NullPointerException("freqEnv == null");
		if(qEnv == nullptr)throw NullPointerException("qEnv == null");
		src->init(cfg);
		freqEnv->init(cfg);
		qEnv->init(cfg);
		if(gainEnv != nullptr)gainEnv->init(cfg);
	}
	void BiquadEnvFilterSrc::postProcess(u_sample * input, u_index length){
		src->postProcess(input, length);
	}
	void BiquadEnvFilterSrc::cc(ChannelConfig & cfg, ChannelControl & cc){
		src->cc(cfg, cc);
		freqEnv->cc(cfg, cc);
		qEnv->cc(cfg, cc);
		if(gainEnv != nullptr)gainEnv->cc(cfg, cc);
	}
	u_sample BiquadEnvFilterSrc::getAmp(const Note & note){
		s_note_id freqID=static_cast<s_note_id>(freqEnv->getAmp(note));
		u_freq freqValue=note.cfg->tuning->getFrequencyByID(freqID);
		freqValue=Util::clamp(freqValue, static_cast<u_freq>(0.0), static_cast<u_freq>(note.cfg->sampleRate / 2.1));
		u_sample qValue=qEnv->getAmp(note);
		u_sample gainValue=0;
		if(gainEnv != nullptr)gainValue=gainEnv->getAmp(note);
		u_sample y=src->getAmp(note);
		BiquadEnvFilterSrcKeyData * data=getData(note);
		IIR & iir=data->filter;
		IIRUtil::RBJ_biquad(iir, RBJParams{type, freqValue, note.cfg->sampleRate, qValue, gainValue});
		y=iir.procDsp(y);
		return y;
	}
	bool BiquadEnvFilterSrc::noMoreData(const Note & note)const{
		return src->noMoreData(note);
	}
	NoteProcPtr BiquadEnvFilterSrc::clone(){
		return mksp<BiquadEnvFilterSrc>(src->clone(), freqEnv->clone(), qEnv->clone(), type);
	}
	BiquadEnvFilterSrcKeyData * BiquadEnvFilterSrc::init(BiquadEnvFilterSrcKeyData * data, const Note & note){
		if(data == nullptr){
			data=new BiquadEnvFilterSrcKeyData();
		}
		IIRUtil::RBJ_biquad(data->filter, RBJParams{FilterPassType::LOWPASS, 20, note.cfg->sampleRate});
		data->filter.init(note.cfg->sampleRate);
		data->filter.resetMemory();
		return data;
	}
	String BiquadEnvFilterSrc::toString()const{
		return StringFormat::object2string("BiquadEnvFilterSrc", src, freqEnv, qEnv, (int)type);
	}
}