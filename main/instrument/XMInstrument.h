#pragma once
#include "collection/ArrayList.hpp"
#include "util/XMFile.h"
#include "interface/NoteProcessor.h"
#include "interface/InstrumentProvider.h"
#include "synth/osc/sampler/WaveSampler.h"
#include "array/Array.hpp"
namespace yzrilyzr_simplesynth{
	ECLASS(XMInstrument, public InstrumentProvider){
	private:
	yzrilyzr_collection::ArrayList<NoteProcPtr> insts;
	u_sp<yzrilyzr_util::XMFile::Module> mod;
	public:
	XMInstrument(u_sp<yzrilyzr_util::XMFile::Module> mod);
	NoteProcPtr get(s_bank_id bank, s_program_id program, u_sample_rate sampleRate)override{
		if(program >= insts.size())return nullptr;
		return insts[program]->clone();
	}
	NoteProcPtr getDrumSet(s_bank_id bank, u_sample_rate sampleRate)override{
		return nullptr;
	}
	};
	ECLASS(XMNoteProcessor, public NoteProcessor){
	private:
	yzrilyzr_util::XMFile::Instrument * xmInstrument;
	std::vector<NoteProcPtr> samples;
	NoteProcPtr volEnvelop=nullptr;
	NoteProcPtr panEnvelop=nullptr;
	NoteProcPtr fadeoutEnvelop=nullptr;
	int instrumentInModIndex;
	u_sp<yzrilyzr_util::XMFile::Module> mod;
	//
	int currentEffect=-1;
	int currentVolCol=-1;
	int effectArg=0;
	s_note_id portamentoIDDelta=0;
	s_note_id arpeggioIDDelta=0;
	public:
	~XMNoteProcessor()=default;
	XMNoteProcessor()=default;
	XMNoteProcessor(u_sp<yzrilyzr_util::XMFile::Module> mod, yzrilyzr_util::XMFile::Instrument * instrument, int instrumentInModIndex);
	private:
	int getLoopType(yzrilyzr_util::XMFile::Loop loop_type);
	public:
	u_sample getAmp(const Note & note)override;
	void init(ChannelConfig & cfg)override;
	bool noMoreData(const Note & note)const override;
	void cc(ChannelConfig & cfg, ChannelControl & cc)override;
	void noteOn(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel)override;
	NoteProcPtr clone()override;
	};
}