#include "AmpWithCC.h"
#include "events/ChannelEvent.h"
#include "events/ChannelConfig.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_lang;
using namespace yzrilyzr_array;
namespace yzrilyzr_simplesynth{
	AmpWithCC::~AmpWithCC(){}
	AmpWithCC::AmpWithCC() :AmpWithCC(nullptr, yzrilyzr_array::IntArray()){}
	AmpWithCC::AmpWithCC(NoteProcPtr a,const yzrilyzr_array::IntArray& cc) : AmpUnaryComposition(a){
		this->cc=cc;
	}
	u_sample AmpWithCC::getAmp(Note & note){
		return a->getAmp(note);
	}
	NoteProcPtr AmpWithCC::clone(){
		return std::make_shared<AmpWithCC>(a, cc);
	}
	void AmpWithCC::init(ChannelConfig & cfg){
		AmpUnaryComposition::init(cfg);
		for(uint32_t i=0;i < cc.length;i+=2){
			ChannelControl * cc1=new ChannelControl(cc[i], cc[i + 1]);
			cfg.postInstantEvent(cc1);
		}
	}
	String AmpWithCC::toString()const{
		return StringFormat::object2string("AmpWithCC", a, cc);
	}
}