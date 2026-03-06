#pragma once
#include "util/XMFile.h"
#include "array/Array.hpp"
#include "synth/source/XMNoteProcessor.h"
#include "synth/generators/sampler/WaveSampler.h"
namespace yzrilyzr_simplesynth{
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
	bool noMoreData(const Note & note)override;
	void cc(ChannelConfig & cfg, ChannelControl & cc)override;
	void noteOn(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel)override;
	NoteProcPtr clone()override;
	};
}