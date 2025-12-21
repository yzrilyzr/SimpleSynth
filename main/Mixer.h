#pragma once
#include "SimpleSynth.h"
#include "array/Array.hpp"
#include "collection/HashMap.hpp"
#include "collection/LinkedList.hpp"
#include "events/ChannelEvent.h"
#include "events/Note.h"
#include "events/PNData.h"
#include "interface/IChannel.h"
#include "interface/IMixer.h"
#include "interface/InstrumentProvider.h"
#include "interface/NoteTuning.h"
#include "synth/envelopers/AHDSREnvelop.h"
#include "util/FixedThreadPool.h"
#include "util/Flag.h"
#include "util/Pool2.hpp"
#include "util/Util.h"
#include <map>
#include <mutex>
namespace yzrilyzr_dsp{
	class DSPChain;
	class Limiter;
}
namespace std{
	template<>
	struct hash<std::pair<yzrilyzr_lang::String, s_midichannel_id>>{
		size_t operator()(const std::pair<yzrilyzr_lang::String, s_midichannel_id> & obj) const noexcept{
			return static_cast<size_t>(obj.first.hashCode() ^ obj.second);
		}
	};
}
namespace yzrilyzr_simplesynth{
	class Channel;
	class InstrumentProvider;
	ECLASS(NotePool, public yzrilyzr_util::Pool2<Note, CHANNEL_MAX_VOICE >){
		public:
		void reset();
		protected:
		Note * newInstance() override;
		void onReuse(Note * note) override;
		uint8_t uniqueID=0;
	};
	ECLASS(Channel, public IChannel){
		public:
			// ==================== 公共成员变量 ====================
		uint8_t channelCount=2;          // 声道数(默认立体声)
		yzrilyzr_array::SampleArray output[2]; // 输出缓冲区指针数组
		u_time_stamp lastActiveTime=0;         // 最后活跃时间
		bool alwaysActive=false;        // 是否始终保持活跃状态
		NotePool workingNotesPool; // 工作音符池
		ChannelConfig channelConfig;

		// ==================== 构造与析构 ====================
		Channel();                         // 默认构造函数
		~Channel();                        // 析构函数

		// ==================== 缓冲区管理 ====================
		void setBufferSize(u_index bs);   // 设置缓冲区大小
		u_index getBufferSize()const;   // 设置缓冲区大小
		bool hasData();                    // 检查是否有数据需要处理
		void reset()override;                      // 重置通道状态
		void resetChannel();               // 完全重置通道
		void fillBuffer();                 // 填充音频缓冲区

		// ==================== 时间管理 ====================
		u_time getCurrentTime() const;     // 获取当前播放时间(秒)
		u_time_f getEventDeltaTime() const;// 获取事件处理间隔时间
		void setEventDeltaTime(u_freq Hz); // 设置事件处理频率

		// ==================== 状态查询 ====================
		u_index getPostedEventCount()const;      // 获取已发布事件数量
		u_index getCurrentProcessingNoteCount()const; // 获取当前处理的音符数量
		u_time_f getProcessTime() const;   // 获取通道处理时间
		void setAlwaysActive(bool v);

		// ==================== 通道属性 ====================
		s_midichannel_id getChannelId() const; // 获取通道ID
		void setChannelId(s_midichannel_id id); // 设置通道ID
		u_sample_rate getSampleRate()const;     // 获取采样率
		void setSampleRate(u_sample_rate sr)override; // 设置采样率
		s_note_id getNoteShift() const;       // 获取音符偏移量
		void setNoteShift(int8_t noteShift); // 设置音符偏移量
		u_sp<AHDSREnvelop> getAHDSREnv()const; // 获取AHDSR包络

		// ==================== MIDI控制方法 ====================
		bool isMonoMode() const;           // 检查是否为单音模式
		void setMonoMode(bool monoMode);   // 设置单音模式
		u_normal_01_f getModulation() const; // 获取调制值
		void setModulation(u_normal_01_f v); // 设置调制值
		u_normal_01_f getExpression() const; // 获取表情值
		void setExpression(u_normal_01_f i);  // 设置表情值
		u_normal_01_f getBreath() const; // 获取表情值
		void setBreath(u_normal_01_f i);  // 设置表情值
		u_normal_01_f getFoot() const; // 获取表情值
		void setFoot(u_normal_01_f i);  // 设置表情值
		u_normal_11_f getPan() const;      // 获取声像位置
		void setPan(u_normal_11_f pan);    // 设置声像位置
		s_note_id getPitchBendRange() const; // 获取弯音范围
		void setPitchBendRange(s_note_id pitchBendRange); // 设置弯音范围
		u_normal_01_f getVolume() const;  // 获取音量
		void setVolume(u_normal_01_f volume); // 设置音量
		u_normal_11_f getPitchBend() const; // 获取弯音值
		void setPitchBend(u_normal_11_f pitchBend1); // 设置弯音值
		u_normal_01_f getDetune() const;  // 获取失谐值
		void setSostenuto(bool b);        // 设置选择性延音
		void setPortamento(bool b);       // 设置滑音开关
		void setPortamentoTime(u_time_f v); // 设置滑音时间
		void setModDelay(u_time_f v);     // 设置调制延迟
		void setModDepth(float v);        // 设置调制深度
		void setModRate(float v);         // 设置调制速率
		void setLegato(bool legato);      // 设置连奏模式
		void setSoftPedal(bool softPedal); // 设置弱音开关
		bool isLegato() const;            // 检查连奏状态
		bool isPortamento() const;        // 检查滑音状态
		u_time_f getPortamentoTime() const; // 获取滑音时间
		bool isSustain() const;           // 检查延音状态
		bool isSoftPedal() const;         // 检查弱音状态
		bool isSostenuto() const;         // 检查选择性延音状态
		void setSustain(bool sustain);    // 设置延音状态
		bool isDrumSetChannel() const;    // 检查是否为鼓组通道
		void setDrumSetChannel(bool value); // 设置鼓组通道状态

		// ==================== 事件处理 ====================
		void noteOn(s_note_id_i noteId, s_note_vel velocity); // 音符开启
		void noteOff(s_note_id_i noteId); // 音符关闭
		void sendInstantEvent(ChannelEvent * n1); // 发送即时事件
		void sendPostEvent(ChannelEvent * n1, u_time startAt); // 发送延时事件

		// ==================== DSP效果处理 ====================
		void addDSPToChain(u_sp<yzrilyzr_dsp::DSP> *dsp); // 添加DSP效果到处理链
		yzrilyzr_dsp::Chorus & getChorus(u_index ch)const override; // 获取合唱效果器
		yzrilyzr_dsp::Phaser & getPhaser(u_index ch)const override; // 获取相位效果器
		yzrilyzr_dsp::Freeverb & getReverb(u_index ch)const override; // 获取混响效果器
		u_sample * getOutput(uint32_t chIndex)const override;
		ChannelConfig & getConfig()override;
		// ==================== RPN/NRPN参数 ====================
		PNData & getRPN();                 // 获取RPN参数
		PNData & getNRPN();                // 获取NRPN参数

		private:
			// ==================== 私有成员变量 ====================
		std::recursive_mutex eventLock;       // 事件队列锁
		std::recursive_mutex noteLock;        // 音符操作锁
		yzrilyzr_collection::LinkedList<ChannelEvent *> postEventQueue; // 延时事件队列
		yzrilyzr_collection::LinkedList<ChannelEvent *> instantEventQueue; // 即时事件队列
		u_sp<yzrilyzr_dsp::DSPChain> dspChain[2]; // DSP处理链
		u_sp<yzrilyzr_dsp::DSP> choruser[2];     // 合唱效果器
		u_sp<yzrilyzr_dsp::DSP> phaser[2];     // 合唱效果器
		u_sp<yzrilyzr_dsp::DSP> reverber[2];      // 混响效果器
		u_sp<yzrilyzr_dsp::DSP> panner[2];       // 声像效果器
		u_sp<yzrilyzr_dsp::DSP> limiter[2];      // 限制器
		s_midichannel_id channelID=-1;     // 通道ID
		u_time_f processTime;            // 处理时间统计
		bool commited=false;           // 提交状态
		bool isDrumSet=false;          // 是否为鼓组通道
		u_time_f eventProcessDeltaTime=0.001f; // 事件处理间隔时间
		u_time_f eventTimeSum=0;       // 事件处理时间累计
		bool lastSostenutoState=false; // 上次选择性延音状态
		bool lastSustainState=false;   // 上次延音状态

		// 事件处理
		public:
		void procEvent(ChannelEvent & event);
		void procNoteOn(NoteOn & note);    // 处理音符开启事件
		void procNoteOff(NoteOff & note);  // 处理音符关闭事件
		void procNotePressure(NotePressure & note); // 处理音符力度事件
		void procChannelPressure(s_note_vel value); // 处理通道力度事件
		void procChannelControl(ChannelControl & cc); // 处理控制事件
		void procNotePitchBend(NotePitchBend & note); // 处理音符弯音事件
		void procInstrument(ProgramChange & event); // 处理乐器变更事件
		private:
		void procNRPN(bool lsb, uint16_t nrpnController, uint16_t value); // 处理NRPN事件
		void procDataEntry();             // 处理数据输入事件
		// 音符状态管理
		void closeNotSustainNotes();
		void checkSostenuto();
		void checkSustainState();

	};
	ECLASS(Mixer, public IMixer){
		public:
		// ==================== 音频缓冲区相关 ====================

		Mixer(u_index bufferSize);     // 构造函数
		~Mixer();                      // 析构函数

		// 缓冲区管理
		void setBufferSize(u_index bs)override; // 设置缓冲区大小
		u_index getBufferSize()const override;   // 设置缓冲区大小
		void reset()override;                   // 重置混音器状态
		bool hasData()override;                 // 检查是否有数据需要处理
		void pause(bool pause);         // 暂停/恢复混音器
		bool isPaused() const;          // 获取暂停状态

		// ==================== 采样率相关 ====================
		void setSampleRate(u_sample_rate sr)override; // 设置采样率

		// ==================== 合成模式相关 ====================
		void setSynthMode(int8_t mode, int32_t cores)override; // 设置合成模式

		// ==================== DSP处理相关 ====================
		void resetLimiter()override;            // 重置限制器状态

		void setEQ(int32_t seg, double value); // 设置均衡器参数
		u_sp<yzrilyzr_dsp::DSPChain> * getEQ()override;             // 获取均衡器链
		// ==================== MIDI快速消息 ====================
		void sendInstantEvent(ChannelEvent * event)override;
		void postEvent(ChannelEvent * event, u_time startAt)override;

		// ==================== 通道管理 ====================
		void addChannel(u_sp<Channel>channel);     // 添加通道
		void removeChannel(u_sp<Channel> channel);   // 移除指定通道
		void removeAllChannels();              // 移除所有通道

		void setMIDIChannel(s_midichannel_id id, u_sp<Channel> channel); // 设置通道
		void setMIDIChannel(const yzrilyzr_lang::String & groupName, s_midichannel_id id, u_sp<Channel> channel); // 设置通道
		u_sp<IChannel> getMIDIChannel(const yzrilyzr_lang::String & groupName, s_midichannel_id id)override; // 获取指定通道
		void removeMIDIChannel(s_midichannel_id id);   // 移除指定通道
		void removeMIDIChannel(const yzrilyzr_lang::String & groupName, s_midichannel_id id);   // 移除指定通道

		// ==================== 事件统计 ====================
		u_index getCurrentProcessingNoteCount()override; // 获取当前处理的音符数量
		u_index getPostedEventCount()override;           // 获取已发布事件数量

		// ==================== 时间管理 ====================
		u_time getCurrentTime() const override;          // 获取当前时间
		s_sample_index getCurrentSampleIndex() const override; // 获取当前采样索引
		u_time_stamp getIdleChannelLiveTime() const{ // 获取空闲通道存活时间
			return idleChannelLiveTime;
		}
		void setIdleChannelLiveTime(u_time_stamp t){  // 设置空闲通道存活时间
			idleChannelLiveTime=t;
		}

		// ==================== 采样跳过 ====================
		u_sample_rate getSkipSample() const;    // 获取跳过的采样数
		void setSkipSample(u_sample_rate skip);  // 设置跳过的采样数

		// ==================== 混音核心功能 ====================
		void commitChannels();
		void mixChannels();
		void waitForChannels();
		void mix()override;
		u_sample * getOutput(uint32_t chIndex)const override;
		std::vector<u_sp<IChannel>> getAllChannels()const override;
		bool hasMIDIChannel(const yzrilyzr_lang::String & group, s_midichannel_id id)override;

		private:
			// ==================== 私有成员变量 ====================
		yzrilyzr_array::SampleArray output[2]; // 输出缓冲区指针数组
		std::unordered_map<std::pair<yzrilyzr_lang::String, s_midichannel_id>, u_sp<Channel>> midiChannelMap; // MIDI通道映射表
		yzrilyzr_collection::ArrayList<u_sp<Channel>> channels;                // 全部通道列表
		yzrilyzr_dsp::Limiter ** nonDrumSetLimiter=nullptr;      // 非鼓组限制器
		yzrilyzr_dsp::Limiter ** masterLimiter=nullptr;          // 主限制器
		std::recursive_mutex  channelLock;              // 通道访问锁
		std::recursive_mutex  midiChannelMapLock;              // 通道访问锁
		u_sp<yzrilyzr_dsp::DSPChain> finalEQ[2];               // 最终均衡器链

		s_sample_index currentSampleIndex=0;      // 当前采样索引
		bool _pause=false;                        // 暂停状态
		u_sample_rate skipSample=1;               // 采样跳过设置

		u_time_f processTime=0;                   // 处理时间统计
		u_time_stamp idleChannelLiveTime=300000;         // 空闲通道存活时间
		yzrilyzr_util::FixedThreadPool * threadPool=nullptr;      // 线程池
		std::vector<std::future<void>> futures;
		yzrilyzr_util::Flag flags;

		static constexpr uint32_t FLAG_RESET_INDEX=0b1;
		static constexpr uint32_t FLAG_RESET_LIMITER=0b10;
		static constexpr uint32_t FLAG_RESET_BUFFER=0b1000;
		static constexpr uint32_t FLAG_RESET_EQ=0b10000;
	};
}
