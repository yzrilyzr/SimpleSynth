#include "Mixer.h"
#include "NRPN.h"
#include "SynthUtil.h"
#include "dsp/AmpMultiply.h"
#include "dsp/Chorus.h"
#include "dsp/DSP3D.h"
#include "dsp/DSPChain.h"
#include "dsp/Freeverb.h"
#include "dsp/Limiter.h"
#include "dsp/Phaser.h"
#include "events/ChannelEvent.h"
#include "events/Note.h"
#include "events/NoteUpdater.h"
#include "interface/InstrumentProvider.h"
#include "interface/NoteProcessor.h"
#include "lang/Runtime.h"
#include "lang/System.h"
#include "lang/Thread.h"
#include "synth/envelopers/EnvelopMultiplier.h"
#include "util/MIDIFile.h"
#include "util/Random.h"
#include "util/Util.h"
#include <future>
#include <vector>
using namespace yzrilyzr_array;
using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_collection;
using namespace yzrilyzr_lang;


namespace yzrilyzr_simplesynth{
	Mixer::Mixer(u_index bufferSize){
		setBufferSize(bufferSize);
		setSynthMode(MODE_SINGLE_THREAD, -1);
		auto defCfg=ChannelConfig::DefaultConfig();
		getGlobalConfig().set(*defCfg);
		nonDrumSetLimiter=new Limiter * [2]{new Limiter(5, 5000, 5000, 1), new Limiter(5, 5000, 5000, 1)};
		masterLimiter=new Limiter * [2]{new Limiter(5, 5000, 5000, 1), new Limiter(5, 5000, 5000, 1)};
		finalEQ[0]=mksp<DSPChain>();
		finalEQ[1]=mksp<DSPChain>();
		//finalEQ[0]->add(IIRUtil::newBiquadIIRFilter(100, 44100, 1.5,5, FilterPassType::LOWSHELF));
		//finalEQ[1]->add(IIRUtil::newBiquadIIRFilter(100, 44100, 1.5,5, FilterPassType::LOWSHELF));
	}
	Mixer::~Mixer(){
		removeAllChannels();
		delete nonDrumSetLimiter[0];
		delete nonDrumSetLimiter[1];
		delete[] nonDrumSetLimiter;
		delete masterLimiter[0];
		delete masterLimiter[1];
		delete[] masterLimiter;
		delete threadPool;
	}
	void Mixer::setBufferSize(u_index bs){
		for(int32_t i=0;i < getOutputChannelCount();i++){
			output[i]=SampleArray(bs);
		}
	}
	void Mixer::setSynthMode(int8_t mode, int32_t cores){
		this->synthMode=mode;
		if(cores == -1) cores=Runtime::getRuntime().availableProcessors();
		if(mode == MODE_FUTURE);
		else if(synthMode == MODE_THREAD_POOL){
			if(threadPool != nullptr)delete threadPool;
			threadPool=new FixedThreadPool(cores);
		}
	}
	void fillBuffer1(u_sp<Channel> c){
		Thread::currentThread()->setPriority(Thread::Priority::HIGH);
		c->fillBuffer();
	}
	void Mixer::commitChannels(){
		if(_pause) return;
		switch(synthMode){
			case MODE_SINGLE_THREAD:
			{
				std::unique_lock<std::recursive_mutex > lock(channelLock);
				for(u_sp<Channel> mix : channels){
					mix->fillBuffer();
				}
				break;
			}
			case MODE_THREAD_POOL:
			{
				std::unique_lock<std::recursive_mutex > lock(channelLock);
				for(u_sp<Channel> mix : channels){
					threadPool->commit([mix](){
						Thread::currentThread()->setPriority(Thread::Priority::HIGH);
						mix->fillBuffer();
					});
				}
				break;
			}
			case MODE_FUTURE:
			{
				std::unique_lock<std::recursive_mutex > lock(channelLock);
				for(u_sp<Channel> mix : channels){
					futures.push_back(std::async(std::launch::async, fillBuffer1, mix));
				}
				break;
			}
		}
	}
	void Mixer::waitForChannels(){
		if(_pause) return;
		switch(synthMode){
			case MODE_THREAD_POOL:
			{
				threadPool->waitAll();
				break;
			}
			case MODE_FUTURE:
			{
				for(auto & fu : futures){
					fu.get();
				}
				futures.clear();
				break;
			}
		}
	}
	void Mixer::mixChannels(){
		if(_pause) return;
		u_index blen=getBufferSize();
		u_index chCount=getOutputChannelCount();
		u_sample_rate sampleRate=getSampleRate();
		bool tUseEQ=useEQ;
		bool tUseLimiter=useLimiter;
		Limiter ** tLimiter=this->nonDrumSetLimiter;
		Limiter ** masterLimiter=this->masterLimiter;
		std::unique_lock<std::recursive_mutex> lock(channelLock);
		auto channelItr=channels.iterator();
		u_time_stamp now=System::currentTimeMillis();
		while(channelItr->hasNext()){
			u_sp<Channel> mix=channelItr->next();
			if(mix->hasData()){
				mix->lastActiveTime=now;
			}
			if(!mix->alwaysActive && now - mix->lastActiveTime > idleChannelLiveTime){
				std::cout << "Removed Idle Channel:" << mix->getChannelID() << std::endl;
				channelItr->remove();
			}
		}
		for(u_index ch=0;ch < chCount;ch++){
			u_sample * thisOutput=output[ch]._array;
			memset(thisOutput, 0, blen * sizeof(u_sample));
			for(auto & c : channels){
				u_sample * channelOutput=c->output[ch]._array;
				bool isDrumSet=IMixer::isDrumSetChannel(c->getConfig(), c->getChannelID());
				if(isDrumSet)continue;
				for(u_index sample=0;sample < blen;sample++){
					thisOutput[sample]+=channelOutput[sample];
				}
			}
			if(tUseLimiter) tLimiter[ch]->procBlock(thisOutput, blen);
			for(auto & c : channels){
				u_sample * channelOutput=c->output[ch]._array;
				bool isDrumSet=IMixer::isDrumSetChannel(c->getConfig(), c->getChannelID());
				if(!isDrumSet)continue;
				for(u_index sample=0;sample < blen;sample++){
					thisOutput[sample]+=channelOutput[sample];
				}
			}
			if(tUseEQ) finalEQ[ch]->procBlock(thisOutput, thisOutput, blen);
			if(tUseLimiter) masterLimiter[ch]->procBlock(thisOutput, blen);
		}
		currentSampleIndex+=blen;
		if(flags.hasAndRemove(FLAG_RESET_INDEX)){
			currentSampleIndex=0;
			skipSample=1;
		}
		if(flags.hasAndRemove(FLAG_RESET_LIMITER)){
			for(u_index ch=0;ch < chCount;ch++){
				nonDrumSetLimiter[ch]->resetMemory();
				masterLimiter[ch]->resetMemory();
			}
		}
		if(flags.hasAndRemove(FLAG_RESET_EQ)){
			for(u_index ch=0;ch < chCount;ch++){
				finalEQ[ch]->resetMemory();
			}
		}
		if(flags.hasAndRemove(FLAG_RESET_BUFFER)){
			for(u_index i=0;i < chCount;i++){
				SampleArray & outputBuf=output[i];
				memset(outputBuf._array, 0, sizeof(u_sample) * outputBuf.length);
			}
		}
	}
	void Mixer::mix(){
		u_time t=(u_time)System::nanoTime();
		commitChannels();
		waitForChannels();
		mixChannels();
		processTime=(u_time_f)((u_time_f)(System::nanoTime() - t) / 1000000000.0);
	}

	std::vector<u_sp<IChannel>> Mixer::getAllChannels()const{
		std::vector<u_sp<IChannel>> chann;
		for(auto & i : channels){
			chann.emplace_back(spsc<IChannel>(i));
		}
		return chann;
	}

	void Mixer::setSampleRate(u_sample_rate sr){
		IMixer::setSampleRate(sr);
		for(u_index ch=0;ch < getOutputChannelCount();ch++){
			if(useLimiter){
				nonDrumSetLimiter[ch]->init(sr);
				masterLimiter[ch]->init(sr);
			}
			if(useEQ)finalEQ[ch]->init(sr);
		}
	}
	void Mixer::setEQ(int32_t seg, double value){}
	u_sp<IChannel> Mixer::getMIDIChannel(const String & name, s_midichannel_id channelID){
		auto res=midiChannelMap.find({name, channelID});
		if(res == midiChannelMap.end()){
			auto channel=mksp<Channel>();
			channel->setName(name + " #" + std::to_string(channelID));
			if(IMixer::isDrumSetChannel(channel->getConfig(), channelID)) channel->setSustain(true);
			setMIDIChannel(name, channelID, channel);
			return spsc<IChannel>(channel);
		}
		return spsc<IChannel>(res->second);
	}
	u_sp<yzrilyzr_dsp::DSPChain> * Mixer::getEQ(){
		return finalEQ;
	}
	void Mixer::addChannel(u_sp<Channel> channel){
		std::unique_lock<std::recursive_mutex > lock(channelLock);
		channels.add(channel);
	}
	void Mixer::setMIDIChannel(s_midichannel_id id, u_sp<Channel>channel){
		setMIDIChannel(DEFAULT_MIDI_CHANNEL_GROUP_NAME, id, channel);
	}
	u_index Mixer::getBufferSize()const{
		return output[0].length;
	}
	void Mixer::setMIDIChannel(const String & name, s_midichannel_id id, u_sp<Channel>channel){
		channel->setBufferSize(getBufferSize());
		channel->setSampleRate(getSampleRate());
		channel->setChannelId(id);
		channel->getConfig().set(getGlobalConfig());
		addChannel(channel);
		std::unique_lock<std::recursive_mutex > lock(midiChannelMapLock);
		midiChannelMap[{name, id}]=channel;
	}
	void Mixer::removeMIDIChannel(s_midichannel_id channel){
		removeMIDIChannel(DEFAULT_MIDI_CHANNEL_GROUP_NAME, channel);
	}
	void Mixer::removeMIDIChannel(const String & name, s_midichannel_id channelID){
		std::unique_lock<std::recursive_mutex > lock(midiChannelMapLock);
		auto res=midiChannelMap.find({name, channelID});
		if(res != midiChannelMap.end()){
			u_sp<Channel> paramRegPtr=res->second;
			midiChannelMap.erase({name, channelID});
			removeChannel(paramRegPtr);
		}
	}
	void Mixer::removeChannel(u_sp<Channel> channel){
		std::unique_lock<std::recursive_mutex > lock(channelLock);
		channels.remove(channel);
	}
	void Mixer::removeAllChannels(){
		std::unique_lock<std::recursive_mutex > lock(channelLock);
		std::unique_lock<std::recursive_mutex > lock1(midiChannelMapLock);
		channels.clear();
		midiChannelMap.clear();
	}
	void Mixer::reset(){
		flags.add(FLAG_RESET_INDEX | FLAG_RESET_LIMITER | FLAG_RESET_BUFFER | FLAG_RESET_EQ);
		removeAllChannels();
	}
	void Mixer::resetLimiter(){
		flags.add(FLAG_RESET_LIMITER);
	}
	bool Mixer::hasData(){
		std::unique_lock<std::recursive_mutex > lock(channelLock);
		for(auto & c : channels){
			if(c->hasData()) return true;
		}
		return false;
	}
	u_index Mixer::getCurrentProcessingNoteCount(){
		u_index sum=0;
		std::unique_lock<std::recursive_mutex > lock(channelLock);
		for(auto & c : channels) sum+=c->getCurrentProcessingNoteCount();
		return sum;
	}
	u_index Mixer::getPostedEventCount(){
		u_index sum=0;
		std::unique_lock<std::recursive_mutex > lock(channelLock);
		for(auto & c : channels) sum+=c->getPostedEventCount();
		return sum;
	}
	void Mixer::pause(bool pause){
		this->_pause=pause;
	}
	bool Mixer::isPaused() const{
		return _pause;
	}
	u_time Mixer::getCurrentTime() const{
		return getCurrentSampleIndex() / (u_time)getSampleRate();
	}
	s_sample_index Mixer::getCurrentSampleIndex() const{
		return currentSampleIndex;
	}
	u_sample_rate Mixer::getSkipSample() const{
		return skipSample;
	}
	void Mixer::setSkipSample(u_sample_rate skip){
		this->skipSample=skip;
	}
	u_sample * Mixer::getOutput(uint32_t chIndex)const{
		return output[chIndex]._array;
	}
	void Mixer::sendInstantEvent(ChannelEvent * event){
		if(event->groupName.empty())event->groupName=DEFAULT_MIDI_CHANNEL_GROUP_NAME;
		u_sp<Channel> ch=spsc<Channel>(getMIDIChannel(event->groupName, event->channelID));
		ch->sendInstantEvent(event);
	}
	void Mixer::postEvent(ChannelEvent * event, u_time startAt){
		u_sp<Channel> ch=spsc<Channel>(getMIDIChannel(event->groupName, event->channelID));
		ch->sendPostEvent(event, startAt);
	}
	bool Mixer::hasMIDIChannel(const String & groupName, s_midichannel_id channelID){
		auto res=midiChannelMap.find({groupName, channelID});
		if(res == midiChannelMap.end())return false;
		return true;
	}
	Note * NotePool::newInstance(){
		return new Note(uniqueID++);
	}
	void NotePool::reset(){
		uniqueID=0;
	}
	void NotePool::onReuse(Note * note){}
	void NotePool::onClear(){ uniqueID=0; }
	void Channel::setChannelId(s_midichannel_id id){
		this->channelID=id;
		if(IMixer::isDrumSetChannel(getConfig(), id)){
			channelConfig.setNoteProcessor(channelConfig.instrument->getDrumSet(channelConfig.Bank, getSampleRate()));
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
		for(u_index i=0;i < channelCount;i++){
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
		for(u_index i=0;i < channelCount;i++){
			SampleArray & outputBuf=output[i];
			memset(outputBuf._array, 0, sizeof(u_sample) * outputBuf.length);
		}
		for(u_index ch=0;ch < channelCount;ch++){
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
	void Channel::setBufferSize(u_index bs){
		for(u_index i=0;i < channelCount;i++){
			output[i]=SampleArray(bs);
		}
	}
	u_index Channel::getBufferSize()const{
		return output[0].length;
	}
	Channel::~Channel(){
	}
	Channel::Channel(){
		channelConfig.channel=this;
		//
		dspChain[0]=mksp<DSPChain>();
		dspChain[1]=mksp<DSPChain>();
		//
		panner[0]=mksp<AmpMultiply>();
		panner[1]=mksp<AmpMultiply>();
		//
		Random rand;
		choruser[0]=mksp<Chorus>(mksp<SineOscillator>(0.027, rand.nextDouble()), 30, 0.3, 0);
		choruser[1]=mksp<Chorus>(mksp<SineOscillator>(0.021, rand.nextDouble()), 30, 0.3, 0);
		//
		phaser[0]=mksp<Phaser>(0.5, 2.0, 0.0, 0.3, 4);
		phaser[1]=mksp<Phaser>(0.5, 2.0, 0.0, 0.3, 4);
		//
		reverber[0]=mksp<Freeverb>(0.0);
		reverber[1]=mksp<Freeverb>(0.0);
		//
		limiter[0]=mksp<Limiter>(5, 300, 500, 0.707, EnvelopDetector::RMS, 200);
		limiter[1]=mksp<Limiter>(5, 300, 500, 0.707, EnvelopDetector::RMS, 200);
		//
		addDSPToChain(panner);
		addDSPToChain(choruser);
		addDSPToChain(phaser);
		addDSPToChain(reverber);
		lastActiveTime=System::currentTimeMillis();
	}
	void Channel::fillBuffer(){
		int64_t t=System::nanoTime();
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
		s_sample_index mixerCurrentSampleIndex=channelConfig.mixer->getCurrentSampleIndex();
		int blen=getBufferSize();
		int chCount=channelCount;
		int channelCurrentSampleIndex=0;
		u_sample * s_outputL=output[0]._array;
		u_sample * s_outputR=output[1]._array;
		for(u_index sample=0;sample < blen;sample++){
			channelConfig.currentTime=(u_time)(mixerCurrentSampleIndex + channelCurrentSampleIndex) * invSampleRate;
			s_eventTimeSum+=(u_time_f)s_currentDeltaTime;
			if(s_eventTimeSum > setEventDeltaTime){
				std::unique_lock <std::recursive_mutex > lock(eventLock);
				{
					auto eventIterator=postEventQueue.iterator();
					while(eventIterator->hasNext()){
						ChannelEvent * event=eventIterator->next();
						if(channelConfig.currentTime < event->startAtTime) break;
						eventIterator->remove();
						procEvent(*event);
						delete event;
					}
				}
				{
					auto eventIterator=instantEventQueue.iterator();
					while(eventIterator->hasNext()){
						ChannelEvent * event=eventIterator->next();
						if(channelConfig.currentTime < event->startAtTime) break;
						eventIterator->remove();
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
		for(u_index sample=0;sample < blen;sample++){
			currentTime=(u_time)(mixerCurrentSampleIndex + channelCurrentSampleIndex) * invSampleRate;*/
			u_sample outputf=0;
			if(channelConfig.noteProcessor != nullptr){
				NoteProcessor & np=*channelConfig.noteProcessor;
				for(u_index i=0;i < workingNotesPool.size();i++){
					auto & po=workingNotesPool.get(i);
					Note & note=*po.object;
					if(note.noMoreData)continue;
					NoteUpdater::preUpdateNote(note, channelConfig);
					u_sample noteout=np.getAmp(note);
					NoteUpdater::postUpdateNote(note,  channelConfig);
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
		for(u_index i=0;i < workingNotesPool.size();i++){
			auto & po=workingNotesPool.get(i);
			Note & note=*po.object;
			if(note.noMoreData){
				workingNotesPool.returnObject(po);
				i--;
			}
		}
		memcpy(s_outputR, s_outputL, sizeof(u_sample) * blen);
		if(auto p=channelConfig.dsp3d){
			p->setChannel(0);
			p->procBlock(s_outputL, blen);
			p->setChannel(1);
			p->procBlock(s_outputR, blen);
		} else{
			spsc<AmpMultiply>(panner[0])->setValue(Util::clamp01(1 - channelConfig.Pan));
			spsc<AmpMultiply>(panner[1])->setValue(Util::clamp01(1 + channelConfig.Pan));
			spsc<DSP>(dspChain[0])->procBlock(s_outputL, blen);
			spsc<DSP>(dspChain[1])->procBlock(s_outputR, blen);
		}
		if(channelConfig.mixer->isUseLimiter()){
			spsc<DSP>(limiter[0])->procBlock(s_outputL, blen);
			spsc<DSP>(limiter[1])->procBlock(s_outputR, blen);
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
	u_index Channel::getPostedEventCount()const{
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
			for(u_index i=0;i < workingNotesPool.size();i++){
				auto & po=workingNotesPool.get(i);
				Note & n=*po.object;
				n.forceClose(channelConfig);
			}
		}
		for(u_index i=0;i < workingNotesPool.size();i++){
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
			for(u_index i=0;i < workingNotesPool.size();i++){
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
		if(channelConfig.Sustain || channelConfig.sostenutoLock[note.id]){
			return;
		}
		if(channelConfig.noteProcessor != nullptr)channelConfig.noteProcessor->noteOff(channelConfig, note.id, note.velocity);
		for(u_index i=0;i < workingNotesPool.size();i++){
			auto & po=workingNotesPool.get(i);
			Note & n=*po.object;
			if(n.id == note.id){
				NoteUpdater::noteOff(n,  channelConfig, note.velocity);
			}
		}
	}
	void Channel::procNotePressure(NotePressure & note){
		if(Note::idInvalid(note.id)) return;
		for(u_index i=0;i < workingNotesPool.size();i++){
			auto & po=workingNotesPool.get(i);
			Note & n=*po.object;
			if(note.id == n.id){
				n.velocity=note.velocity;
			}
		}
	}
	void Channel::procNotePitchBend(NotePitchBend & note){
		if(Note::idInvalid(note.id)) return;
		for(u_index i=0;i < workingNotesPool.size();i++){
			auto & po=workingNotesPool.get(i);
			Note & n=*po.object;
			n.pitchBend=note.value;
		}
	}
	void Channel::procChannelPressure(s_note_vel value){
		for(u_index i=0;i < workingNotesPool.size();i++){
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
				for(u_index i=0;i < workingNotesPool.size();i++){
					auto & po=workingNotesPool.get(i);
					Note & n=*po.object;
					n.requestClose(channelConfig);
					offMap[n.id]=true;
				}
				if(channelConfig.noteProcessor != nullptr){
					for(u_index i=0;i < CHANNEL_MAX_NOTE_ID;i++){
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
					u_sp<AHDSREnvelop> a=getAHDSREnv();
					if(a){
						a->attackTime=pow(10000.0, cc.value / 127.0f) / 1000.0;
					}
				}
				break;
			case MIDIFile::CC::DECAY_TIME:
				if(ENABLE_MIDI_CC_ADSR){
					u_sp<AHDSREnvelop> a=getAHDSREnv();
					if(a){
						a->decayTime=pow(10000.0, cc.value / 127.0f) / 1000.0;
					}
				}
				break;
			case MIDIFile::CC::RELEASE_TIME:
				if(ENABLE_MIDI_CC_ADSR){
					u_sp<AHDSREnvelop> a=getAHDSREnv();
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

		for(u_index i=0; i < workingNotesPool.size(); i++){
			auto & po=workingNotesPool.get(i);
			Note & n=*po.object;
			if(channelConfig.sostenutoLock[n.id]) continue;
			if(!channelConfig.noteHoldMap[n.id]){
				offMap[n.id]=true;
				n.requestClose(channelConfig);
			}
		}
		if(channelConfig.noteProcessor != nullptr){
			for(u_index i=0;i < CHANNEL_MAX_NOTE_ID;i++){
				if(offMap[i])channelConfig.noteProcessor->noteOff(channelConfig, i, 0);
			}
		}
	}
	u_sp<AHDSREnvelop> Channel::getAHDSREnv()const{
		auto a=dynamic_cast<EnvelopMultiplier *>(channelConfig.noteProcessor);
		if(!a)return nullptr;
		auto b=spdc<AHDSREnvelop>(a->a);
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
		auto instr=channelConfig.instrument;
		if(event.noteProcessor != nullptr){
			src=event.noteProcessor;
		} else if(instr == nullptr){
			std::cout << "Midi Instrument not set" << std::endl;
			src=SynthUtil::getDefault();
		} else if(IMixer::isDrumSetChannel(getConfig(), channelID)){
			src=instr->getDrumSet(channelConfig.Bank, getSampleRate());			
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
	u_index Channel::getCurrentProcessingNoteCount()const{
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
	void Channel::addDSPToChain(u_sp<yzrilyzr_dsp::DSP> * dsp){
		for(u_index i=0;i < 2;i++){
			dspChain[i]->add(dsp[i]);
		}
	}
	u_sample_rate Channel::getSampleRate()const{
		return channelConfig.sampleRate;
	}
	void Channel::setPan(u_normal_11_f pan){
		this->channelConfig.Pan=pan;
	}
	void Channel::setNoteShift(int8_t noteShift){
		this->channelConfig.NoteShift=noteShift;
	}
	Chorus & Channel::getChorus(u_index ch)const{
		return *(spsc<Chorus>(choruser[ch]));
	}
	Phaser & Channel::getPhaser(u_index ch)const{
		return *(spsc<Phaser>(phaser[ch]));
	}
	Freeverb & Channel::getReverb(u_index ch)const{
		return *(spsc<Freeverb>(reverber[ch]));
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
				channelConfig.mixer->setUseLimiter(value >= 64);
				break;
			case NRPN::BUILDER_START:
				//if(value == 127)setNoteProcessor(mksp<AmpBuilderProcessor>());
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
	s_midichannel_id Channel::getChannelID() const{
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
		return output[chIndex]._array;
	}
	ChannelConfig & Channel::getConfig(){
		return channelConfig;
	}
}