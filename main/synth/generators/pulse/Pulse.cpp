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
	u_sample Pulse::getAmp(Note & note){
		return PWM::pwm(getPhase(note), width, rise, fall, delay) * note.velocitySynth;
	}
	void Pulse::cc(ChannelConfig & cfg, ChannelControl & cc){
		PNData & data=cfg.nrpn;
		if(cc.isMSB()){
			switch(data.select){
				case 10000:
					width=data.dataMSB / 127.0;
					break;
				case 10001:
					rise=data.dataMSB / 127.0;
					break;
				case 10002:
					fall=data.dataMSB / 127.0;
					break;
				case 10003:
					delay=data.dataMSB / 127.0;
					break;
			}
		}
	}
	NoteProcPtr Pulse::clone(){
		return std::make_shared<Pulse>(getPhaseSource(), width, rise, fall, delay);
	}
	std::string Pulse::toString()const{
		return StringFormat::object2string("Pulse", getPhaseSource(), width, rise, fall, delay);
	}
}