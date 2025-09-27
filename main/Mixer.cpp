#include "Channel.h"
#include "Mixer.h"
#include "interface/InstrumentProvider.h"
#include "tuning/EqualTemperament.h"
#include "dsp/DSPChain.h"
#include "dsp/Limiter.h"
#include <future>
#include "lang/System.h"
#include "lang/Thread.h"
#include "lang/Runtime.h"
#include <vector>
using namespace yzrilyzr_array;

using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_collection;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	Mixer::Mixer(size_t bufferSize){
		setBufferSize(bufferSize);
		setSynthMode(MODE_FUTURE, -1);
		setNoteTuning(std::make_shared<EqualTemperament>());
		nonDrumSetLimiter=new Limiter * [2]{new Limiter(5,5000, 5000, 1), new Limiter(5,5000, 5000, 1)};
		masterLimiter=new Limiter * [2]{new Limiter(5,5000, 5000, 1), new Limiter(5,5000, 5000, 1)};
		finalEQ[0]=std::make_shared<DSPChain>();
		finalEQ[1]=std::make_shared<DSPChain>();
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
	void Mixer::setBufferSize(size_t bs){
		for(int32_t i=0;i < getOutputChannelCount();i++){
			output[i]=std::make_shared<SampleArray>(bs);
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
	void fillBuffer1(std::shared_ptr<Channel> c){
		Thread::currentThread()->setPriority(Thread::Priority::HIGH);
		c->fillBuffer();
	}
	void Mixer::commitChannels(){
		if(_pause) return;
		switch(synthMode){
			case MODE_SINGLE_THREAD:
			{
				std::unique_lock<std::recursive_mutex > lock(channelLock);
				for(std::shared_ptr<Channel> mix : channels){
					mix->fillBuffer();
				}
				break;
			}
			case MODE_THREAD_POOL:
			{
				std::unique_lock<std::recursive_mutex > lock(channelLock);
				for(std::shared_ptr<Channel> mix : channels){
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
				for(std::shared_ptr<Channel> mix : channels){
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
		size_t blen=getBufferSize();
		size_t chCount=getOutputChannelCount();
		u_sample_rate sampleRate=getSampleRate();
		bool tUseEQ=useEQ;
		bool tUseLimiter=useLimiter;
		Limiter ** tLimiter=this->nonDrumSetLimiter;
		Limiter ** masterLimiter=this->masterLimiter;
		std::unique_lock<std::recursive_mutex> lock(channelLock);
		ArrayList<std::shared_ptr<Channel>>::ListItr channelItr(&channels);
		u_time_stamp now=System::currentTimeMillis();
		while(channelItr.hasNext()){
			std::shared_ptr<Channel> mix=channelItr.next();
			if(mix->hasData()){
				mix->lastActiveTime=now;
			}
			if(!mix->alwaysActive && now - mix->lastActiveTime > idleChannelLiveTime){
				std::cout << "Removed Idle Channel:" << mix->getChannelId() << std::endl;
				channelItr.remove();
			}
		}
		for(int ch=0;ch < chCount;ch++){
			u_sample * thisOutput=output[ch]->_array;
			memset(thisOutput, 0, blen * sizeof(u_sample));
			for(auto & c : channels){
				u_sample * channelOutput=c->output[ch]->_array;
				bool isDrumSet=c->isDrumSetChannel();
				if(isDrumSet)continue;
				for(int sample=0;sample < blen;sample++){
					thisOutput[sample]+=channelOutput[sample];
				}
			}
			if(tUseLimiter) tLimiter[ch]->procBlock(thisOutput, blen);
			for(auto & c : channels){
				u_sample * channelOutput=c->output[ch]->_array;
				bool isDrumSet=c->isDrumSetChannel();
				if(!isDrumSet)continue;
				for(int sample=0;sample < blen;sample++){
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
			for(int ch=0;ch < chCount;ch++){
				nonDrumSetLimiter[ch]->resetMemory();
				masterLimiter[ch]->resetMemory();
			}
		}
		if(flags.hasAndRemove(FLAG_RESET_EQ)){
			for(int ch=0;ch < chCount;ch++){
				finalEQ[ch]->resetMemory();
			}
		}
		if(flags.hasAndRemove(FLAG_RESET_BUFFER)){
			for(int i=0;i < chCount;i++){
				SampleArray & outputBuf=*output[i];
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

	std::vector<std::shared_ptr<IChannel>> Mixer::getAllChannels()const{
		std::vector<std::shared_ptr<IChannel>> chann;
		for(auto & i : channels){
			chann.emplace_back(std::dynamic_pointer_cast<IChannel>(i));
		}
		return chann;
	}

	void Mixer::setSampleRate(u_sample_rate sr){
		IMixer::setSampleRate(sr);
		for(int ch=0;ch < getOutputChannelCount();ch++){
			if(useLimiter){
				nonDrumSetLimiter[ch]->init(sr);
				masterLimiter[ch]->init(sr);
			}
			if(useEQ)finalEQ[ch]->init(sr);
		}
	}
	void Mixer::setEQ(int32_t seg, double value){}
	std::shared_ptr<IChannel> Mixer::getMIDIChannel(const std::string & name, s_midichannel_id channelID){
		auto res=midiChannelMap.find({name, channelID});
		if(res == midiChannelMap.end()){
			auto channel=std::make_shared<Channel>();
			channel->setName(name + " #" + std::to_string(channelID));
			if(channelID == 9) channel->setSustain(true);
			setMIDIChannel(name, channelID, channel);
			return std::dynamic_pointer_cast<IChannel>(channel);
		}
		return std::dynamic_pointer_cast<IChannel>(res->second);
	}
	std::shared_ptr<yzrilyzr_dsp::DSPChain> * Mixer::getEQ(){
		return finalEQ;
	}
	void Mixer::addChannel(std::shared_ptr<Channel> channel){
		std::unique_lock<std::recursive_mutex > lock(channelLock);
		channels.add(channel);
	}
	void Mixer::setMIDIChannel(s_midichannel_id id, std::shared_ptr<Channel>channel){
		setMIDIChannel(DEFAULT_MIDI_CHANNEL_GROUP_NAME, id, channel);
	}
	size_t Mixer::getBufferSize()const{
		return output[0]->length;
	}
	void Mixer::setMIDIChannel(const std::string & name, s_midichannel_id id, std::shared_ptr<Channel>channel){
		channel->setBufferSize(getBufferSize());
		channel->setMixer(this);
		channel->setSampleRate(getSampleRate());
		channel->setChannelId(id);
		addChannel(channel);
		std::unique_lock<std::recursive_mutex > lock(midiChannelMapLock);
		midiChannelMap[{name, id}]=channel;
	}
	void Mixer::removeMIDIChannel(s_midichannel_id channel){
		removeMIDIChannel(DEFAULT_MIDI_CHANNEL_GROUP_NAME, channel);
	}
	void Mixer::removeMIDIChannel(const std::string & name, s_midichannel_id channelID){
		std::unique_lock<std::recursive_mutex > lock(midiChannelMapLock);
		auto res=midiChannelMap.find({name, channelID});
		if(res != midiChannelMap.end()){
			std::shared_ptr<Channel> paramRegPtr=res->second;
			midiChannelMap.erase({name, channelID});
			removeChannel(paramRegPtr);
		}
	}
	void Mixer::removeChannel(std::shared_ptr<Channel> channel){
		std::unique_lock<std::recursive_mutex > lock(channelLock);
		channels.removeElement(channel);
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
	size_t Mixer::getCurrentProcessingNoteCount(){
		size_t sum=0;
		std::unique_lock<std::recursive_mutex > lock(channelLock);
		for(auto & c : channels) sum+=c->getCurrentProcessingNoteCount();
		return sum;
	}
	size_t Mixer::getPostedEventCount(){
		size_t sum=0;
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
		return output[chIndex]->_array;
	}
	void Mixer::sendInstantEvent(ChannelEvent * event){
		std::shared_ptr<Channel> ch=std::dynamic_pointer_cast<Channel>(getMIDIChannel(event->groupName, event->channelID));
		ch->sendInstantEvent(event);
	}
	void Mixer::postEvent(ChannelEvent * event, u_time startAt){
		std::shared_ptr<Channel> ch=std::dynamic_pointer_cast<Channel>(getMIDIChannel(event->groupName, event->channelID));
		ch->sendPostEvent(event, startAt);
	}
	bool Mixer::hasMIDIChannel(const std::string & groupName, s_midichannel_id channelID){
		auto res=midiChannelMap.find({groupName, channelID});
		if(res == midiChannelMap.end())return false;
		return true;
	}
}