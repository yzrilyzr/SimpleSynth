#include "ReplaceableInstrument.h"

namespace yzrilyzr_simplesynth{
	ReplaceableInstrument::ReplaceableInstrument(){}
	ReplaceableInstrument::ReplaceableInstrument(u_sp<InstrumentProvider> parent) : ReplaceableInstrument(){
		this->parent=parent;
	}
	NoteProcPtr ReplaceableInstrument::getDrumSet(){
		return drumSet;
	}
	void ReplaceableInstrument::setDrumSet(NoteProcPtr drumSet){
		this->drumSet=drumSet;
	}
	void ReplaceableInstrument::put(s_program_id program, NoteProcPtr src){
		put(0, program, src);
	}
	NoteProcPtr ReplaceableInstrument::get(s_bank_id bank, s_program_id program, u_sample_rate sampleRate){
		auto it=map.find({bank, program});
		if(it != map.end()){
			NoteProcPtr p=it->second;
			if(p != nullptr) return p;
		}
		if(parent != nullptr) return parent->get(bank, program, sampleRate);
		return nullptr;
	}
	void ReplaceableInstrument::put(s_bank_id bank, s_program_id program, NoteProcPtr src){
		map[{bank, program}]=src;
	}
	NoteProcPtr ReplaceableInstrument::getDrumSet(s_bank_id bank, u_sample_rate sampleRate){
		if(drumSet != nullptr) return drumSet;
		if(parent != nullptr) return parent->getDrumSet(bank, sampleRate);
		return nullptr;
	}
}