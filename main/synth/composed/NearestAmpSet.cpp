#include "NearestAmpSet.h"
#include "interface/NoteProcessor.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	NearestAmpSet::~NearestAmpSet(){
		//delete notes;
	}
	NearestAmpSet::NearestAmpSet(std::shared_ptr<PhaseSrc> freq) : Osc(freq){}
	NearestAmpSet::NearestAmpSet() : NearestAmpSet(nullptr){}

	std::string NearestAmpSet::toString()const{
		return StringFormat::format("NearestAmpSet(%s)", getPhaseSource());
	}
	bool NearestAmpSet::noMoreData(Note & note){
		int id=note.id;
		NoteProcPtr processor=notes[id];
		if(processor == nullptr) return true;
		return processor->noMoreData(note);
	}
	NearestAmpSetBuilder::NearestAmpSetBuilder(){
		objptr=std::make_shared<NearestAmpSet>();
	}
	NearestAmpSetBuilder & NearestAmpSetBuilder::add(int note, NoteProcPtr noteProcessor){
		objptr->notes[note]=noteProcessor;
		return *this;
	}
	NoteProcPtr NearestAmpSetBuilder::build(){
		int firstNote=-1;
		int lastNote=-1;
		NoteProcPtr firstProcessor=nullptr;
		NoteProcPtr lastProcessor=nullptr;

		// 首先找到第一个和最后一个有数据的音符
		for(uint32_t i=0; i < CHANNEL_MAX_NOTE_ID; i++){
			if(objptr->notes[i] != nullptr){
				if(firstNote == -1){
					firstNote=i;
					firstProcessor=objptr->notes[i];
				}
				lastNote=i;
				lastProcessor=objptr->notes[i];
			}
		}

		// 如果没有找到任何处理器，直接返回
		if(firstNote == -1){
			return std::dynamic_pointer_cast<NoteProcessor>(objptr);
		}

		// 填充第一个有数据音符之前的部分
		for(int i=0; i < firstNote; i++){
			objptr->notes[i]=firstProcessor;
		}

		// 填充中间部分
		int prevNote=firstNote;
		NoteProcPtr prevProcessor=firstProcessor;

		for(int i=firstNote + 1; i <= lastNote; i++){
			if(objptr->notes[i] != nullptr){
				// 填充两个有数据音符之间的部分
				for(int j=prevNote + 1; j < i; j++){
					// 选择最近的处理器
					if((j - prevNote) <= (i - j)){
						objptr->notes[j]=prevProcessor;
					} else{
						objptr->notes[j]=objptr->notes[i];
					}
				}
				prevNote=i;
				prevProcessor=objptr->notes[i];
			}
		}

		// 填充最后一个有数据音符之后的部分
		for(int i=lastNote + 1; i < CHANNEL_MAX_NOTE_ID; i++){
			objptr->notes[i]=lastProcessor;
		}

		return std::dynamic_pointer_cast<NoteProcessor>(objptr);
	}
	u_sample NearestAmpSet::getAmp(Note & note){
		uint8_t id=note.id;
		NoteProcPtr processor=notes[id];
		if(processor == nullptr) return 0; // 返回0而不是true
		return processor->getAmp(note);
	}
}