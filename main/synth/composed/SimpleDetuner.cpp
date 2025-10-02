#include "SimpleDetuner.h"
#include "events/NoteUpdater.h"
using namespace yzrilyzr_array;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	u_sample SimpleDetuner::getAmp(Note & note){
		u_sample sum=0;
		SimpleDetunerKeyData & data=*getData(note);
		Array<Note *> & notes=*data.notes;
		int32_t c=notes.length;
		if(c > 0){
			int32_t initId=-c / 2;
			ChannelConfig & cfg=*note.cfg;
			for(u_index i=0;i < c;i++){
				Note & n1=*notes[i];
				int32_t off=initId + i;
				n1.velocity=note.velocity;
				n1.pitchBend=note.pitchBend + static_cast<s_note_id>(off) * offset;
				NoteUpdater::preUpdateNote(n1,cfg);
				sum+=a->getAmp(n1);
				NoteUpdater::postUpdateNote(n1, cfg);
			}
			//sum/=static_cast<u_sample>(c);
		}
		return sum;
	}
	SimpleDetunerKeyData * SimpleDetuner::init(SimpleDetunerKeyData * data, Note & note){
		u_index c=count;
		if(data == nullptr){
			data=new SimpleDetunerKeyData();
			data->notes=nullptr;
		}
		if(data->notes == nullptr || data->notes->length != c){
			delete data->notes;
			data->notes=new Array<Note *>(c);
			for(u_index i=0;i < c;i++){
				int id=uniqueID.fetch_add(1);
				(*data->notes)[i]=new Note(id);
			}
		}
		for(u_index i=0;i < c;i++){
			Note * n=(*data->notes)[i];
			n->set(note);
			n->phaseSynth=(static_cast<float>(i) + 0.5f) / (c + 1);
		}
		return data;
	}
	NoteProcPtr SimpleDetuner::clone(){
		return std::make_shared<SimpleDetuner>(a->clone(), count, offset);
	}
	String SimpleDetuner::toString() const{
		return StringFormat::object2string("SimpleDetuner", a, count, offset);
	}
}