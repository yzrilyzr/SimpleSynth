#include "SimpleDetuner.h"
#include "events/NoteUpdater.h"
#include "dsp/DSP.h"
using namespace yzrilyzr_array;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	void SimpleDetuner::onRegisterParam(){
		AmpUnaryComposition::onRegisterParam();
		static int minCount=1, maxCount=7;
		registerParam("Count", yzrilyzr_util::ParamType::Int, &count, &minCount, &maxCount);
		RegisterUtil::registerParamNormal01(*this, "Offset", &offset);
		RegisterUtil::registerParamNormal01(*this, "InitPhase", &initPhase);
	}
	u_sample SimpleDetuner::getAmp(const Note & note){
		u_sample sum=0;
		SimpleDetunerKeyData & data=*getData(note);
		Array<Note> & notes=data.notes;
		int32_t c=notes.length;
		if(c > 0){
			int32_t initId=-c / 2;
			ChannelConfig & cfg=*note.cfg;
			for(u_index i=0;i < c;i++){
				Note & n1=notes[i];
				int32_t off=initId + i;
				n1.velocity=note.velocity;
				n1.pitchBend=note.pitchBend + static_cast<s_note_id>(off) * offset;
				NoteUpdater::preUpdateNote(n1, cfg);
				sum+=a->getAmp(n1);
				NoteUpdater::postUpdateNote(n1, cfg);
			}
		}
		return sum;
	}
	SimpleDetunerKeyData * SimpleDetuner::init(SimpleDetunerKeyData * data, const Note & note){
		u_index c=count;
		if(data == nullptr){
			data=new SimpleDetunerKeyData();
		}
		if(data->notes == nullptr || data->notes.length != c){
			data->notes=Array<Note>(c);
		}
		if(!data->uniqueIDSet){
			for(u_index i=0;i < c;i++){
				(data->notes)[i].uniqueID=uniqueID.fetch_add(1);
			}
			data->uniqueIDSet=true;
		}
		for(u_index i=0;i < c;i++){
			Note & n=data->notes[i];
			n.set(note);
			n.phaseSynth=(static_cast<float>(i) + 0.5f) / (c + 1) * initPhase;
		}
		return data;
	}
	void SimpleDetuner::init(ChannelConfig & cfg){
		uniqueID.store(0);
		for(u_index i=0;i < CHANNEL_MAX_VOICE;i++){
			if(data[i] != nullptr){
				data[i]->uniqueIDSet=false;
			};
		}
	}

	NoteProcPtr SimpleDetuner::clone(){
		return mksp<SimpleDetuner>(a->clone(), count, offset);
	}
	String SimpleDetuner::toString() const{
		return StringFormat::object2string("SimpleDetuner", a, count, offset);
	}
}