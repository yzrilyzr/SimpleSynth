#include "NeighbourMixAmpSet.h"
#include "interface/NoteProcessor.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	NeighbourMixAmpSet::NeighbourMixAmpSet(std::shared_ptr<PhaseSrc> freq) : Osc(freq){}
	NeighbourMixAmpSet::NeighbourMixAmpSet() : NeighbourMixAmpSet(nullptr){}
	NeighbourMixAmpSet * NeighbourMixAmpSet::add(int note, NoteProcPtr noteProcessor){
		notes[note][0]=noteProcessor;
		notes[note][1]=noteProcessor;
		notesRatio[note][0]=1.0;
		notesRatio[note][1]=1.0;
		return this;
	}
	String NeighbourMixAmpSet::toString()const{
		return StringFormat::format("InterpolateAmpSet(%s)", getPhaseSource());
	}
	bool NeighbourMixAmpSet::noMoreData(Note & note){
		int id=note.id;
		auto d=notes[id];
		if(d[0] == nullptr || d[1] == nullptr) return true;
		if(d[0] == d[1]) return d[0]->noMoreData(note);
		return d[0]->noMoreData(note) && d[1]->noMoreData(note);
	}
	NeighbourMixAmpSet * NeighbourMixAmpSet::build(){
		int i0=-1;
		auto d=notes[0];
		for(uint32_t i=0;i < CHANNEL_MAX_NOTE_ID;i++){
			auto d1=notes[i];
			if(d1[0] != nullptr || d1[1] != nullptr){
				if(d[0] == nullptr || d[1] == nullptr){
					d=d1;
					notes[0][0]=d[0];
					notes[0][1]=d[1];
					notesRatio[0][0]=notesRatio[i][0];
					notesRatio[0][1]=notesRatio[i][1];
					i0=0;
				}
				for(uint32_t j=i0 + 1;j < i;j++){
					notes[j][0]=d[1];
					notes[j][1]=d1[0];
					double size=i - i0;
					notesRatio[j][0]=(i - j) / size;
					notesRatio[j][1]=(j - i0) / size;
				}
				d=d1;
				i0=i;
			}
		}
		if(i0 < 127){
			for(uint32_t j=i0 + 1;j < CHANNEL_MAX_NOTE_ID;j++){
				notes[j][0]=d[0];
				notes[j][1]=d[1];
			}
		}
		return this;
	}
	u_sample NeighbourMixAmpSet::getAmp(Note & note){
		uint8_t id=note.id;
		auto d=notes[id];
		if(d[0] == nullptr || d[1] == nullptr) return true;
		if(d[0] == d[1]) return d[0]->getAmp(note);
		double * ratio=notesRatio[id];
		u_sample s1=d[0]->getAmp(note);
		u_sample s2=d[1]->getAmp(note);
		return s1 * ratio[0] + s2 * ratio[1];
	}
}