#pragma once
#include "collection/ArrayList.hpp"
#include "util/XMFile.h"
#include "interface/NoteProcessor.h"
#include "interface/InstrumentProvider.h"
namespace yzrilyzr_simplesynth{
	ECLASS(XMInstrument, public InstrumentProvider){
	private:
	yzrilyzr_collection::ArrayList<NoteProcPtr> insts;
	std::shared_ptr<yzrilyzr_util::XMFile::Module> mod;
	public:
	XMInstrument(std::shared_ptr<yzrilyzr_util::XMFile::Module> mod);
	NoteProcPtr get(s_bank_id bank, s_program_id program, u_sample_rate sampleRate)override{
		return insts[program]->clone();
	}
	NoteProcPtr getDrumSet(s_bank_id bank, u_sample_rate sampleRate)override{
		return nullptr;
	}
	};
}