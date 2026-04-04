#include "ArpeggiatorTrigger.h"
#include "lang/StringFormat.hpp"
#include "dsp/DSP.h"
#include "events/Note.h"
#include "events/NoteUpdater.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_array;
using namespace yzrilyzr_lang;

namespace yzrilyzr_simplesynth{
	ArpeggiatorTrigger::ArpeggiatorTrigger() :AmpUnaryComposition(nullptr),
		offsets(nullptr),
		rate(0){}
	ArpeggiatorTrigger::ArpeggiatorTrigger(NoteProcPtr a, yzrilyzr_array::IntArray offsets, u_freq rate) :
		AmpUnaryComposition(a),
		offsets(offsets),
		rate(rate){}
	void ArpeggiatorTrigger::init(ChannelConfig & cfg){
		AmpUnaryComposition::init(cfg);
		if(offsets == nullptr)throw NullPointerException("offsets == null");
		uniqueID.store(0);
		for(u_index i=0;i < CHANNEL_MAX_VOICE;i++){
			if(data[i] != nullptr){
				data[i]->uniqueIDSet=false;
			};
		}
	}
	u_sample ArpeggiatorTrigger::getAmp(const Note & note){
		ArpeggiatorTriggerKeyData & data=*getData(note);
		u_index offset=(note.passedTime * rate);
		offset=offset % offsets.length;
		if(data.lastTrigger != offset){
			NoteUpdater::noteOn(data.notes[offset], *note.cfg, note.id + offsets[offset], note.velocity);
			if(data.lastTrigger != -1){
				//如果源没有包络，则会多个音高同时响
				NoteUpdater::noteOff(data.notes[data.lastTrigger], *note.cfg, note.velocity);
			}
			data.lastTrigger=offset;
		}
		u_sample v=0;
		for(u_index i=0;i < offsets.length;i++){
			if(note.passedTime < static_cast<u_time>(i) / rate)continue;
			Note & arp=data.notes[i];
			NoteUpdater::preUpdateNote(arp, *note.cfg);
			v+=a->getAmp(arp);
			NoteUpdater::postUpdateNote(arp, *note.cfg);
		}
		return v;
	}
	NoteProcPtr ArpeggiatorTrigger::clone(){
		return mksp<ArpeggiatorTrigger>(a, offsets, rate);
	}
	yzrilyzr_lang::String ArpeggiatorTrigger::toString() const{
		return StringFormat::object2string("ArpeggiatorTrigger", a, offsets, rate);
	}
	void ArpeggiatorTrigger::onRegisterParam(){
		AmpUnaryComposition::onRegisterParam();
		registerParamIntArray("Offsets", &offsets);
		static u_freq min=0.25, max=100;
		registerParam("Rate", ParamType::Freq, &rate, &min, &max);
	}
	bool ArpeggiatorTrigger::noMoreData(const Note & note)const{
		return note.closed(*note.cfg);//使用外部包络控制release行为避免立即截断
	}
	ArpeggiatorTriggerKeyData * ArpeggiatorTrigger::init(ArpeggiatorTriggerKeyData * data, const Note & note){
		if(data == nullptr)data=new ArpeggiatorTriggerKeyData();
		data->lastTrigger=-1;
		if(data->notes == nullptr || data->notes.length != offsets.length){
			data->notes=Array<Note>(offsets.length);
		}
		if(!data->uniqueIDSet){
			for(u_index i=0;i < offsets.length;i++){
				(data->notes)[i].uniqueID=uniqueID.fetch_add(1);
			}
			data->uniqueIDSet=true;
		}
		for(u_index i=0;i < offsets.length;i++){
			data->notes[i].set(note);
			data->notes[i].id+=offsets[i];
		}
		return data;
	}
}