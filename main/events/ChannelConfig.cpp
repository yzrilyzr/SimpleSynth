#include "events/ChannelConfig.h"
#include "interface/IMixer.h"
#include "events/ChannelEvent.h"
namespace yzrilyzr_simplesynth{
	ChannelConfig::~ChannelConfig(){}
	void ChannelConfig::postInstantEvent(ChannelEvent * event){
		if(mixer != nullptr)mixer->sendInstantEvent(event);
	}
	void ChannelConfig::setOnlyChannelConfig(ChannelConfig & other){
		Sustain=other.Sustain;
		MonoMode=other.MonoMode;
		Portamento=other.Portamento;
		PortamentoTime=other.PortamentoTime;
		Legato=other.Legato;
		Sostenuto=other.Sostenuto;
		SoftPedal=other.SoftPedal;
		Modulation=other.Modulation;
		ModRate=other.ModRate;
		ModDelay=other.ModDelay;
		ModDepth=other.ModDepth;
		Volume=other.Volume;
		Pan=other.Pan;
		ChannelPitchBend=other.ChannelPitchBend;
		PitchBendRange=other.PitchBendRange;
		Detune=other.Detune;
		Expression=other.Expression;
		Breath=other.Breath;
		Foot=other.Foot;
		FineTune=other.FineTune;
		CoarseTune=other.CoarseTune;
		NoteShift=other.NoteShift;
		Bank=other.Bank;
		lastNote=other.lastNote;
		noteProcessor=other.noteProcessor;
		tuning=other.tuning;
		velocityMap=other.velocityMap;
		rpn.set(other.rpn);
		nrpn.set(other.nrpn);
		memcpy(noteHoldMap, other.noteHoldMap, sizeof(bool) * CHANNEL_MAX_NOTE_ID);
		memcpy(sostenutoLock, other.sostenutoLock, sizeof(bool) * CHANNEL_MAX_NOTE_ID);
	}
	void ChannelConfig::allNotesOff(){
		for(int i=0;i < CHANNEL_MAX_NOTE_ID;i++){
			noteHoldMap[i]=false;
		}
	}
	void ChannelConfig::sostenutoChange(){
		if(Sostenuto){
			if(Sustain){
				// 延音锁定全部音符
				std::fill(sostenutoLock, sostenutoLock + CHANNEL_MAX_NOTE_ID, true);
				return;
			}
			// 非延音，锁定当前按下的音符
			for(int i=0; i < CHANNEL_MAX_NOTE_ID; i++){
				if(noteHoldMap[i])sostenutoLock[i]=true;
			}
			return;
		}
		// 解除所有锁定
		std::fill(sostenutoLock, sostenutoLock + CHANNEL_MAX_NOTE_ID, false);
	}
	void ChannelConfig::setNoteProcessor(NoteProcPtr val){
		sp_noteProcessor=val;
		noteProcessor=val.get();
	}
	void ChannelConfig::setNoteTuning(std::shared_ptr<NoteTuning> val){
		sp_tuning=val;
		tuning=val.get();
	}
	void ChannelConfig::setNoteVelocityMap(std::shared_ptr<yzrilyzr_interpolator::Interpolator> val){
		sp_velocityMap=val;
		velocityMap=val.get();
	}
	void ChannelConfig::reset(){
		Pan=0;
		Volume=0.7f;
		Expression=1;
		Breath=1;
		Foot=1;
		ChannelPitchBend=0;
		PitchBendRange=2;
		Detune=0.0f;
		NoteShift=0;
		Modulation=0;
		ModDelay=0.3f;
		ModDepth=0.5f;
		ModRate=5.0f;
		FineTune=0;
		CoarseTune=0;
		Sustain=false;
		Legato=false;
		SoftPedal=false;
		MonoMode=false;
		Sostenuto=false;
		Portamento=false;
		PortamentoTime=0;
		rpn.reset();
		nrpn.reset();
		Bank=0;
	}
}