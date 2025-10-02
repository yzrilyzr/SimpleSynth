#include "LFSRNoise.h"
#include "events/ChannelEvent.h"
#include "util/MIDIFile.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	LFSRNoise::LFSRNoise(){
		static int32_t min=6, max=14;
		registerParam("ShiftCount", ParamType::Int, &shiftCount, &min, &max);
		lfsr=0x4000;
		shiftCount=6;
	}
	LFSRNoise::LFSRNoise(int shiftCount){
		lfsr=0x4000;
		this->shiftCount=shiftCount;
	}
	u_sample LFSRNoise::getAmp(Note & note){
		if(shiftCount == 14){
			lfsrCurrent=nextBit();
		} else{
			if(note.phaseSynth - (int)note.phaseSynth > 0.5){
				if(!lfsrUpdate){
					lfsrUpdate=true;
					lfsrCurrent=nextBit();
				}
			} else lfsrUpdate=false;
		}
		return (lfsrCurrent * 2 - 1) * note.velocitySynth;
	}
	int LFSRNoise::nextBit(){
		int lfsr=this->lfsr;
		int feedback=(lfsr & 1) ^ ((lfsr >> 1) & 1);
		lfsr>>=1;
		feedback^=((lfsr >> shiftCount) & 1);
		lfsr|=feedback << 14;
		this->lfsr=lfsr;
		return lfsr & 1;
	}
	void LFSRNoise::cc(ChannelConfig & cfg, ChannelControl & cc){
		if(cc.control == MIDIFile::CC::DATA_ENTRY_MSB){
			if(cfg.nrpn.select == 16320){
				shiftCount=cfg.nrpn.dataMSB >= 64?14:6;
			}
		}
	}
	NoteProcPtr LFSRNoise::clone(){
		return std::make_shared<LFSRNoise>(shiftCount);
	}
	String LFSRNoise::toString()const{
		return StringFormat::object2string("LFSRNoise", shiftCount);
	}
}