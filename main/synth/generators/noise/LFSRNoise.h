#pragma once
#include "interface/NoteProcessor.h"
#include "events/Note.h"

namespace yzrilyzr_simplesynth{
	ECLASS(LFSRNoise, public NoteProcessor){
	private:
	bool lfsrUpdate=false;
	int lfsr=0;
	int lfsrCurrent=0;
	int shiftCount=0;
	public:
	LFSRNoise();
	LFSRNoise(int shiftCount);
	u_sample getAmp(Note & note) override;
	int nextBit();
	void cc(ChannelConfig & cfg, ChannelControl & cc) override;
	NoteProcPtr clone() override;
	std::string toString() const override;
	};
}