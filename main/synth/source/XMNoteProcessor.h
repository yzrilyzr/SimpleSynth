#pragma once
#include "util/XMFile.h"
#include "array/Array.hpp"
#include "synth/source/XMNoteProcessor.h"
#include "synth/generators/sampler/WaveSampler.h"
namespace yzrilyzr_simplesynth{
	ECLASS(XMNoteProcessor, public NoteProcessor){
	private:
	yzrilyzr_util::XMFile::Instrument * xmInstrument;
	std::vector<std::shared_ptr<WaveSampler>> samples;
	NoteProcPtr volEnvelop=nullptr;
	NoteProcPtr panEnvelop=nullptr;
	int index;
	std::shared_ptr<yzrilyzr_util::XMFile::Module> mod;
	double portamentoSpeed=0;
	public:
	XMNoteProcessor()=default;
	~XMNoteProcessor()=default;
	XMNoteProcessor(std::shared_ptr<yzrilyzr_util::XMFile::Module> mod, yzrilyzr_util::XMFile::Instrument * instrument, int ii);
	private:
	int getLoopType(yzrilyzr_util::XMFile::Loop loop_type);
	double calculateDecayRate(int fadeout);
	// 计算音量包络在时间 t 的值
	double envelopeValue(int fadeout, double time);
	public:
	u_sample getAmp(Note & note)override;
	void init(ChannelConfig & cfg)override{
		volEnvelop->init(cfg);
		if(panEnvelop != nullptr)panEnvelop->init(cfg);
	}
	bool noMoreData(Note & note)override{
		if(note.fclosed(*note.cfg))return true;
		if(note.closed(*note.cfg) && xmInstrument->volume_fadeout != 0){
			u_time closedPassed=note.closedPassedTime;
			u_time fadeEnv=envelopeValue(xmInstrument->volume_fadeout, closedPassed);
			return fadeEnv < 0.00001;
		}
		return volEnvelop->noMoreData(note);
	}
	void cc(ChannelConfig & cfg, ChannelControl & cc)override;
	NoteProcPtr clone()override;
	};
}