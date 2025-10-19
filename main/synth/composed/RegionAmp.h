#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "events/Note.h"
#include "util/Util.h"
#include <cmath>
#include <limits>
#include <unordered_set>

namespace yzrilyzr_simplesynth{
	ECLASS(RegionAmp, public NoteProcessor){
	private:
	struct Region{
		uint8_t noteStart=0;
		uint8_t noteEnd=0;
		uint8_t velStart=0;
		uint8_t velEnd=0;
		NoteProcPtr ptr=nullptr;
	};
	std::vector<Region> region;
	std::unordered_set <NoteProcPtr> addedAllRegions;
	std::vector< NoteProcPtr> buildRegion[CHANNEL_MAX_NOTE_ID * CHANNEL_MAX_NOTE_ID];
	public:
	void put(uint8_t noteS, uint8_t noteE, uint8_t velS, uint8_t velE, NoteProcPtr proc){
		region.emplace_back(Region{noteS, noteE, velS, velE, proc});
		addedAllRegions.insert(proc);
	}
	void init(ChannelConfig & cfg) override{
		for(auto & p : addedAllRegions){
			if(p != nullptr)p->init(cfg);
		}
	}
	bool noMoreData(Note & note) override{
		u_index noteid=(u_index)note.id;
		u_index vel=yzrilyzr_util::Util::clamp((int)(note.velocity * 127.0), 0, 127);
		auto & reg1=buildRegion[noteid * CHANNEL_MAX_NOTE_ID + vel];
		for(auto & ptr : reg1){
			return ptr->noMoreData(note);
		}
		return true;
	}
	void cc(ChannelConfig & cfg, ChannelControl & cc) override{
		for(auto & p : addedAllRegions){
			if(p != nullptr)p->cc(cfg, cc);
		}
	}
	void noteOn(ChannelConfig & cfg, s_note_id_i id, s_note_vel fvel) override{
		u_index noteid=(u_index)id;
		u_index vel=yzrilyzr_util::Util::clamp((int)(fvel * 127.0), 0, 127);
		auto & reg1=buildRegion[noteid * CHANNEL_MAX_NOTE_ID + vel];
		for(auto & ptr : reg1){
			ptr->noteOn(cfg, id, fvel);
		}
	}
	void noteOff(ChannelConfig & cfg, s_note_id_i id, s_note_vel fvel) override{
		u_index noteid=(u_index)id;
		u_index vel=yzrilyzr_util::Util::clamp((int)(fvel * 127.0), 0, 127);
		auto & reg1=buildRegion[noteid * CHANNEL_MAX_NOTE_ID + vel];
		for(auto & ptr : reg1){
			ptr->noteOff(cfg, id, fvel);
		}
	}
	u_sample getAmp(Note & note) override{
		u_index noteid=(u_index)note.id;
		u_index vel=yzrilyzr_util::Util::clamp((int)(note.velocity * 127.0), 0, 127);
		u_sample sum=0;
		auto & reg1=buildRegion[noteid * CHANNEL_MAX_NOTE_ID + vel];
		for(auto & ptr : reg1){
			sum+=ptr->getAmp(note);
		}
		return sum;
	}
	NoteProcPtr clone() override{
		std::shared_ptr<RegionAmp> re=std::make_shared<RegionAmp>();
		re->region=region;
		re->addedAllRegions=addedAllRegions;
		re->build();
		return re;
	}
	void build(){
		for(auto & reg : region){
			for(u_index id=reg.noteStart;id <= reg.noteEnd;id++){
				for(u_index vel=reg.velStart;vel <= reg.velEnd;vel++){
					buildRegion[id * CHANNEL_MAX_NOTE_ID + vel].emplace_back(reg.ptr);
				}
			}
		}
	}
	yzrilyzr_lang::String toString() const override{
		return "RegionAmp";
	}
	};
}