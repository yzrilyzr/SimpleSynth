#pragma once
#include "interface/InstrumentProvider.h"
#include "interface/NoteProcessor.h"
#include <map>

namespace yzrilyzr_simplesynth{
	ECLASS(ReplaceableInstrument, public InstrumentProvider){
	private:
	std::map<std::pair<s_bank_id, s_program_id>, NoteProcPtr> map;
	std::shared_ptr<InstrumentProvider> parent;
	NoteProcPtr drumSet;
	public:
	ReplaceableInstrument();
	ReplaceableInstrument(std::shared_ptr<InstrumentProvider> parent);
	NoteProcPtr getDrumSet();
	void setDrumSet(NoteProcPtr drumSet);
	void put(s_program_id program, NoteProcPtr src);
	void put(s_bank_id bank, s_program_id program, NoteProcPtr src);
	NoteProcPtr get(s_bank_id bank, s_program_id program, u_sample_rate sampleRate) override;
	NoteProcPtr getDrumSet(s_bank_id bank, u_sample_rate sampleRate) override;
	};
}