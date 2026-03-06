#include "AmpUnaryComposition.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_lang;
using namespace yzrilyzr_util;

namespace yzrilyzr_simplesynth{
	void AmpUnaryComposition::onRegisterParam(){
		RegisterUtil::registerParamSrc(*this, "A", &a);
	}
	AmpUnaryComposition::AmpUnaryComposition(NoteProcPtr a1){
		this->a=a1;		
	}
	void AmpUnaryComposition::cc(ChannelConfig & cfg, ChannelControl & cc){
		a->cc(cfg, cc);
	}
	void AmpUnaryComposition::init(ChannelConfig & cfg){
		if(a == nullptr)throw NullPointerException("a == null");
		a->init(cfg);
	}
	String AmpUnaryComposition::toString() const{
		return StringFormat::format("AmpUnaryComposition(%s)", a);
	}
}