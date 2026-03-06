#include "Pulse.h"
#include "events/ChannelEvent.h"
#include "events/Note.h"
#include "events/ChannelConfig.h"
#include "dsp/PWM.h"
#include "events/PNData.h"
#include <memory>
#include "interface/NoteProcessor.h"
#include "yzrutil.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void Pulse::onRegisterParam(){
		Osc::onRegisterParam();
		RegisterUtil::registerParamNormal01(*this, "Width", &width);
		RegisterUtil::registerParamNormal01(*this, "Rise", &rise);
		RegisterUtil::registerParamNormal01(*this, "Fall", &fall);
		RegisterUtil::registerParamNormal01(*this, "Delay", &delay);
	}
	u_sample Pulse::getAmp(const Note & note){
		return PWM::pwm(getPhase(note), width, rise, fall, delay) * note.velocitySynth;
	}
	void Pulse::cc(ChannelConfig & cfg, ChannelControl & cc){
		PNData & data=cfg.nrpn;
		if(cc.isMSB()){
			switch(data.select){
				case 10000:
					width=data.dataMSB / PNData::MSB_MAX;
					break;
				case 10001:
					rise=data.dataMSB / PNData::MSB_MAX;
					break;
				case 10002:
					fall=data.dataMSB / PNData::MSB_MAX;
					break;
				case 10003:
					delay=data.dataMSB / PNData::MSB_MAX;
					break;
			}
		}
	}
	NoteProcPtr Pulse::clone(){
		return mksp<Pulse>(getPhaseSource(), width, rise, fall, delay);
	}
	String Pulse::toString()const{
		return StringFormat::object2string("Pulse", getPhaseSource(), width, rise, fall, delay);
	}
}