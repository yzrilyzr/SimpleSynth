#include "Channel.h"
#include "events/ChannelEvent.h"
#include "events/Note.h"
#include "events/NoteUpdater.h"
#include "interface/NoteProcessor.h"
#include "synth/source/AmpBuilderProcessor.h"
#include "interface/InstrumentProvider.h"
#include "lang/System.h"
#include "util/Util.h"
#include "util/MIDIFile.h"
#include "util/Random.h"
#include "SynthUtil.h"
#include "Mixer.h"
#include "NRPN.h"
#include "synth/envelopers/EnvelopMultiplier.h"
#include "dsp/AmpMultiply.h"
#include "dsp/DSPChain.h"
#include "dsp/Chorus.h"
#include "dsp/Delayer.h"
#include "dsp/Phaser.h"
#include "dsp/HRIR.h"
#include "dsp/Limiter.h"
#include "dsp/Freeverb.h"

using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_collection;
using namespace yzrilyzr_array;
using namespace yzrilyzr_lang;

namespace yzrilyzr_simplesynth{
	Note * NotePool::newInstance(){
		return new Note(uniqueID++);
	}
	void NotePool::reset(){
		uniqueID=0;
	}
	void NotePool::onReuse(Note * note){}
	void Channel::setChannelId(s_midichannel_id id){
		this->channelID=id;
		if(id == 9){
			setDrumSetChannel(true);
			noteProcessor=getInstrumentProvider()->getDrumSet(channelConfig.Bank, getSampleRate());
		}
	}
	u_time_f Channel::getProcessTime() const{
		return processTime;
	}
	void Channel::setSustain(bool sus){
		this->channelConfig.Sustain=sus;
	}
	void Channel::sendPostEvent(ChannelEvent * n1, u_time startAt){
		n1->startAtTime=startAt;
		std::unique_lock<std::recursive_mutex > lock(eventLock);
		postEventQueue.add(n1);
	}

	void Channel::setSampleRate(u_sample_rate sr){
		this->channelConfig.sampleRate=sr;
		for(int i=0;i < channelCount;i++){
			dspChain[i]->init(sr);
			limiter[i]->init(sr);
		}
	}
	void Channel::reset(){
		std::unique_lock<std::recursive_mutex > lock1(noteLock);
		workingNotesPool.clear();
		workingNotesPool.reset();
		std::unique_lock<std::recursive_mutex > lock(eventLock);
		postEventQueue.clear();
		instantEventQueue.clear();
		for(int i=0;i < channelCount;i++){
			SampleArray & outputBuf=*output[i];
			memset(outputBuf._array, 0, sizeof(u_sample) * outputBuf.length);
		}
		for(int ch=0;ch < channelCount;ch++){
			dspChain[ch]->resetMemory();
			limiter[ch]->resetMemory();
		}
		resetChannel();
	}
	bool Channel::hasData(){
		if(!postEventQueue.isEmpty() || !instantEventQueue.isEmpty()) return true;
		return !workingNotesPool.isEmpty();
	}
	void Channel::noteOff(uint8_t noteId){
		sendInstantEvent(new NoteOff(noteId));
	}
	void Channel::setBufferSize(size_t bs){
		for(int i=0;i < channelCount;i++){
			output[i]=std::make_shared<SampleArray>(bs);
		}
	}
	size_t Channel::getBufferSize()const{
		return output[0]->length;
	}
	Channel::~Channel(){
		delete hrir;
	}
	Channel::Channel(){
		channelConfig.channel=this;
		//
		dspChain[0]=std::make_shared<DSPChain>();
		dspChain[1]=std::make_shared<DSPChain>();
		//
		panner[0]=std::make_shared<AmpMultiply>();
		panner[1]=std::make_shared<AmpMultiply>();
		//
		choruser[0]=std::make_shared<Chorus>(0.027, 30, 0.3, 0);
		choruser[1]=std::make_shared<Chorus>(0.021, 30, 0.3, 0);
		//
		phaser[0]=std::make_shared<Phaser>(0.5, 2.0, 0.0, 0.3, 4);
		phaser[1]=std::make_shared<Phaser>(0.5, 2.0, 0.0, 0.3, 4);
		//
		reverber[0]=std::make_shared<Freeverb>(0.0);
		reverber[1]=std::make_shared<Freeverb>(0.0);
		//
		limiter[0]=std::make_shared<Limiter>(5,300, 500, 0.707, EnvelopDetector::RMS, 200);
		limiter[1]=std::make_shared<Limiter>(5,300, 500, 0.707, EnvelopDetector::RMS, 200);
		//
		addDSPToChain(panner);
		addDSPToChain(choruser);
		addDSPToChain(phaser);
		addDSPToChain(reverber);
		Random rand;
		getChorus(0).setInitPhase(rand.nextDouble());
		getChorus(1).setInitPhase(rand.nextDouble());
		lastActiveTime=System::currentTimeMillis();
	}
	void Channel::fillBuffer(){
		int64_t t=System::nanoTime();
		if(channelConfig.tuning == nullptr)channelConfig.tuning=mixer->getNoteTuning().get();
		u_sample_rate s_sampleRate=this->getSampleRate();
		u_time invSampleRate=1.0 / (u_time)s_sampleRate;
		u_sample_rate skipSample=1;//mixer->getSkipSample();
		u_time s_currentDeltaTime=skipSample * invSampleRate;
		this->channelConfig.deltaTime=s_currentDeltaTime;
		std::unique_lock<std::recursive_mutex > lock1(noteLock);
		u_time_f setEventDeltaTime=this->eventProcessDeltaTime;
		u_time_f s_eventTimeSum=this->eventTimeSum;
		if(channelConfig.noteProcessor == nullptr){
			workingNotesPool.returnAllObject();
		}
		s_sample_index mixerCurrentSampleIndex=mixer->getCurrentSampleIndex();
		int blen=getBufferSize();
		int chCount=channelCount;
		int channelCurrentSampleIndex=0;
		u_sample * s_outputL=output[0]->_array;
		u_sample * s_outputR=output[1]->_array;
		for(int sample=0;sample < blen;sample++){
			channelConfig.currentTime=(u_time)(mixerCurrentSampleIndex + channelCurrentSampleIndex) * invSampleRate;
			s_eventTimeSum+=(u_time_f)s_currentDeltaTime;
			if(s_eventTimeSum > setEventDeltaTime){
				std::unique_lock <std::recursive_mutex > lock(eventLock);
				{
					LinkedList<ChannelEvent *>::LinkIterator eventIterator(&postEventQueue);
					while(eventIterator.hasNext()){
						ChannelEvent * event=eventIterator.next();
						if(channelConfig.currentTime < event->startAtTime) break;
						eventIterator.remove();
						procEvent(*event);
						delete event;
					}
				}
				{
					LinkedList<ChannelEvent *>::LinkIterator eventIterator(&instantEventQueue);
					while(eventIterator.hasNext()){
						ChannelEvent * event=eventIterator.next();
						if(channelConfig.currentTime < event->startAtTime) break;
						eventIterator.remove();
						procEvent(*event);
						delete event;
					}
				}
				s_eventTimeSum=0;
				checkSostenuto();
				checkSustainState();
			}
			/*channelCurrentSampleIndex+=skipSample;
		}
		channelCurrentSampleIndex=0;
		for(int sample=0;sample < blen;sample++){
			currentTime=(u_time)(mixerCurrentSampleIndex + channelCurrentSampleIndex) * invSampleRate;*/
			u_sample outputf=0;
			if(channelConfig.noteProcessor != nullptr){
				NoteProcessor & np=*channelConfig.noteProcessor;
				for(int i=0;i < workingNotesPool.size();i++){
					auto & po=workingNotesPool.get(i);
					Note & note=*po.object;
					if(note.noMoreData)continue;
					NoteUpdater::preUpdateNote(note, channelConfig);
					u_sample noteout=np.getAmp(note);
					NoteUpdater::postUpdateNote(note, channelConfig);
					if(isnan(noteout) || std::abs(noteout) > 50){
						std::cout << "Note output Exception" << std::endl;
						np.getAmp(note);
					}
					outputf+=noteout;
					if(np.noMoreData(note)){
						note.noMoreData=true;
					}
				}
				outputf=(u_sample)np.postProcess(outputf);
				if(isnan(outputf) || std::abs(outputf) > 50){
					std::cout << "PostProcess output Exception" << std::endl;
					np.postProcess(outputf);
				}
				outputf*=channelConfig.Volume;
			}
			channelCurrentSampleIndex+=skipSample;
			s_outputL[sample]=outputf;
		}
		for(int i=0;i < workingNotesPool.size();i++){
			auto & po=workingNotesPool.get(i);
			Note & note=*po.object;
			if(note.noMoreData){
				workingNotesPool.returnObject(po);
				i--;
			}
		}
		memcpy(s_outputR, s_outputL, sizeof(u_sample) * blen);
		if(hrir == nullptr){
			std::dynamic_pointer_cast<AmpMultiply>(panner[0])->setValue(Util::clamp01(1 - channelConfig.Pan));
			std::dynamic_pointer_cast<AmpMultiply>(panner[1])->setValue(Util::clamp01(1 + channelConfig.Pan));
			std::dynamic_pointer_cast<DSP>(dspChain[0])->procBlock(s_outputL, blen);
			std::dynamic_pointer_cast<DSP>(dspChain[1])->procBlock(s_outputR, blen);
		} else{
			hrir->setChannel(0);
			hrir->procBlock(s_outputL, blen);
			hrir->setChannel(1);
			hrir->procBlock(s_outputR, blen);
		}
		if(mixer->isUseLimiter()){
			std::dynamic_pointer_cast<DSP>(limiter[0])->procBlock(s_outputL, blen);
			std::dynamic_pointer_cast<DSP>(limiter[1])->procBlock(s_outputR, blen);
		}
		if(isnan(s_outputL[0]) || isnan(s_outputL[1])){
			std::cout << "Output NaN" << std::endl;
			reset();
		}
		if(workingNotesPool.size() != 0){
			lastActiveTime=System::currentTimeMillis();
		}
		this->eventTimeSum=s_eventTimeSum;
		processTime=(u_time_f)((u_time_f)(System::nanoTime() - t) / 1000000000.0f);
	}
	HRIR & Channel::getHRIR(){
		return *hrir;
	}
	void Channel::setHRIR(HRIR * phrir){
		this->hrir=phrir;
	}
	void Channel::procEvent(ChannelEvent & event){
		lastActiveTime=System::currentTimeMillis();
		switch(event.getType()){
			case EventType::NOTE_ON:
			{
				procNoteOn(static_cast<NoteOn &>(event));
				break;
			}
			case EventType::NOTE_OFF:
			{
				procNoteOff(static_cast<NoteOff &>(event));
				break;
			}
			case EventType::NOTE_PITCH_BEND:
			{
				procNotePitchBend(static_cast<NotePitchBend &>(event));
				break;
			}
			case EventType::NOTE_PRESSURE:
			{
				procNotePressure(static_cast<NotePressure &>(event));
				break;
			}
			case EventType::CHANNEL_CONTROL:
			{
				procChannelControl(static_cast<ChannelControl &>(event));
				break;
			}
			case EventType::CHANNEL_PITCH_BEND:
			{
				setPitchBend(static_cast<ChannelPitchBend &>(event).value);
				break;
			}
			case EventType::CHANNEL_PRESSURE:
			{
				procChannelPressure(static_cast<ChannelPressure &>(event).value);
				break;
			}
			case EventType::CHANNEL_PROGRAM_CHANGE:
			{
				procInstrument(static_cast<ProgramChange &>(event));
				break;
			}
		}
	}

	void Channel::resetChannel(){		
		channelConfig.reset();
	}
	void Channel::setModDelay(u_time_f v){
		this->channelConfig.ModDelay=v;
	}
	void Channel::setModDepth(float v){
		this->channelConfig.ModDepth=v;
	}
	void Channel::setModRate(float v){
		this->channelConfig.ModRate=v;
	}
	void Channel::setPitchBend(float pitchBend1){
		this->channelConfig.ChannelPitchBend=pitchBend1;
	}
	void Channel::setLegato(bool legato){
		this->channelConfig.Legato=legato;
	}
	void Channel::setSoftPedal(bool softPedal){
		this->channelConfig.SoftPedal=softPedal;
	}
	size_t Channel::getPostedEventCount()const{
		return postEventQueue.size() + instantEventQueue.size();
	}
	void Channel::procNoteOn(NoteOn & noteOn){
		if(Note::idInvalid(noteOn.id)) return;
		if(noteOn.velocity == static_cast<s_note_vel>(0)){
			NoteOff off1(noteOn.id);
			procNoteOff(off1);
			return;
		}
		if(channelConfig.noteProcessor != nullptr)channelConfig.noteProcessor->noteOn(channelConfig, noteOn.id, noteOn.velocity);
		if(channelConfig.MonoMode){
			for(int i=0;i < workingNotesPool.size();i++){
				auto & po=workingNotesPool.get(i);
				Note & n=*po.object;
				n.forceClose(channelConfig);
			}
		}
		for(int i=0;i < workingNotesPool.size();i++){
			auto & po=workingNotesPool.get(i);
			Note & n=*po.object;
			if(n.id == noteOn.id){
				n.forceClose(channelConfig);
			}
		}
		Note * pnote=nullptr;
		if(workingNotesPool.size() >= workingNotesPool.capacity()){
			std::cout << "NotePool is full" << std::endl;
			setSostenuto(false);
			setSustain(false);
			Note * maxNote=nullptr;
			for(int i=0;i < workingNotesPool.size();i++){
				auto & po=workingNotesPool.get(i);
				Note & n=*po.object;
				if(maxNote == nullptr || n.passedTime > maxNote->passedTime){
					maxNote=&n;
				}
			}
			pnote=maxNote;
		} else{
			pnote=workingNotesPool.borrowObject();
		}
		if(pnote == nullptr)return;
		Note & note=*pnote;
		note.cfg=&channelConfig;
		NoteUpdater::noteOn(note, channelConfig, noteOn.id, noteOn.velocity);
	}
	void Channel::setAlwaysActive(bool v){
		alwaysActive=v;
	}
	void Channel::procNoteOff(NoteOff & note){
		if(Note::idInvalid(note.id)) return;
		channelConfig.noteHoldMap[note.id]=false;
		//延音或选择延音状态，忽略关闭
		if(channelConfig.Sustain || channelConfig.sostenutoLock[note.id] || isDrumSet){
			return;
		}
		if(channelConfig.noteProcessor != nullptr)channelConfig.noteProcessor->noteOff(channelConfig, note.id, note.velocity);
		for(int i=0;i < workingNotesPool.size();i++){
			auto & po=workingNotesPool.get(i);
			Note & n=*po.object;
			if(n.id == note.id){
				NoteUpdater::noteOff(n, channelConfig, note.velocity);
			}
		}
	}
	void Channel::procNotePressure(NotePressure & note){
		if(Note::idInvalid(note.id)) return;
		for(int i=0;i < workingNotesPool.size();i++){
			auto & po=workingNotesPool.get(i);
			Note & n=*po.object;
			if(note.id == n.id){
				n.velocity=note.velocity;
			}
		}
	}
	void Channel::procNotePitchBend(NotePitchBend & note){
		if(Note::idInvalid(note.id)) return;
		for(int i=0;i < workingNotesPool.size();i++){
			auto & po=workingNotesPool.get(i);
			Note & n=*po.object;
			n.pitchBend=note.value;
		}
	}
	void Channel::procChannelPressure(s_note_vel value){
		for(int i=0;i < workingNotesPool.size();i++){
			auto & po=workingNotesPool.get(i);
			Note & n=*po.object;
			n.velocity=value;
		}
	}
	void Channel::procChannelControl(ChannelControl & cc){
		if(!ENABLE_MIDI_CHANNEL_CONTROL)return;
		//System.out.println("MIDI CC:"+cc.control+" Value:"+cc.value);
		switch(cc.control){
			case MIDIFile::CC::VOLUME:
				setVolume(std::pow(cc.value / 127.0f, 2.0f));
				break;
			case MIDIFile::CC::PAN:
				if(cc.value == 64) setPan(0);
				else if(cc.value < 64) setPan((cc.value - 64.0f) / 64.0f);
				else setPan((cc.value - 64.0f) / 63.0f);
				break;
			case MIDIFile::CC::SUSTAIN_SWITCH:
				setSustain(cc.value >= 64);
				break;
			case MIDIFile::CC::RESET_ALL_CONTROLLERS:
				resetChannel();
				break;
			case MIDIFile::CC::RESET_MUTE_ALL_NOTES:
				workingNotesPool.clear();
				workingNotesPool.reset();
				break;
			case MIDIFile::CC::ALL_NOTES_OFF:
			{
				channelConfig.allNotesOff();
				bool offMap[CHANNEL_MAX_NOTE_ID]{false};
				for(int i=0;i < workingNotesPool.size();i++){
					auto & po=workingNotesPool.get(i);
					Note & n=*po.object;
					n.requestClose(channelConfig);
					offMap[n.id]=true;
				}
				if(channelConfig.noteProcessor != nullptr){
					for(int i=0;i < CHANNEL_MAX_NOTE_ID;i++){
						if(offMap[i])channelConfig.noteProcessor->noteOff(channelConfig, i, 0);
					}
				}
			}
			break;
			case MIDIFile::CC::EFFECT_REVERB:
				if(ENABLE_MIDI_CC_EFFECT)setReverb(cc.value / 127.0f);
				break;
			case MIDIFile::CC::EFFECT_CHORUS:
				if(ENABLE_MIDI_CC_EFFECT)setChorus(cc.value / 127.0f);
				break;
			case MIDIFile::CC::EFFECT_DETUNE:
				if(ENABLE_MIDI_CC_EFFECT)setDetune(cc.value / 127.0f);
				break;
			case MIDIFile::CC::EFFECT_PHASER:
				if(ENABLE_MIDI_CC_EFFECT)setPhaser(cc.value / 127.0f);
				break;
			case MIDIFile::CC::RPN_MSB:
				channelConfig.nrpn.reset();
				channelConfig.rpn.reset();
				channelConfig.rpn.selectMSB(cc.value & 0x7f);
				break;
			case MIDIFile::CC::RPN_LSB:
				channelConfig.rpn.selectLSB(cc.value & 0x7f);
				break;
			case MIDIFile::CC::NRPN_MSB:
				channelConfig.rpn.reset();
				channelConfig.nrpn.reset();
				channelConfig.nrpn.selectMSB(cc.value & 0x7f);
				break;
			case MIDIFile::CC::NRPN_LSB:
				channelConfig.rpn.reset();
				channelConfig.nrpn.selectLSB(cc.value & 0x7f);
				break;
			case MIDIFile::CC::DATA_ENTRY_MSB://msb先来
				channelConfig.rpn.setDataMSB(cc.value & 0x7f);
				channelConfig.nrpn.setDataMSB(cc.value & 0x7f);
				procDataEntry();
				break;
			case MIDIFile::CC::DATA_ENTRY_LSB:
				channelConfig.rpn.setDataLSB(cc.value & 0x7f);
				channelConfig.nrpn.setDataLSB(cc.value & 0x7f);
				procDataEntry();
				break;
			case MIDIFile::CC::EXPRESSION:
				setExpression(std::pow(cc.value / 127.0f, 2.0f));
				break;
			case MIDIFile::CC::BREATH:
				setBreath(std::pow(cc.value / 127.0f, 2.0f));
				break;
			case MIDIFile::CC::FOOT:
				setFoot(std::pow(cc.value / 127.0f, 2.0f));
				break;
			case MIDIFile::CC::BANK:
				channelConfig.Bank=(cc.value & 0x7f) << 7;
				break;
			case MIDIFile::CC::BANK + 32:
				channelConfig.Bank|=(cc.value & 0x7f);
				break;
			case MIDIFile::CC::MONO_MODE:
				setMonoMode(true);
				break;
			case MIDIFile::CC::POLY_MODE:
				setMonoMode(false);
				break;
			case MIDIFile::CC::MODULATION:
				setModulation(cc.value / 127.0f);
				break;
			case MIDIFile::CC::PORTAMENTO_TIME:
				setPortamentoTime((std::pow(10.0f, cc.value / 127.0f) - 1.0f) / 9.0f);
				break;
			case MIDIFile::CC::PORTAMENTO_SWITCH:
				setPortamento(cc.value >= 64);
				break;
			case MIDIFile::CC::LEGATO_EFFECT_SWITCH:
				setLegato(cc.value >= 64);
				break;
			case MIDIFile::CC::SOSTENUTO_SWITCH:
				setSostenuto(cc.value >= 64);
				break;
			case MIDIFile::CC::SOFT_PEDAL_SWITCH:
				setSoftPedal(cc.value >= 64);
				break;
			case MIDIFile::CC::VIBRATO_RATE:
				setModRate(std::pow(10.0f, cc.value / 127.0f) - 1.0f);
				break;
			case MIDIFile::CC::VIBRATO_DEPTH:
				setModDepth(cc.value / 127.0f);
				break;
			case MIDIFile::CC::VIBRATO_DELAY:
				setModDelay((std::pow(10.0f, cc.value / 127.0f) - 1.0f) / 9.0f);
				break;
			case MIDIFile::CC::ATTACK_TIME:
				if(ENABLE_MIDI_CC_ADSR){
					std::shared_ptr<AHDSREnvelop> a=getAHDSREnv();
					if(a){
						a->attackTime=pow(10000.0, cc.value / 127.0f) / 1000.0;
					}
				}
				break;
			case MIDIFile::CC::DECAY_TIME:
				if(ENABLE_MIDI_CC_ADSR){
					std::shared_ptr<AHDSREnvelop> a=getAHDSREnv();
					if(a){
						a->decayTime=pow(10000.0, cc.value / 127.0f) / 1000.0;
					}
				}
				break;
			case MIDIFile::CC::RELEASE_TIME:
				if(ENABLE_MIDI_CC_ADSR){
					std::shared_ptr<AHDSREnvelop> a=getAHDSREnv();
					if(a){
						a->releaseTime=pow(10000.0, cc.value / 127.0f) / 1000.0;
					}
				}
				break;
			default:
				std::cout << "Unimplemented Control: " << (int)cc.control << "=" << (int)cc.value << std::endl;
				break;
		}
		if(channelConfig.noteProcessor != nullptr) channelConfig.noteProcessor->cc(channelConfig, cc);
	}
	// 音符状态管理
	void Channel::closeNotSustainNotes(){
		// 关闭未被延音保持的音符
		if(channelConfig.Sustain) return;
		bool offMap[CHANNEL_MAX_NOTE_ID]{false};

		for(int i=0; i < workingNotesPool.size(); i++){
			auto & po=workingNotesPool.get(i);
			Note & n=*po.object;
			if(channelConfig.sostenutoLock[n.id]) continue;
			if(!channelConfig.noteHoldMap[n.id]){
				offMap[n.id]=true;
				n.requestClose(channelConfig);
			}
		}
		if(channelConfig.noteProcessor != nullptr){
			for(int i=0;i < CHANNEL_MAX_NOTE_ID;i++){
				if(offMap[i])channelConfig.noteProcessor->noteOff(channelConfig, i, 0);
			}
		}
	}
	std::shared_ptr<AHDSREnvelop> Channel::getAHDSREnv()const{
		auto a=dynamic_cast<EnvelopMultiplier*>(channelConfig.noteProcessor);
		if(!a)return nullptr;
		auto b=std::dynamic_pointer_cast<AHDSREnvelop>(a->a);
		return b;
	}
	void Channel::procDataEntry(){
		PNData & rpn=channelConfig.rpn;
		if(rpn.active){
			switch(rpn.select){
				case 0://setPitchBendRange
					setPitchBendRange((float)(rpn.dataMSB + rpn.dataLSB / 100.0));
					break;
				case 1://FineTune
					channelConfig.FineTune=(float)((rpn.data - 8192.0) / 8192.0);
					break;
				case 2://CoarseTune
					channelConfig.CoarseTune=rpn.dataMSB - 64;
					break;
					//default:
					//	System.out.println("Unimplemented RPN:"+rpnController+" = "+pnDataValue);
					//	break;
			}
		} else if(channelConfig.nrpn.active){
			procNRPN(true, channelConfig.nrpn.select, channelConfig.nrpn.data);
		}
	}
	void Channel::procInstrument(ProgramChange & event){
		if(!ENABLE_MIDI_PROGRAM_CHANGE)return;
		NoteProcPtr src=nullptr;
		std::shared_ptr<InstrumentProvider> instr=getInstrumentProvider();
		if(event.noteProcessor != nullptr){
			src=event.noteProcessor;
		} else if(instr == nullptr){
			std::cout << "Midi Instrument not set" << std::endl;
			src=SynthUtil::getDefault();
		} else if(channelID == IMixer::MIDI_DRUM_CHANNEL){
			src=instr->getDrumSet(channelConfig.Bank, getSampleRate());
			setDrumSetChannel(true);
		} else{
			src=instr->get(channelConfig.Bank, event.id, getSampleRate());
			if(src == nullptr){
				std::cout << "ProgramChange[Not found]: " << std::to_string(event.id) << ", CH:" << channelID << std::endl;
				src=SynthUtil::getDefault();
			}
		}
		if(src != nullptr){
			channelConfig.setNoteProcessor(src);
			src->init(channelConfig);
		}
	}

	bool Channel::isSoftPedal() const{
		return channelConfig.SoftPedal;
	}
	bool Channel::isSostenuto() const{
		return channelConfig.Sostenuto;
	}
	u_time Channel::getCurrentTime() const{
		return channelConfig.currentTime;
	}
	size_t Channel::getCurrentProcessingNoteCount()const{
		return workingNotesPool.size();
	}
	void Channel::setMonoMode(bool monoMode){
		this->channelConfig.MonoMode=monoMode;
	}
	void Channel::setModulation(u_normal_01_f v){
		this->channelConfig.Modulation=v;
	}
	void Channel::setExpression(u_normal_01_f i){
		this->channelConfig.Expression=i;
	}
	void Channel::setBreath(u_normal_01_f i){
		this->channelConfig.Breath=i;
	}
	u_normal_01_f Channel::getFoot()const{
		return this->channelConfig.Foot;
	}
	void Channel::setFoot(u_normal_01_f i){
		this->channelConfig.Foot=i;
	}
	void Channel::setPitchBendRange(s_note_id pitchBendRange){
		this->channelConfig.PitchBendRange=pitchBendRange;
	}
	void Channel::setVolume(u_normal_01_f volume){
		this->channelConfig.Volume=volume;
	}
	void Channel::addDSPToChain(std::shared_ptr<yzrilyzr_dsp::DSP> * dsp){
		for(int i=0;i < 2;i++){
			dspChain[i]->add(dsp[i]);
		}
	}
	u_sample_rate Channel::getSampleRate()const{
		return mixer->getSampleRate();
	}
	void Channel::setPan(u_normal_11_f pan){
		this->channelConfig.Pan=pan;
	}
	void Channel::setNoteShift(int8_t noteShift){
		this->channelConfig.NoteShift=noteShift;
	}
	Chorus & Channel::getChorus(size_t ch)const{
		return *(std::dynamic_pointer_cast<Chorus>(choruser[ch]));
	}
	Phaser & Channel::getPhaser(size_t ch)const{
		return *(std::dynamic_pointer_cast<Phaser>(phaser[ch]));
	}
	Freeverb & Channel::getReverb(size_t ch)const{
		return *(std::dynamic_pointer_cast<Freeverb>(reverber[ch]));
	}
	void Channel::setChorus(u_normal_01_f chorus){
		//std::cout << "Ch:" << channelID << "	Chorus:" << chorus << std::endl;
		for(int i=0;i < channelCount;i++){
			auto & au=getChorus(i);
			au.depthMs=50.0f * chorus;
			au.init(getSampleRate());
		}
	}
	void Channel::setReverb(u_normal_01_f reverb){
		//std::cout << "Ch:" << channelID << "	Reverb:" << reverb << std::endl;
		for(int i=0;i < channelCount;i++){
			auto & au=getReverb(i);
			au.roomSize=reverb * 0.9f;
			au.damper=Util::clamp(0.3f - reverb * 0.3f, 0.0f, 0.3f);
			au.wetRatio=Util::clamp(reverb, 0.0f, 0.5f);
			au.init(getSampleRate());
		}
	}
	void Channel::setDetune(u_normal_01_f detune){
		//std::cout << "Ch:" << channelID << "	Detune:" << detune << std::endl;
		this->channelConfig.Detune=detune;
	}
	void Channel::setPhaser(u_normal_01_f cphase){
		//std::cout << "Ch:" << channelID << "	Phaser:" << cphase << std::endl;
		for(int i=0; i < channelCount; i++){
			auto & au=getPhaser(i);
			au.wetRatio=cphase;
			au.init(getSampleRate());
		}
	}
	void Channel::setSostenuto(bool sostenuto){
		this->channelConfig.Sostenuto=sostenuto;
	}
	void Channel::setPortamento(bool b){
		this->channelConfig.Portamento=b;
	}
	void Channel::setPortamentoTime(u_time_f v){
		this->channelConfig.PortamentoTime=v;
	}
	void Channel::procNRPN(bool lsb, uint16_t nrpnController, uint16_t value){
		switch(nrpnController){
			case NRPN::MIXER_LIMITER:
				mixer->setUseLimiter(value >= 64);
				break;
			case NRPN::BUILDER_START:
				//if(value == 127)setNoteProcessor(std::make_shared<AmpBuilderProcessor>());
				break;
			default:
				//System.out.println("Unimplemented NRPN:"+nrpnController+" = "+value);
				break;
		}
	}

	void Channel::noteOn(uint8_t noteId, s_note_vel velocity){
		sendInstantEvent(new NoteOn(noteId, velocity));
	}
	void Channel::sendInstantEvent(ChannelEvent * n1){
		n1->startAtTime=channelConfig.currentTime;
		std::unique_lock<std::recursive_mutex > lock(eventLock);
		instantEventQueue.add(n1);
	}
	bool Channel::isSustain() const{
		return channelConfig.Sustain;
	}
	PNData & Channel::getRPN(){
		return channelConfig.rpn;
	}
	PNData & Channel::getNRPN(){
		return channelConfig.nrpn;
	}
	bool Channel::isLegato() const{
		return channelConfig.Legato;
	}
	bool Channel::isPortamento() const{
		return channelConfig.Portamento;
	}
	u_time_f Channel::getPortamentoTime() const{
		return channelConfig.PortamentoTime;
	}
	bool Channel::isMonoMode() const{
		return channelConfig.MonoMode;
	}
	u_time_f Channel::getEventDeltaTime() const{
		return eventProcessDeltaTime;
	}
	void Channel::setEventDeltaTime(u_freq Hz){
		this->eventProcessDeltaTime=static_cast<u_time_f>(1.0 / Hz);
	}
	u_normal_01_f Channel::getModulation() const{
		return channelConfig.Modulation;
	}
	u_normal_01_f Channel::getExpression() const{
		return channelConfig.Expression;
	}
	u_normal_01_f Channel::getBreath() const{
		return channelConfig.Breath;
	}
	s_midichannel_id Channel::getChannelId() const{
		return channelID;
	}
	u_normal_01_f Channel::getPan() const{
		return channelConfig.Pan;
	}
	s_note_id Channel::getNoteShift() const{
		return channelConfig.NoteShift;
	}
	s_note_id Channel::getPitchBendRange() const{
		return channelConfig.PitchBendRange;
	}
	u_normal_01_f Channel::getVolume() const{
		return channelConfig.Volume;
	}
	u_normal_01_f Channel::getPitchBend() const{
		return channelConfig.ChannelPitchBend;
	}
	u_normal_01_f Channel::getDetune() const{
		return channelConfig.Detune;
	}
	bool Channel::isDrumSetChannel()const{
		return isDrumSet;
	}
	void Channel::setDrumSetChannel(bool value){
		isDrumSet=value;
	}
	void Channel::checkSostenuto(){
		// 检查选择性延音状态变化
		if(lastSostenutoState == channelConfig.Sostenuto) return;
		lastSostenutoState=channelConfig.Sostenuto;
		channelConfig.sostenutoChange();
		closeNotSustainNotes();
	}
	void Channel::checkSustainState(){
		// 检查延音状态变化
		if(lastSustainState == channelConfig.Sustain) return;
		lastSustainState=channelConfig.Sustain;
		closeNotSustainNotes();
	}
	u_sample * Channel::getOutput(uint32_t chIndex)const{
		return output[chIndex]->_array;
	}
	ChannelConfig & Channel::getConfig(){
		return channelConfig;
	}
}