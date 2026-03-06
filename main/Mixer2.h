#pragma once
#include "SimpleSynth.h"
#include "dsp/DSP.h"
#include "events/ChannelEvent.h"
#include "events/Note.h"
#include "interface/IChannel.h"
#include "interface/IMixer.h"
#include "util/FixedThreadPool.h"
#include "util/Flag.h"
#include "util/Pool2.hpp"
#include "yzrutil.h"
#include <cstdint>
#include <deque>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

namespace yzrilyzr_dsp{
	class DSPChain;
	class Limiter;
}

namespace yzrilyzr_simplesynth{
	class ChannelEvent;
	class InstrumentProvider;
	class ChannelConfig;
	class ChannelData;
	EBCLASS(NoteTask){
		public:
		Note note;
		yzrilyzr_array::SampleArray output;
		//yzrilyzr_array::Array<Note> noteSnapshots;
		ChannelData * data; 
		NoteTask(uint8_t uniqueID);
	};
	ECLASS(NoteTaskPool, public yzrilyzr_util::Pool2<NoteTask, CHANNEL_MAX_VOICE>){
		private:
		uint8_t uniqueID=0;
		protected:
		NoteTask * newInstance() override;
		void onReuse(NoteTask * note) override;
		void onClear() override;
	};
	ECLASS(ChannelData, public IChannel){
		public:
		std::vector<u_sp<ChannelConfig>> cfgSnapshots;
		yzrilyzr_array::SampleArray output[2];
		yzrilyzr_array::SampleArray noteOutput;
		u_sp<yzrilyzr_dsp::DSPChain> dspChain[2]; // DSP处理链
		u_sp<yzrilyzr_dsp::DSP> choruser[2];     // 合唱效果器
		u_sp<yzrilyzr_dsp::DSP> phaser[2];     // 合唱效果器
		u_sp<yzrilyzr_dsp::DSP> reverber[2];      // 混响效果器
		u_sp<yzrilyzr_dsp::DSP> limiter[2];
		std::set<NoteProcPtr> programCache;
		NoteTaskPool workingNotesPool;
		bool lastSnapshotChange=true;
		ChannelData(const yzrilyzr_lang::String & groupName, s_midichannel_id channelID, uint32_t bufSize);
		void setSampleRate(u_sample_rate sr)override;
		u_sample * getOutput(uint32_t chIndex)const override;
		ChannelConfig & getConfig()override;
		yzrilyzr_dsp::Chorus & getChorus(u_index ch)const override;
		yzrilyzr_dsp::Freeverb & getReverb(u_index ch)const override;
		yzrilyzr_dsp::Phaser & getPhaser(u_index ch)const override;
		void reset()override;
	};
	ECLASS(Mixer2, public IMixer){
		public:
		Mixer2(u_index bufferSize);
		~Mixer2();
		void setBufferSize(u_index bs)override;
		u_index getBufferSize()const override;
		void setSynthMode(int8_t mode, int32_t cores)override;
		void mix()override;
		void sendInstantEvent(ChannelEvent * event)override;
		void postEvent(ChannelEvent * event, u_time startAt)override;
		u_time getCurrentTime()const override;
		void setSampleRate(u_sample_rate sam)override;
		void resetLimiter()override;            // 重置限制器状态
		void reset()override;
		u_index getCurrentProcessingNoteCount()override;
		u_index getPostedEventCount()override;
		s_sample_index getCurrentSampleIndex() const override;
		bool hasData() override;
		u_sample * getOutput(uint32_t chIndex)const override;
		std::vector<u_sp<IChannel>> getAllChannels()const override;
		u_sp<IChannel> getMIDIChannel(const yzrilyzr_lang::String & group, s_midichannel_id ch)override;
		u_sp<yzrilyzr_dsp::DSPChain> * getEQ()override;
		bool hasMIDIChannel(const yzrilyzr_lang::String & group, s_midichannel_id id)override;
		private:
		static constexpr int const FLAG_RESET=0b1;
		yzrilyzr_array::SampleArray output[2]; // 输出缓冲区指针数组
		yzrilyzr_array::SampleArray drumOutput[2]; // 输出缓冲区指针数组
		std::deque<ChannelEvent *> instantEventQueue; // 即时事件队列
		std::deque<ChannelEvent *> postEventQueue; // 即时事件队列
		yzrilyzr_util::FixedThreadPool * threadPool=nullptr;
		std::vector<std::future<void>> futures;
		int32_t synthMode=0;
		uint64_t mixerCurrentSampleIndex=0;
		u_sp<yzrilyzr_dsp::Limiter>nonDrumSetLimiter[2];      // 非鼓组限制器
		u_sp<yzrilyzr_dsp::Limiter>drumSetLimiter[2];      // 鼓组限制器
		u_sp<yzrilyzr_dsp::Limiter>masterLimiter[2];      // 主限制器
		u_sp<yzrilyzr_dsp::DSPChain> finalEQ[2];               // 最终均衡器链
		std::unordered_map<yzrilyzr_lang::String, std::unordered_map<s_midichannel_id, u_sp<ChannelData>>> channelData;
		std::vector<u_sp<ChannelData>> allChannelData;
		yzrilyzr_util::Flag flags;
		u_sp<ChannelData> getOrCreateMIDIChannelData(const yzrilyzr_lang::String & groupName, s_midichannel_id channelID);
		void mReset();
		void mResetLimiter();
		void procEvent(ChannelData & data, ChannelConfig & cfg, ChannelEvent & event);
		void procNoteOn(ChannelData & data, ChannelConfig & cfg, NoteOn & event);    // 处理音符开启事件
		void procNoteOff(ChannelData & data, ChannelConfig & cfg, NoteOff & event);  // 处理音符关闭事件
		void procNotePressure(ChannelData & data, ChannelConfig & cfg, NotePressure & event); // 处理音符力度事件
		void procChannelPitchBend(ChannelData & data, ChannelConfig & cfg, ChannelPitchBend & event); // 处理音符弯音事件
		void procChannelPressure(ChannelData & data, ChannelConfig & cfg, ChannelPressure & event); // 处理通道力度事件
		void procChannelControl(ChannelData & data, ChannelConfig & cfg, ChannelControl & event); // 处理控制事件
		void procNotePitchBend(ChannelData & data, ChannelConfig & cfg, NotePitchBend & event); // 处理音符弯音事件
		void procInstrument(ChannelData & data, ChannelConfig & cfg, ProgramChange & event); // 处理乐器变更事件
		void procTuningChange(ChannelData & data, ChannelConfig & cfg, TuningChange & event); // 处理乐器变更事件
		void procDataEntry(bool lsb, ChannelData & data, ChannelConfig & cfg);
		void procNRPN(bool lsb, ChannelData & data, uint16_t nrpnController, uint16_t value);
		void procNoteTask(NoteTask & task);
		void procNoteMix(ChannelData & data, std::vector< NoteTask *> *noteTasks);
		void waitForAllTasks();
		void setDataSnapshotBaseInfo(ChannelData & data);
		void transferSnapshot(ChannelData & data, int32_t startInc);
		void closeNotSustainNotes(ChannelData & data, ChannelConfig & cfg);
		private:
		void processChannelSnapshots();
		void processInstantEvents(u_time deltaTime, u_index bufSize);
		void processScheduledEvents(u_time deltaTime, u_index bufSize);
		void synthesizeNotes();
		void prepareNoteMixTasks(std::unordered_map<ChannelData *, std::vector<NoteTask *>>&noteMixTasks);
		void submitNoteMixTasks(std::unordered_map<ChannelData *, std::vector<NoteTask *>>&noteMixTasks);
		void cleanupFinishedNotes();
		void mixNonDrumChannelsToOutput(u_index chc, u_index bufSize);
		void processNonDrumLimiters(u_index chc, u_index bufSize);
		void mixDrumChannelsToOutput(u_index chc, u_index bufSize);
		void processDrumLimiters(u_index chc, u_index bufSize);
		void mixDrumToMainOutput(u_index chc, u_index bufSize);
		void processMasterEffects(u_index chc, u_index bufSize);
		void finalizeMix(u_index bufSize);
	};
}