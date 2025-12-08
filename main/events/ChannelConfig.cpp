#include "events/ChannelConfig.h"
#include "events/ChannelEvent.h"
#include "instrument/SimpleMIDIInstrument.h"
#include "interface/IMixer.h"
#include "interpolator/PowInterpolator.h"
#include "tuning/EqualTemperament.h"

using namespace yzrilyzr_interpolator;

namespace yzrilyzr_simplesynth{
	ChannelConfig::~ChannelConfig(){}
	void ChannelConfig::postInstantEvent(ChannelEvent * event){
		if(mixer != nullptr)mixer->sendInstantEvent(event);
	}
	void ChannelConfig::setOnlyChannelConfig(const ChannelConfig & other){
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
		dsp3d=other.dsp3d;
		instrument=other.instrument;
		rpn.set(other.rpn);
		nrpn.set(other.nrpn);
		memcpy(noteHoldMap, other.noteHoldMap, sizeof(bool) * CHANNEL_MAX_NOTE_ID);
		memcpy(sostenutoLock, other.sostenutoLock, sizeof(bool) * CHANNEL_MAX_NOTE_ID);
	}
	void ChannelConfig::allNotesOff(){
		for(u_index i=0;i < CHANNEL_MAX_NOTE_ID;i++){
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
			for(u_index i=0; i < CHANNEL_MAX_NOTE_ID; i++){
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
	void ChannelConfig::setContext(IMixer * mixer,  IChannel * channel){
		this->mixer=mixer;
		this->channel=channel;
	}
	void ChannelConfig::setNoteTuning(std::shared_ptr<NoteTuning> val){
		sp_tuning=val;
		tuning=val.get();
	}
	void ChannelConfig::setNoteVelocityMap(std::shared_ptr<yzrilyzr_interpolator::Interpolator> val){
		sp_velocityMap=val;
		velocityMap=val.get();
	}
	void ChannelConfig::setInstrumentProvider(std::shared_ptr<InstrumentProvider> instr){
		sp_instrument=instr;
		instrument=instr.get();
	}
	/*std::shared_ptr<InstrumentProvider> ChannelConfig::getInstrumentProvider()const{
		return sp_instrument;
	}
	std::shared_ptr<NoteTuning> ChannelConfig::getNoteTuning()const{
		return sp_tuning;
	}
	std::shared_ptr<yzrilyzr_interpolator::Interpolator> ChannelConfig::getNoteVelocityMap()const{
		return sp_velocityMap;
	}*/
	void ChannelConfig::set3DEffect(std::shared_ptr<yzrilyzr_dsp::DSP3D> dsp3d){
		this->sp_dsp3d=dsp3d;
		this->dsp3d=dsp3d.get();
	}
	/*std::shared_ptr<yzrilyzr_dsp::DSP3D> ChannelConfig::get3DEffect(){
		return sp_dsp3d;
	}*/
	void ChannelConfig::set(const ChannelConfig & other){
		setOnlyChannelConfig(other);
		sp_noteProcessor=other.sp_noteProcessor;
		sp_instrument=other.sp_instrument;
		sp_dsp3d=other.sp_dsp3d;
		sp_velocityMap=other.sp_velocityMap;
		sp_tuning=other.sp_tuning;
		if(mixer == nullptr)mixer=other.mixer;
		if(channel == nullptr)channel=other.channel;
	}
	std::shared_ptr<ChannelConfig> ChannelConfig::DefaultConfig(){
		std::shared_ptr<ChannelConfig> p=std::make_shared< ChannelConfig>();
		p->setNoteTuning(std::make_shared<EqualTemperament>());
		p->setInstrumentProvider(std::make_shared<SimpleMIDIInstrument>());
		p->setNoteVelocityMap(std::make_shared<PowInterpolator>(2));
		return p;
	}
}