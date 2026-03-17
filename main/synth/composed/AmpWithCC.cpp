#include "AmpWithCC.h"
#include "events/ChannelEvent.h"
#include "events/ChannelConfig.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_lang;
using namespace yzrilyzr_array;
namespace yzrilyzr_simplesynth{
	AmpWithCC::~AmpWithCC(){}
	AmpWithCC::AmpWithCC() :AmpWithCC(nullptr, IntArray()){}
	AmpWithCC::AmpWithCC(NoteProcPtr a,const IntArray& cc) : AmpUnaryComposition(a){
		this->cc=cc;
	}
	u_sample AmpWithCC::getAmp(const Note & note){
		return a->getAmp(note);
	}
	NoteProcPtr AmpWithCC::clone(){
		return mksp<AmpWithCC>(a, cc);
	}
	void AmpWithCC::init(ChannelConfig & cfg){
		AmpUnaryComposition::init(cfg);
		for(uint32_t i=0;i < cc.length;i+=2){
			cfg.postInstantEvent(mkup<ChannelControl>(cc[i], cc[i + 1]));
		}
	}
	String AmpWithCC::toString()const{
		return StringFormat::object2string("AmpWithCC", a, cc);
	}
}