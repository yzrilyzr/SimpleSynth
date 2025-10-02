#include "MultiKeyTrigger.h"
#include "events/Note.h"
#include "events/NoteUpdater.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_util;
using namespace yzrilyzr_array;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	MultiKeyTrigger::~MultiKeyTrigger(){}
	MultiKeyTrigger::MultiKeyTrigger() :MultiKeyTrigger(nullptr, nullptr, nullptr){}
	MultiKeyTrigger::MultiKeyTrigger(NoteProcPtr a, std::shared_ptr<IntArray> noteShift, std::shared_ptr<DoubleArray> velocityMul) : AmpUnaryComposition(a){
		this->notesCount=noteShift->length;
		this->idShift=noteShift;
		this->velocityMul=velocityMul;
	}
	u_sample MultiKeyTrigger::getAmp(Note & note){
		u_sample sum=0;
		MultiKeyTriggerKeyData * data=getData(note);
		Note ** notes=data->notes;
		ChannelConfig & cfg=*note.cfg;
		for(u_index i=0;i < notesCount;i++){
			Note & n1=*notes[i];
			NoteUpdater::preUpdateNote(n1,cfg);
			sum+=a->getAmp(n1);
			NoteUpdater::postUpdateNote(n1, cfg);
		}
		return sum;
	}
	NoteProcPtr MultiKeyTrigger::clone(){
		return std::make_shared<MultiKeyTrigger>(a, idShift, velocityMul);
	}
	MultiKeyTriggerKeyData * MultiKeyTrigger::init(MultiKeyTriggerKeyData * data, Note & note){
		if(data == nullptr){
			data=new MultiKeyTriggerKeyData();
			data->notes=new Note * [notesCount];
			data->size=notesCount;
			for(u_index i=0;i < notesCount;i++){
				int id=uniqueID.fetch_add(1);
				data->notes[i]=new Note(id);
			}
		}
		for(u_index i=0;i < notesCount;i++){
			Note * n=data->notes[i];
			n->set(note);
			n->id=note.id + (*idShift)[i];
			n->velocity=note.velocity * (*velocityMul)[i];
		}
		return data;
	}
	String MultiKeyTrigger::toString() const{
		return StringFormat::object2string("MultiKeyTrigger", a, idShift, velocityMul);
	}
}