#include "AmpUnaryComposition.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	AmpUnaryComposition::AmpUnaryComposition(NoteProcPtr a1){
		this->a=a1;
		registerParamSrc("A", &a);
	}
	void AmpUnaryComposition::cc(ChannelConfig & cfg, ChannelControl & cc){
		a->cc(cfg, cc);
	}
	void AmpUnaryComposition::init(ChannelConfig & cfg){
		if(a == nullptr)throw NullPointerException("a == null");
		a->init(cfg);
	}
	std::string AmpUnaryComposition::toString() const{
		return StringFormat::format("AmpUnaryComposition(%s)", a);
	}
}