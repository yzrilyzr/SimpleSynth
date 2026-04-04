#include "synth/composed/AmpBinaryComposition.h"
#include "lang/StringFormat.hpp"
#include "lang/Exception.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void AmpBinaryComposition::onRegisterParam(){
		RegisterUtil::registerParamSrc(*this, "A", &a);
		RegisterUtil::registerParamSrc(*this, "B", &b);
	}
	AmpBinaryComposition::AmpBinaryComposition(NoteProcPtr a1, NoteProcPtr b1){
		this->a=a1;
		this->b=b1;		
	}
	String AmpBinaryComposition::toString() const{
		return StringFormat::format("AmpBinaryComposition(%s,%s)", a, b);
	}
	void AmpBinaryComposition::init(ChannelConfig & cfg){
		if(a == nullptr)throw NullPointerException("a == null");
		if(b == nullptr)throw NullPointerException("b == null");
		a->init(cfg);
		b->init(cfg);
	}
	void  AmpBinaryComposition::cc(ChannelConfig & cfg, ChannelControl & cc){
		a->cc(cfg, cc);
		b->cc(cfg, cc);
	}
}