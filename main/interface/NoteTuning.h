#pragma once
#include "SimpleSynth.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(NoteTuning){
	private:
	u_freq noteFrequencies[CHANNEL_MAX_NOTE_ID]={0};
	protected:
	u_freq baseFreq;//基频
	public:
	/**
	 * 根据音符ID的返回音符频率
	 */
	u_freq getFrequencyByID(s_note_id id){
		int idint=static_cast<int>(id);
		if(id != idint || idint > 127 || idint < 0) return getFrequency(id);
		return noteFrequencies[idint];
	}
	virtual u_freq getFrequency(s_note_id id)=0;
	/**
	 * 根据音符频率返回音符ID
	 */
	virtual s_note_id getIDByFrequency(u_freq frequency)=0;
	void setBaseFreq(const u_freq freq){
		baseFreq=freq;
		generateNoteFrequencyTable();
	}
	u_freq getBaseFreq()const{
		return baseFreq;
	}
	virtual void setBaseFreqByA(u_freq frequency)=0;
	protected:
	void generateNoteFrequencyTable(){
		for(s_note_id_i i=0;i < CHANNEL_MAX_NOTE_ID;i++){
			noteFrequencies[i]=getFrequency(i);
		}
	}
	};
}
