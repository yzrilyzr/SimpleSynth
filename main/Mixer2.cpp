#include "Mixer2.h"
#include "NRPN.h"
#include "SimpleSynth.h"
#include "SynthUtil.h"
#include "array/Array.hpp"
#include "dsp/Chorus.h"
#include "dsp/DSP.h"
#include "dsp/DSP3D.h"
#include "dsp/DSPChain.h"
#include "dsp/Freeverb.h"
#include "dsp/Limiter.h"
#include "dsp/Phaser.h"
#include "events/ChannelConfig.h"
#include "events/ChannelEvent.h"
#include "events/Note.h"
#include "events/NoteUpdater.h"
#include "events/PNData.h"
#include "interface/InstrumentProvider.h"
#include "interface/NoteProcessor.h"
#include "lang/Runtime.h"
#include "lang/System.h"
#include "util/FixedThreadPool.h"
#include "util/MIDIFile.h"
#include "util/Pool2.hpp"
#include "util/Random.h"
#include "util/Util.h"
#include "yzrutil.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <utility>
#include <vector>
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_collection;
using namespace yzrilyzr_array;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_interpolator;
namespace yzrilyzr_simplesynth{
	NoteTask * NoteTaskPool::newInstance(){
		return new NoteTask(uniqueID++);
	}
	void NoteTaskPool::reset(){}
	void NoteTaskPool::onReuse(NoteTask * note){}
	NoteTask::NoteTask(uint8_t uniqueID) :note(uniqueID){}
	ChannelData::ChannelData(const String & groupName, s_midichannel_id channelID, uint32_t bufSize){
		this->groupName=groupName;
		this->channelID=channelID;
		cfgSnapshots.resize(bufSize);
		for(u_index i=0; i < bufSize; ++i){
			cfgSnapshots[i]=std::make_shared<ChannelConfig>();
			cfgSnapshots[i]->channel=this;
		}

		output[0]=std::make_shared<SampleArray>(bufSize);
		output[1]=std::make_shared<SampleArray>(bufSize);
		noteOutput=std::make_shared<SampleArray>(bufSize);

		dspChain[0]=std::make_shared<DSPChain>();
		dspChain[1]=std::make_shared<DSPChain>();
		//
		Random rand;
		choruser[0]=std::make_shared<Chorus>(std::make_shared<SineOscillator>(0.027, rand.nextDouble()), 30, 0.3, 0);
		choruser[1]=std::make_shared<Chorus>(std::make_shared<SineOscillator>(0.021, rand.nextDouble()), 30, 0.3, 0);
		//
		phaser[0]=std::make_shared<Phaser>(0.5, 2.0, 0.0, 0.3, 4);
		phaser[1]=std::make_shared<Phaser>(0.5, 2.0, 0.0, 0.3, 4);
		//
		reverber[0]=std::make_shared<Freeverb>(0.0);
		reverber[1]=std::make_shared<Freeverb>(0.0);
		//
		limiter[0]=std::make_shared<Limiter>(5, 300, 500, 0.707, EnvelopDetector::RMS, 300);
		limiter[1]=std::make_shared<Limiter>(5, 300, 500, 0.707, EnvelopDetector::RMS, 300);
		//
		for(u_index i=0;i < 2;i++){
			dspChain[i]->add(choruser[i]);
			dspChain[i]->add(phaser[i]);
			dspChain[i]->add(reverber[i]);
		}
		setName(groupName + " #" + std::to_string(channelID));
	}
	void ChannelData::setSampleRate(u_sample_rate sr){
		for(u_index i=0;i < 2;i++){
			dspChain[i]->init(sr);
			limiter[i]->init(sr);
		}
		if(auto p=getConfig().dsp3d){
			p->init(sr);
		}
	}
	Chorus & ChannelData::getChorus(u_index ch)const{
		return *std::dynamic_pointer_cast<Chorus>(choruser[ch]);
	}
	Freeverb & ChannelData::getReverb(u_index ch)const{
		return *std::dynamic_pointer_cast<Freeverb>(reverber[ch]);
	}
	Phaser & ChannelData::getPhaser(u_index ch)const{
		return *std::dynamic_pointer_cast<Phaser>(phaser[ch]);
	}
	void ChannelData::reset(){}
	u_sample * ChannelData::getOutput(uint32_t chIndex)const{
		return output[chIndex]->_array;
	}
	ChannelConfig & ChannelData::getConfig(){
		return *cfgSnapshots[0];
	}
	Mixer2::Mixer2(u_index bufferSize){
		setBufferSize(bufferSize);
		setSynthMode(MODE_SINGLE_THREAD, -1);
		auto defCfg=ChannelConfig::DefaultConfig();
		getGlobalConfig().set(*defCfg);
		getGlobalConfig().mixer=this;
		nonDrumSetLimiter[0]=std::make_shared<Limiter>(5, 5000, 5000, 1.0);
		nonDrumSetLimiter[1]=std::make_shared<Limiter>(5, 5000, 5000, 1.0);
		drumSetLimiter[0]=std::make_shared<Limiter>(5, 500, 500, 3.0);
		drumSetLimiter[1]=std::make_shared<Limiter>(5, 500, 500, 3.0);
		masterLimiter[0]=std::make_shared<Limiter>(1, 500, 5000, 1.0);
		masterLimiter[1]=std::make_shared<Limiter>(1, 500, 5000, 1.0);
		finalEQ[0]=std::make_shared<DSPChain>();
		finalEQ[1]=std::make_shared<DSPChain>();
	}
	Mixer2::~Mixer2(){
		delete threadPool;
	}
	void Mixer2::mix(){
		u_time t=(u_time)System::nanoTime();

		// 0. 全局锁保护整个mix过程
		std::unique_lock<std::shared_mutex> mLock(mixLock);

		// 1. 准备基本参数
		u_sample_rate sampleRate=getSampleRate();
		u_time deltaTime=1.0 / sampleRate;
		u_index bufSize=getBufferSize();
		u_index chc=getOutputChannelCount();

		// 2. 处理通道和事件
		processChannelSnapshots();
		processInstantEvents(deltaTime, bufSize);
		processScheduledEvents(deltaTime, bufSize);

		// 3. 音符合成阶段
		synthesizeNotes();
		waitForAllTasks();

		// 4. 音符混合阶段
		std::unordered_map<ChannelData *, std::vector<NoteTask *>> noteMixTasks;
		prepareNoteMixTasks(noteMixTasks);
		submitNoteMixTasks(noteMixTasks);
		waitForAllTasks();

		// 5. 清理和混音阶段
		cleanupFinishedNotes();

		// 清空主输出缓冲区
		for(u_index ch=0; ch < chc; ch++){
			memset(output[ch]->_array, 0, bufSize * sizeof(u_sample));
		}

		// 6. 混合和处理非鼓组通道
		mixNonDrumChannelsToOutput(chc, bufSize);
		processNonDrumLimiters(chc, bufSize);

		// 7. 混合和处理鼓组通道
		mixDrumChannelsToOutput(chc, bufSize);
		processDrumLimiters(chc, bufSize);
		mixDrumToMainOutput(chc, bufSize);

		// 8. 主效果器处理
		processMasterEffects(chc, bufSize);

		// 9. 收尾工作
		finalizeMix(bufSize);

		processTime=(u_time_f)((u_time_f)(System::nanoTime() - t) / 1000000000.0);
	}
	void Mixer2::waitForAllTasks(){
		if(synthMode == MODE_THREAD_POOL){
			threadPool->waitAll();
		} else if(synthMode == MODE_FUTURE){
			for(auto & fu : futures){
				fu.get();
			}
			futures.clear();
		}
	}
	void Mixer2::processChannelSnapshots(){
		std::unique_lock<std::shared_mutex> lock(channelLock);
		for(auto & data : allChannelData){
			ChannelData & dat=*data;
			setDataSnapshotBaseInfo(dat);
			if(dat.lastSnapshotChange){
				transferSnapshot(dat, -1);
				dat.lastSnapshotChange=false;
			}
		}
	}

	void Mixer2::processInstantEvents(u_time deltaTime, u_index bufSize){
		std::unique_lock<std::shared_mutex> lock(eventLock);
		for(auto it=instantEventQueue.begin(); it != instantEventQueue.end();){
			ChannelEvent * event=*it;
			int64_t snapshotIndex=0;
			auto & data=*getOrCreateMIDIChannelData(event->groupName, event->channelID);
			ChannelConfig & cfg=*data.cfgSnapshots[snapshotIndex];
			procEvent(data, cfg, *event);
			transferSnapshot(data, snapshotIndex);
			data.lastSnapshotChange=true;
			it=instantEventQueue.erase(it);
			delete event;
		}
	}

	void Mixer2::processScheduledEvents(u_time deltaTime, u_index bufSize){
		std::unique_lock<std::shared_mutex> lock(eventLock);
		for(auto it=postEventQueue.begin(); it != postEventQueue.end();){
			ChannelEvent * event=*it;
			u_time ti=event->startAtTime - getCurrentTime();
			int64_t snapshotIndex=static_cast<int64_t>(ti / deltaTime);
			if(snapshotIndex < 0) snapshotIndex=0;
			if(snapshotIndex >= static_cast<int64_t>(bufSize)) break;

			auto & data=*getOrCreateMIDIChannelData(event->groupName, event->channelID);
			ChannelConfig & cfg=*data.cfgSnapshots[snapshotIndex];
			procEvent(data, cfg, *event);
			transferSnapshot(data, snapshotIndex);
			data.lastSnapshotChange=true;
			it=postEventQueue.erase(it);
			delete event;
		}
	}

	void Mixer2::synthesizeNotes(){
		std::unique_lock<std::shared_mutex> lock(channelLock);
	#ifdef _DEBUG
		std::map<std::pair<s_midichannel_id, uint8_t>, std::pair<ChannelData *, Note *>> uniqueMap;
	#endif

		for(auto & data : allChannelData){
			auto & pool=data->workingNotesPool;
			for(u_index i=0, j=pool.size(); i < j; i++){
				auto & po=pool.get(i);
				NoteTask & task=*po.object;

			#ifdef _DEBUG
				if(uniqueMap.find({data->getChannelID(), task.note.uniqueID}) != uniqueMap.end()){
					auto pa=uniqueMap[{data->getChannelID(), task.note.uniqueID}];
					if(pa.first->getGroupName() == data->getGroupName()){
						throw IllegalStateException("Duplicated note unique id");
					}
				}
				uniqueMap[{data->getChannelID(), task.note.uniqueID}]={data.get(), &task.note};
			#endif

				if(synthMode == MODE_THREAD_POOL){
					threadPool->commit([this, &task](){
						procNoteTask(task);
					});
				} else if(synthMode == MODE_FUTURE){
					futures.push_back(std::async(std::launch::async, [this, &task](){
						procNoteTask(task);
					}));
				} else{
					procNoteTask(task);
				}
			}
		}
	}

	void Mixer2::prepareNoteMixTasks(std::unordered_map<ChannelData *, std::vector<NoteTask *>> & noteMixTasks){
		std::unique_lock<std::shared_mutex> lock(channelLock);

		// 清空所有通道的输出缓冲区
		for(auto & data : allChannelData){
			for(u_index ch=0; ch < getOutputChannelCount(); ch++){
				memset(data->output[ch]->_array, 0, getBufferSize() * sizeof(u_sample));
			}
		}

		// 准备混合任务
		for(auto & data : allChannelData){
			auto & pool=data->workingNotesPool;
			for(u_index i=0; i < pool.size(); i++){
				auto & po=pool.get(i);
				NoteTask & task=*po.object;
				ChannelData * taskData=task.data;

				auto it=noteMixTasks.find(taskData);
				if(it == noteMixTasks.end()){
					it=noteMixTasks.emplace(taskData, std::vector<NoteTask *>()).first;
				}
				it->second.emplace_back(&task);
			}
		}
	}

	void Mixer2::submitNoteMixTasks(std::unordered_map<ChannelData *, std::vector<NoteTask *>> & noteMixTasks){
		std::unique_lock<std::shared_mutex> lock(channelLock);

		for(auto & data : allChannelData){
			ChannelData & d=*data;
			std::vector<NoteTask *> * tasks=nullptr;

			auto it=noteMixTasks.find(data.get());
			if(it != noteMixTasks.end()){
				tasks=&it->second;
			}

			if(synthMode == MODE_THREAD_POOL){
				threadPool->commit([this, &data=*data, tasks](){
					procNoteMix(data, tasks);
				});
			} else if(synthMode == MODE_FUTURE){
				futures.push_back(std::async(std::launch::async, [this, &data=*data, tasks](){
					procNoteMix(data, tasks);
				}));
			} else{
				procNoteMix(d, tasks);
			}
		}
	}

	void Mixer2::cleanupFinishedNotes(){
		std::unique_lock<std::shared_mutex> lock(channelLock);

		for(auto & data : allChannelData){
			auto & pool=data->workingNotesPool;
			for(u_index i=0; i < pool.size(); i++){
				auto & po=pool.get(i);
				NoteTask & task=*po.object;

				if(task.note.noMoreData || task.note.cfg->noteProcessor == nullptr){
					pool.returnObject(po);
					i--;
				}
			}
			//data->programCache.clear();
		}
	}

	void Mixer2::mixNonDrumChannelsToOutput(u_index chc, u_index bufSize){
		std::unique_lock<std::shared_mutex> lock(channelLock);

		for(auto & data : allChannelData){
			if(data->isDrumSetChannel()) continue;

			for(u_index ch=0; ch < chc; ch++){
				u_sample * mixerOut=output[ch]->_array;
				u_sample * channelOut=data->output[ch]->_array;

				for(u_index i=0; i < bufSize; i++){
					mixerOut[i]+=channelOut[i];
				}
			}
		}
	}

	void Mixer2::processNonDrumLimiters(u_index chc, u_index bufSize){
		if(!useLimiter) return;

		std::unique_lock<std::shared_mutex> lock(dspLock);
		for(u_index ch=0; ch < chc; ch++){
			nonDrumSetLimiter[ch]->procBlock(output[ch]->_array, bufSize);
		}
	}

	void Mixer2::mixDrumChannelsToOutput(u_index chc, u_index bufSize){
		// 清空鼓组输出缓冲区
		for(u_index ch=0; ch < chc; ch++){
			memset(drumOutput[ch]->_array, 0, bufSize * sizeof(u_sample));
		}

		std::unique_lock<std::shared_mutex> lock(channelLock);

		for(auto & data : allChannelData){
			if(!data->isDrumSetChannel()) continue;

			for(u_index ch=0; ch < chc; ch++){
				u_sample * chOut=data->output[ch]->_array;
				u_sample * drumOut=drumOutput[ch]->_array;

				for(u_index i=0; i < bufSize; i++){
					drumOut[i]+=chOut[i];
				}
			}
		}
	}

	void Mixer2::processDrumLimiters(u_index chc, u_index bufSize){
		if(!useLimiter) return;

		std::unique_lock<std::shared_mutex> lock(dspLock);
		for(u_index ch=0; ch < chc; ch++){
			drumSetLimiter[ch]->procBlock(drumOutput[ch]->_array, bufSize);
		}
	}

	void Mixer2::mixDrumToMainOutput(u_index chc, u_index bufSize){
		for(u_index ch=0; ch < chc; ch++){
			u_sample * mixerOut=output[ch]->_array;
			u_sample * drumOut=drumOutput[ch]->_array;

			for(u_index i=0; i < bufSize; i++){
				mixerOut[i]+=drumOut[i];
			}
		}
	}

	void Mixer2::processMasterEffects(u_index chc, u_index bufSize){
		std::unique_lock<std::shared_mutex> lock(dspLock);

		if(useEQ){
			for(u_index ch=0; ch < chc; ch++){
				std::dynamic_pointer_cast<DSP>(finalEQ[ch])->procBlock(output[ch]->_array, bufSize);
			}
		}

		if(useLimiter){
			for(u_index ch=0; ch < chc; ch++){
				masterLimiter[ch]->procBlock(output[ch]->_array, bufSize);
			}
		}
	}

	void Mixer2::finalizeMix(u_index bufSize){
		mixerCurrentSampleIndex+=bufSize;

		if(flags.hasAndRemove(FLAG_RESET)){
			mReset();
		}
	}
	void Mixer2::transferSnapshot(ChannelData & data, int32_t startInc){
		auto & cfgSnapshots=data.cfgSnapshots;
		u_index bufSize=cfgSnapshots.size();
		std::shared_ptr<ChannelConfig> last=cfgSnapshots[startInc == -1?bufSize - 1:startInc];
		for(u_index i=startInc + 1;i < bufSize;i++){
			ChannelConfig & curt=*cfgSnapshots[i];
			curt.setOnlyChannelConfig(*last);
		}
	}
	void  Mixer2::reset(){
		std::unique_lock <std::shared_mutex > lock0(mixLock);
		std::unique_lock <std::shared_mutex > lock1(channelLock);
		std::unique_lock <std::shared_mutex > lock2(eventLock);
		std::unique_lock <std::shared_mutex > lock3(dspLock);
		mReset();		
	}
	void  Mixer2::mReset(){
		channelData.clear();
		allChannelData.clear();
		instantEventQueue.clear();
		postEventQueue.clear();
		mResetLimiter();
	}

	u_time Mixer2::getCurrentTime() const{
		return mixerCurrentSampleIndex / (u_time)getSampleRate();
	}
	u_index Mixer2::getCurrentProcessingNoteCount(){
		std::unique_lock <std::shared_mutex > lock(channelLock);
		u_index sum=0;
		for(auto & data : allChannelData){
			sum+=data->workingNotesPool.size();
		}
		return sum;
	}
	u_index Mixer2::getPostedEventCount(){
		return instantEventQueue.size() + postEventQueue.size();
	}
	bool Mixer2::hasData(){
		return !postEventQueue.empty() || !instantEventQueue.empty() || getCurrentProcessingNoteCount() != 0;
	}

	void Mixer2::setDataSnapshotBaseInfo(ChannelData & data){
		u_sample_rate sampleRate=getSampleRate();
		u_time deltaTime=1.0 / sampleRate;
		u_index bufSize=getBufferSize();
		auto & cfgSnapshots=data.cfgSnapshots;
		//填充基础信息
		for(u_index index=0;index < bufSize;index++){
			ChannelConfig & current=*cfgSnapshots[index];
			current.currentTime=(mixerCurrentSampleIndex + index) * deltaTime;
			current.deltaTime=deltaTime;
			current.sampleRate=sampleRate;
		}
	}
	/**
	* 一对一任务
	*/
	void Mixer2::procNoteTask(NoteTask & task){
		u_index bufSize=getBufferSize();
		Note & note=task.note;
		auto & data=*task.data;
		const auto & cfgSnapshots=data.cfgSnapshots;
		if(task.output == nullptr || task.output->length != getBufferSize()){
			task.output=std::make_shared<SampleArray>(getBufferSize());
		}
		u_sample * output=task.output->_array;
		//填充Note数据
		memset(output, 0, bufSize * sizeof(u_sample));
		for(u_index index=0;index < bufSize;index++){
			ChannelConfig & cfg=*cfgSnapshots[index];
			if(note.noMoreData)break;
			note.cfg=&cfg;
			NoteUpdater::preUpdateNote(note, cfg);
			if(note.passedTime < 0)continue;
			if(note.uniqueID >= CHANNEL_MAX_VOICE){
				std::cout << "Invalid Note Unique ID" << std::endl;
				flags.add(FLAG_RESET);
				return;
			}
			if(cfg.noteProcessor == nullptr){
				continue;
			}
			NoteProcessor & np=*cfg.noteProcessor;
			u_sample noteout=np.getAmp(note);
			NoteUpdater::postUpdateNote(note, cfg);
		#ifdef _DEBUG
			if(isnan(noteout) || std::abs(noteout) > 50){
				std::cout << "Unexpected Note output value" << std::endl;
				//调试？在此打断点重现
				np.getAmp(note);
				noteout=0;
			}
		#endif
			output[index]=noteout;
			if(np.noMoreData(note))note.noMoreData=true;
		}
	}
	void Mixer2::procNoteMix(ChannelData & data, std::vector< NoteTask *> * noteTasks){
		u_index bufSize=getBufferSize();
		auto & cfgSnapshots=data.cfgSnapshots;
		u_sample * allNoteOutput=data.noteOutput->_array;
		memset(allNoteOutput, 0, sizeof(u_sample) * bufSize);
		//混合Note音符
		if(noteTasks != nullptr){
			std::vector< NoteTask *> noteTasksRef=*noteTasks;
			for(auto & task : noteTasksRef){
				u_sample * noteoutput=task->output->_array;
				for(u_index i=0;i < bufSize;i++){
					allNoteOutput[i]+=noteoutput[i];
				}
			}
		}
		u_index chc=getOutputChannelCount();
		//应用后处理
		for(u_index i=0;i < bufSize;i++){
			ChannelConfig & cfg=*cfgSnapshots[i];
			NoteProcessor * noteProcessor=cfg.noteProcessor;
			if(noteProcessor == nullptr)continue;
			allNoteOutput[i]=noteProcessor->postProcess(allNoteOutput[i]) * cfg.Volume;
		}
		//应用声像
		if(auto dsp3d=data.getConfig().dsp3d){
			for(u_index ch=0;ch < chc;ch++){
				dsp3d->setChannel(ch);
				dsp3d->procBlock(allNoteOutput, data.output[ch]->_array, bufSize);
			}
		} else{
			if(chc == 2){
				u_sample * channelOutputL=data.output[0]->_array;
				u_sample * channelOutputR=data.output[1]->_array;
				for(u_index i=0;i < bufSize;i++){
					ChannelConfig & cfg=*cfgSnapshots[i];
					channelOutputL[i]=allNoteOutput[i] * Util::clamp01(1 - cfg.Pan);
					channelOutputR[i]=allNoteOutput[i] * Util::clamp01(1 + cfg.Pan);
				}
			}
			//应用DSP链
			if(channelUseDSP){
				for(u_index ch=0;ch < chc;ch++){
					std::dynamic_pointer_cast<DSP>(data.dspChain[ch])->procBlock(data.output[ch]->_array, bufSize);
				}
			}
		}
		if(useLimiter){
			for(u_index ch=0;ch < chc;ch++){
				data.limiter[ch]->procBlock(data.output[ch]->_array, bufSize);
			}
		}
	}
	void Mixer2::sendInstantEvent(ChannelEvent * event){
		std::unique_lock <std::shared_mutex > lock(eventLock);
		if(event->groupName.empty())event->groupName=DEFAULT_MIDI_CHANNEL_GROUP_NAME;
		instantEventQueue.push_back(event);
	}
	void Mixer2::postEvent(ChannelEvent * event, u_time startAt){
		std::unique_lock <std::shared_mutex > lock(eventLock);
			if(event->groupName.empty())event->groupName=DEFAULT_MIDI_CHANNEL_GROUP_NAME;
			event->startAtTime=startAt;
			postEventQueue.push_back(event);
	}
	void Mixer2::setSampleRate(u_sample_rate sr){
		IMixer::setSampleRate(sr);
		for(u_index ch=0;ch < getOutputChannelCount();ch++){
			nonDrumSetLimiter[ch]->init(sr);
			drumSetLimiter[ch]->init(sr);
			masterLimiter[ch]->init(sr);
			finalEQ[ch]->init(sr);
		}
	}
	void Mixer2::setBufferSize(u_index bs){
		for(u_index i=0;i < getOutputChannelCount();i++){
			output[i]=std::make_shared<SampleArray>(bs);
			drumOutput[i]=std::make_shared<SampleArray>(bs);
		}
	}
	u_index Mixer2::getBufferSize()const{
		return output[0]->length;
	}
	std::shared_ptr<ChannelData> Mixer2::getOrCreateMIDIChannelData(const String & groupName, s_midichannel_id channelID){
		auto outerIt=channelData.find(groupName);
		if(outerIt == channelData.end()){
			outerIt=channelData.emplace(
				groupName,
				std::unordered_map<s_midichannel_id, std::shared_ptr<ChannelData>>()
			).first;
		}

		auto & innerMap=outerIt->second;
		auto innerIt=innerMap.find(channelID);
		if(innerIt == innerMap.end()){
			u_index bufSize=getBufferSize();
			auto newChannelData=std::make_shared<ChannelData>(groupName, channelID, bufSize);
			ChannelData & ref=*newChannelData;
			ref.getConfig().set(getGlobalConfig());
			ref.setSampleRate(getSampleRate());
			setDataSnapshotBaseInfo(ref);
			transferSnapshot(ref, 0);
			allChannelData.push_back(newChannelData);
			innerIt=innerMap.emplace(channelID, newChannelData).first;
			//std::cout << "NewChannel: " << groupName << ", CH: " << channelID << std::endl;
		}
		return innerIt->second;
	}

	void Mixer2::procEvent(ChannelData & data, ChannelConfig & cfg, ChannelEvent & event){
		//std::cout << event.getType() << "  " << event.groupName << std::endl;
		switch(event.getType()){
			case EventType::NOTE_ON:
			{
				procNoteOn(data, cfg, static_cast<NoteOn &>(event));
				break;
			}
			case EventType::NOTE_OFF:
			{
				procNoteOff(data, cfg, static_cast<NoteOff &>(event));
				break;
			}
			case EventType::NOTE_PITCH_BEND:
			{
				procNotePitchBend(data, cfg, static_cast<NotePitchBend &>(event));
				break;
			}
			case EventType::NOTE_PRESSURE:
			{
				procNotePressure(data, cfg, static_cast<NotePressure &>(event));
				break;
			}
			case EventType::CHANNEL_CONTROL:
			{
				procChannelControl(data, cfg, static_cast<ChannelControl &>(event));
				break;
			}
			case EventType::CHANNEL_PITCH_BEND:
			{
				procChannelPitchBend(data, cfg, static_cast<ChannelPitchBend &>(event));
				break;
			}
			case EventType::CHANNEL_PRESSURE:
			{
				procChannelPressure(data, cfg, static_cast<ChannelPressure &>(event));
				break;
			}
			case EventType::CHANNEL_PROGRAM_CHANGE:
			{
				procInstrument(data, cfg, static_cast<ProgramChange &>(event));
				break;
			}
			case EventType::TUNING_CHANGE:
			{
				procTuningChange(data, cfg, static_cast<TuningChange &>(event));
				break;
			}
		}
	}
	void Mixer2::procNoteOn(ChannelData & data, ChannelConfig & cfg, NoteOn & event){
		if(Note::idInvalid(event.id)) return;
		if(event.velocity == static_cast<s_note_vel>(0)){
			NoteOff off1(event.id);
			procNoteOff(data, cfg, off1);
			return;
		}
		if(cfg.noteProcessor == nullptr){
			NoteProcPtr ins=nullptr;
			u_sample_rate sampleRate=getSampleRate();
			auto instrp=cfg.instrument;
			if(data.isDrumSetChannel()){
				ins=instrp->getDrumSet(cfg.Bank, sampleRate);
				cfg.Sustain=true;
			} else{
				ins=instrp->get(0, 0, sampleRate);
				cfg.Sustain=false;
			}
			if(ins != nullptr){
				cfg.setNoteProcessor(ins);
				ins->init(cfg);
			}
		}
		if(cfg.noteProcessor != nullptr)cfg.noteProcessor->noteOn(cfg, event.id, event.velocity);
		auto & pool=data.workingNotesPool;
		if(cfg.MonoMode){
			for(u_index i=0;i < pool.size();i++){
				auto & po=pool.get(i);
				NoteTask & n=*po.object;
				n.note.forceClose(cfg);
			}
		}
		for(u_index i=0;i < pool.size();i++){
			auto & po=pool.get(i);
			NoteTask & n=*po.object;
			if(n.note.id == event.id){
				n.note.forceClose(cfg);
			}
		}
		NoteTask * pnote=nullptr;
		if(pool.size() >= pool.capacity()){
			//std::cout << "NotePool is full" << std::endl;
			NoteTask * maxNote=nullptr;
			for(u_index i=0;i < pool.size();i++){
				auto & po=pool.get(i);
				NoteTask & n=*po.object;
				if(maxNote == nullptr || n.note.passedTime > maxNote->note.passedTime){
					maxNote=&n;
				}
			}
			pnote=maxNote;
		} else{
			pnote=pool.borrowObject();
		}
		if(pnote == nullptr)return;
		pnote->data=&data;
		Note & note=pnote->note;
		NoteUpdater::noteOn(note, cfg, event.id, event.velocity);
	}
	void Mixer2::procNoteOff(ChannelData & data, ChannelConfig & cfg, NoteOff & event){
		if(Note::idInvalid(event.id)) return;
		cfg.noteHoldMap[event.id]=false;
		if(cfg.Sustain || cfg.sostenutoLock[event.id] || event.channelID == MIDI_DRUM_CHANNEL){
			return;
		}
		if(cfg.noteProcessor != nullptr)cfg.noteProcessor->noteOff(cfg, event.id, event.velocity);
		auto & pool=data.workingNotesPool;
		for(u_index i=0;i < pool.size();i++){
			auto & po=pool.get(i);
			NoteTask & n=*po.object;
			//可能有多个匹配
			if(n.note.id == event.id){
				NoteUpdater::noteOff(n.note, cfg, event.velocity);
			}
		}
	}
	void Mixer2::procNotePressure(ChannelData & data, ChannelConfig & cfg, NotePressure & event){
		auto & pool=data.workingNotesPool;
		for(u_index i=0;i < pool.size();i++){
			auto & po=pool.get(i);
			NoteTask & nt=*po.object;
			Note & n=nt.note;
			if(event.id == n.id){
				n.velocity=event.velocity;
			}
		}
	}
	void Mixer2::procChannelPitchBend(ChannelData & data, ChannelConfig & cfg, ChannelPitchBend & event){
		cfg.ChannelPitchBend=event.value;
	}
	void Mixer2::procChannelPressure(ChannelData & data, ChannelConfig & cfg, ChannelPressure & event){
		auto & pool=data.workingNotesPool;
		for(u_index i=0;i < pool.size();i++){
			auto & po=pool.get(i);
			NoteTask & nt=*po.object;
			Note & n=nt.note;
			n.velocity=event.value;
		}
	}
	void Mixer2::procNotePitchBend(ChannelData & data, ChannelConfig & cfg, NotePitchBend & event){
		auto & pool=data.workingNotesPool;
		for(u_index i=0;i < pool.size();i++){
			auto & po=pool.get(i);
			NoteTask & nt=*po.object;
			Note & n=nt.note;
			if(event.id == n.id){
				n.pitchBend=event.value;
			}
		}
	}

	void Mixer2::procChannelControl(ChannelData & data, ChannelConfig & cfg, ChannelControl & cc){
		if(!data.ENABLE_MIDI_CHANNEL_CONTROL)return;
		switch(cc.control){
			case MIDIFile::CC::VOLUME:
				cfg.Volume=std::pow(cc.value / 127.0f, 2.0f);
				break;
			case MIDIFile::CC::EXPRESSION:
				cfg.Expression=std::pow(cc.value / 127.0f, 2.0f);
				break;
			case MIDIFile::CC::BREATH:
				cfg.Breath=std::pow(cc.value / 127.0f, 2.0f);
				break;
			case MIDIFile::CC::FOOT:
				cfg.Foot=std::pow(cc.value / 127.0f, 2.0f);
				break;
			case MIDIFile::CC::PAN:
				if(auto dsp3d=data.getConfig().dsp3d){
					dsp3d->setPos((cc.value - 64.0f) / 127.0f, 0, 3);
				} else{
					if(cc.value == 64) cfg.Pan=0;
					else if(cc.value < 64) cfg.Pan=(cc.value - 64.0f) / 64.0f;
					else cfg.Pan=(cc.value - 64.0f) / 63.0f;
				}
				break;
			case MIDIFile::CC::SUSTAIN_SWITCH:
				if(cc.channelID == MIDI_DRUM_CHANNEL){
					cfg.Sustain=true;
					return;
				}
				cfg.Sustain=cc.value >= 64;
				closeNotSustainNotes(data, cfg);
				break;
			case MIDIFile::CC::RESET_ALL_CONTROLLERS:
				if(auto dsp3d=data.getConfig().dsp3d){
					dsp3d->setPos(0, 0, 3);
				}
				cfg.reset();
				break;
			case MIDIFile::CC::RESET_MUTE_ALL_NOTES:
			{
				auto & pool=data.workingNotesPool;
				pool.clear();
				pool.reset();
				mResetLimiter();
			}
			break;
			case MIDIFile::CC::ALL_NOTES_OFF:
			{
				cfg.allNotesOff();
				bool offMap[CHANNEL_MAX_NOTE_ID]{false};
				auto & pool=data.workingNotesPool;
				for(u_index i=0;i < pool.size();i++){
					auto & po=pool.get(i);
					NoteTask & n=*po.object;
					n.note.requestClose(cfg);
				}
				if(cfg.noteProcessor != nullptr){
					for(u_index i=0;i < CHANNEL_MAX_NOTE_ID;i++){
						if(offMap[i])cfg.noteProcessor->noteOff(cfg, i, 0);
					}
				}
				mResetLimiter();
			}
			break;
			case MIDIFile::CC::EFFECT_REVERB:
				if(enable_MIDI_CC_EFFECT)data.setReverb(cc.value / 127.0f);
				break;
			case MIDIFile::CC::EFFECT_CHORUS:
				if(enable_MIDI_CC_EFFECT)data.setChorus(cc.value / 127.0f);
				break;
			case MIDIFile::CC::EFFECT_DETUNE:
				if(enable_MIDI_CC_EFFECT)data.setDetune(cc.value / 127.0f);
				break;
			case MIDIFile::CC::EFFECT_PHASER:
				if(enable_MIDI_CC_EFFECT)data.setPhaser(cc.value / 127.0f);
				break;
			case MIDIFile::CC::RPN_MSB:
				cfg.nrpn.reset();
				cfg.rpn.reset();
				cfg.rpn.selectMSB(cc.value & 0x7f);
				break;
			case MIDIFile::CC::RPN_LSB:
				cfg.rpn.selectLSB(cc.value & 0x7f);
				break;
			case MIDIFile::CC::NRPN_MSB:
				cfg.rpn.reset();
				cfg.nrpn.reset();
				cfg.nrpn.selectMSB(cc.value & 0x7f);
				break;
			case MIDIFile::CC::NRPN_LSB:
				cfg.rpn.reset();
				cfg.nrpn.selectLSB(cc.value & 0x7f);
				break;
			case MIDIFile::CC::DATA_ENTRY_MSB://msb先来
				cfg.rpn.setDataMSB(cc.value & 0x7f);
				cfg.nrpn.setDataMSB(cc.value & 0x7f);
				procDataEntry(false, data, cfg);
				break;
			case MIDIFile::CC::DATA_ENTRY_LSB:
				cfg.rpn.setDataLSB(cc.value & 0x7f);
				cfg.nrpn.setDataLSB(cc.value & 0x7f);
				procDataEntry(true, data, cfg);
				break;
			case MIDIFile::CC::BANK:
				cfg.Bank=(cc.value & 0x7f) << 7;
				break;
			case MIDIFile::CC::BANK + 32:
				cfg.Bank|=(cc.value & 0x7f);
				break;
			case MIDIFile::CC::MONO_MODE:
				cfg.MonoMode=true;
				break;
			case MIDIFile::CC::POLY_MODE:
				cfg.MonoMode=false;
				break;
			case MIDIFile::CC::MODULATION:
				cfg.Modulation=(cc.value / 127.0f);
				break;
			case MIDIFile::CC::PORTAMENTO_TIME:
				cfg.PortamentoTime=(std::pow(10.0f, cc.value / 127.0f) - 1.0f) / 9.0f;
				break;
			case MIDIFile::CC::PORTAMENTO_SWITCH:
				cfg.Portamento=(cc.value >= 64);
				break;
			case MIDIFile::CC::LEGATO_EFFECT_SWITCH:
				cfg.Legato=(cc.value >= 64);
				break;
			case MIDIFile::CC::SOSTENUTO_SWITCH:
				cfg.Sostenuto=(cc.value >= 64);
				cfg.sostenutoChange();
				closeNotSustainNotes(data, cfg);
				break;
			case MIDIFile::CC::SOFT_PEDAL_SWITCH:
				cfg.SoftPedal=(cc.value >= 64);
				break;
			case MIDIFile::CC::VIBRATO_RATE:
				cfg.ModRate=std::pow(10.0f, cc.value / 127.0f) - 1.0f;
				break;
			case MIDIFile::CC::VIBRATO_DEPTH:
				cfg.ModDepth=(cc.value / 127.0f);
				break;
			case MIDIFile::CC::VIBRATO_DELAY:
				cfg.ModDelay=(std::pow(10.0f, cc.value / 127.0f) - 1.0f) / 9.0f;
				break;
			case MIDIFile::CC::ATTACK_TIME:
				/*if(enable_MIDI_CC_ADSR){
					std::shared_ptr<AHDSREnvelop> a=getAHDSREnv();
					if(a){
						a->attackTime=pow(10000.0, cc.value / 127.0f) / 1000.0;
					}
				}*/
				break;
			case MIDIFile::CC::DECAY_TIME:
				/*if(enable_MIDI_CC_ADSR){
					std::shared_ptr<AHDSREnvelop> a=getAHDSREnv();
					if(a){
						a->decayTime=pow(10000.0, cc.value / 127.0f) / 1000.0;
					}
				}*/
				break;
			case MIDIFile::CC::RELEASE_TIME:
				/*if(enable_MIDI_CC_ADSR){
					std::shared_ptr<AHDSREnvelop> a=getAHDSREnv();
					if(a){
						a->releaseTime=pow(10000.0, cc.value / 127.0f) / 1000.0;
					}
				}*/
				break;
			default:
				//std::cout << "Unimplemented Control: " << (int)cc.control << "=" << (int)cc.value << std::endl;
				break;
		}
		if(cfg.noteProcessor != nullptr) cfg.noteProcessor->cc(cfg, cc);
	}
	void Mixer2::procDataEntry(bool lsb, ChannelData & data, ChannelConfig & cfg){
		PNData & rpn=cfg.rpn;
		if(rpn.active){
			switch(rpn.select){
				case 0://setPitchBendRange
					cfg.PitchBendRange=((float)(rpn.dataMSB + rpn.dataLSB / 100.0));
					break;
				case 1://FineTune
					cfg.FineTune=(float)((rpn.data - 8192.0) / 8192.0);
					break;
				case 2://CoarseTune
					cfg.CoarseTune=rpn.dataMSB - 64;
					break;
					//default:
					//	System.out.println("Unimplemented RPN:"+rpnController+" = "+pnDataValue);
					//	break;
			}
		} else if(cfg.nrpn.active){
			procNRPN(lsb, data, cfg.nrpn.select, cfg.nrpn.data);
		}
	}
	void Mixer2::procNRPN(bool lsb, ChannelData & data, uint16_t nrpnController, uint16_t value){
		switch(nrpnController){
			case NRPN::MIXER_LIMITER:
				setUseLimiter(value >= 64);
				break;
			case NRPN::MIXER_ENABLE_MIDI_CC_EFFECT:
				enable_MIDI_CC_EFFECT=value >= 64;
				break;
			case NRPN::MIXER_ENABLE_MIDI_CC_ADSR:
				enable_MIDI_CC_ADSR=value >= 64;
				break;
			case NRPN::CHANNEL_3D_YAW:
				if(lsb)
					if(auto dsp3d=data.getConfig().dsp3d)
						dsp3d->setPos(value * 2.0 / PNData::LSB_MAX - 1.0, dsp3d->pitch, dsp3d->distance);
				break;
			case NRPN::CHANNEL_3D_PITCH:
				if(lsb)
					if(auto dsp3d=data.getConfig().dsp3d)
						dsp3d->setPos(dsp3d->yaw, value * 2.0 / PNData::LSB_MAX - 1.0, dsp3d->distance);
				break;
			case NRPN::CHANNEL_3D_DISTANCE:
				if(lsb)
					if(auto dsp3d=data.getConfig().dsp3d)
						dsp3d->setPos(dsp3d->yaw, dsp3d->pitch, value * 100 / PNData::LSB_MAX);
				break;
			case NRPN::CHANNEL_3D_X:
				if(lsb)
					if(auto dsp3d=data.getConfig().dsp3d)
						dsp3d->setPosXYZ(value * 1000 / PNData::LSB_MAX - 500.0, dsp3d->y, dsp3d->z);
				break;
			case NRPN::CHANNEL_3D_Y:
				if(lsb)
					if(auto dsp3d=data.getConfig().dsp3d)
						dsp3d->setPosXYZ(dsp3d->x, value * 1000 / PNData::LSB_MAX - 500.0, dsp3d->z);
				break;
			case NRPN::CHANNEL_3D_Z:
				if(lsb)
					if(auto dsp3d=data.getConfig().dsp3d)
						dsp3d->setPosXYZ(dsp3d->x, dsp3d->y, value * 1000 / PNData::LSB_MAX - 500.0);
				break;
			default:
				//System.out.println("Unimplemented NRPN:"+nrpnController+" = "+value);
				break;
		}
	}
	std::vector<std::shared_ptr<IChannel>> Mixer2::getAllChannels()const{
		std::vector<std::shared_ptr<IChannel>> chann;
		for(auto & i : allChannelData){
			chann.emplace_back(std::dynamic_pointer_cast<IChannel>(i));
		}
		return chann;
	}
	void Mixer2::resetLimiter(){
		std::unique_lock <std::shared_mutex > lock(dspLock);
		mResetLimiter();
	}
	void Mixer2::mResetLimiter(){
		for(u_index i=0;i < getOutputChannelCount();i++){
			masterLimiter[i]->resetMemory();
			nonDrumSetLimiter[i]->resetMemory();
			drumSetLimiter[i]->resetMemory();
		}
	}

	void Mixer2::procTuningChange(ChannelData & data, ChannelConfig & cfg, TuningChange & event){
		data.getConfig().setNoteTuning(event.value);
	}
	void Mixer2::procInstrument(ChannelData & data, ChannelConfig & cfg, ProgramChange & event){
		if(!data.ENABLE_MIDI_PROGRAM_CHANGE)return;
		NoteProcPtr ptr=nullptr;
		auto instr=data.getConfig().instrument;
		u_sample_rate sampleRate=getSampleRate();
		if(event.noteProcessor != nullptr){
			ptr=event.noteProcessor;
		} else if(instr == nullptr){
			std::cout << "InstrumentProvider not set" << std::endl;
			ptr=SynthUtil::getDefault();
		} else if(event.channelID == IMixer::MIDI_DRUM_CHANNEL){
			ptr=instr->getDrumSet(cfg.Bank, sampleRate);
			cfg.Sustain=true;
		} else{
			ptr=instr->get(cfg.Bank, event.id, sampleRate);
			if(ptr == nullptr){
				std::cout << "ProgramChange[Not found]: " << std::to_string(event.id) << ", CH:" << event.channelID << std::endl;
				ptr=SynthUtil::getDefault();
			}
		}
		if(ptr != nullptr){
			data.programCache.emplace(ptr);
			cfg.setNoteProcessor(ptr);
			ptr->init(cfg);
		}
		//std::cout << "ProgramChange: " << data.groupName << ", CH:" << (int)data.channelID << ", P:" << event.id << std::endl;
	}
	void Mixer2::closeNotSustainNotes(ChannelData & data, ChannelConfig & cfg){
		// 关闭未被延音保持的音符
		if(cfg.Sustain) return;
		bool offMap[CHANNEL_MAX_NOTE_ID]{false};
		auto & pool=data.workingNotesPool;
		for(u_index i=0; i < pool.size(); i++){
			auto & po=pool.get(i);
			NoteTask & n=*po.object;
			Note & note=n.note;
			if(cfg.sostenutoLock[note.id]) continue;
			if(!cfg.noteHoldMap[note.id]){
				offMap[note.id]=true;
				note.requestClose(cfg);
			}
		}
		if(cfg.noteProcessor != nullptr){
			for(u_index i=0;i < CHANNEL_MAX_NOTE_ID;i++){
				if(offMap[i])cfg.noteProcessor->noteOff(cfg, i, 0);
			}
		}
	}
	void Mixer2::setSynthMode(int8_t mode, int32_t cores){
		synthMode=mode;
		if(cores == -1) cores=Runtime::getRuntime().availableProcessors();
		if(mode == MODE_THREAD_POOL){
			delete threadPool;
			threadPool=new FixedThreadPool(cores);
		}
	}
	std::shared_ptr<yzrilyzr_dsp::DSPChain> * Mixer2::getEQ(){
		return finalEQ;
	}
	std::shared_ptr<IChannel> Mixer2::getMIDIChannel(const String & group, s_midichannel_id ch){
		std::unique_lock <std::shared_mutex > lock(channelLock);
		return getOrCreateMIDIChannelData(group, ch);
	}
	u_sample * Mixer2::getOutput(uint32_t chIndex)const{
		return output[chIndex]->_array;
	}
	s_sample_index Mixer2::getCurrentSampleIndex() const{
		return mixerCurrentSampleIndex;
	}
	bool Mixer2::hasMIDIChannel(const String & groupName, s_midichannel_id channelID){
		auto outerIt=channelData.find(groupName);
		if(outerIt == channelData.end())return false;
		auto & innerMap=outerIt->second;
		auto innerIt=innerMap.find(channelID);
		if(innerIt == innerMap.end())return false;
		return true;
	}
}