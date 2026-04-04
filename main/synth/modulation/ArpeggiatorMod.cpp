#include "ArpeggiatorMod.h"
#include "lang/StringFormat.hpp"
#include "dsp/DSP.h"
#include "events/Note.h"
#include "events/NoteUpdater.h"
#include "interface/NoteTuning.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_array;
using namespace yzrilyzr_lang;

namespace yzrilyzr_simplesynth{
	ArpeggiatorMod::ArpeggiatorMod() :NoteModulation(nullptr),
		offsets(nullptr),
		rate(0){}
	ArpeggiatorMod::ArpeggiatorMod(NoteProcPtr a, yzrilyzr_array::IntArray offsets, u_freq rate) :
		NoteModulation(a),
		offsets(offsets),
		rate(rate){}
	void ArpeggiatorMod::init(ChannelConfig & cfg){
		NoteModulation::init(cfg);
		if(offsets == nullptr)throw NullPointerException("offsets == null");
	}
	void ArpeggiatorMod::applyMod(Note & note){
		u_index offset=(note.passedTime * rate);
		offset=offset % offsets.length;
		note.idSynth+=offsets[offset];
		note.freqSynth=note.cfg->tuning->getFrequencyByID(note.idSynth);
		ArpeggiatorModKeyData & data=*getData(note);
		data.phase+=note.freqSynth * note.cfg->deltaTime;
		note.phaseSynth=data.phase;
	}
	NoteProcPtr ArpeggiatorMod::clone(){
		return mksp<ArpeggiatorMod>(a, offsets, rate);
	}
	yzrilyzr_lang::String ArpeggiatorMod::toString() const{
		return StringFormat::object2string("ArpeggiatorMod", a, offsets, rate);
	}
	ArpeggiatorModKeyData * ArpeggiatorMod::init(ArpeggiatorModKeyData * data, const Note & note){
		if(data == nullptr)data=new ArpeggiatorModKeyData();
		data->phase=0;
		return data;
	}

	void ArpeggiatorMod::onRegisterParam(){
		AmpUnaryComposition::onRegisterParam();
		registerParamIntArray("Offsets", &offsets);
		static u_freq min=0.25, max=100;
		registerParam("Rate", ParamType::Freq, &rate, &min, &max);
	}
}