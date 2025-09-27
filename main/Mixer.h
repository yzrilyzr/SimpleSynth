#pragma once
#include "SimpleSynth.h"
#include "interface/IMixer.h"
#include "collection/HashMap.hpp"
#include "interface/NoteTuning.h"
#include "array/SampleArray.h"
#include <mutex>
#include "util/FixedThreadPool.h"
#include "util/Flag.h"
#include <map>
namespace yzrilyzr_dsp{
	class DSPChain;
	class Limiter;
}

namespace yzrilyzr_simplesynth{
	class Channel;
	class InstrumentProvider;

	ECLASS(Mixer, public IMixer){
		public:
		// ==================== 音频缓冲区相关 ====================

		Mixer(size_t bufferSize);     // 构造函数
		~Mixer();                      // 析构函数

		// 缓冲区管理
		void setBufferSize(size_t bs)override; // 设置缓冲区大小
		size_t getBufferSize()const override;   // 设置缓冲区大小
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
		std::shared_ptr<yzrilyzr_dsp::DSPChain> * getEQ()override;             // 获取均衡器链
		// ==================== MIDI快速消息 ====================
		void sendInstantEvent(ChannelEvent * event)override;
		void postEvent(ChannelEvent * event, u_time startAt)override;

		// ==================== 通道管理 ====================
		void addChannel(std::shared_ptr<Channel>channel);     // 添加通道
		void removeChannel(std::shared_ptr<Channel> channel);   // 移除指定通道
		void removeAllChannels();              // 移除所有通道

		void setMIDIChannel(s_midichannel_id id, std::shared_ptr<Channel> channel); // 设置通道
		void setMIDIChannel(const std::string & groupName, s_midichannel_id id, std::shared_ptr<Channel> channel); // 设置通道
		std::shared_ptr<IChannel> getMIDIChannel(const std::string & groupName, s_midichannel_id id)override; // 获取指定通道
		void removeMIDIChannel(s_midichannel_id id);   // 移除指定通道
		void removeMIDIChannel(const std::string & groupName, s_midichannel_id id);   // 移除指定通道

		// ==================== 事件统计 ====================
		size_t getCurrentProcessingNoteCount()override; // 获取当前处理的音符数量
		size_t getPostedEventCount()override;           // 获取已发布事件数量

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
		std::vector<std::shared_ptr<IChannel>> getAllChannels()const override;
		bool hasMIDIChannel(const std::string & group, s_midichannel_id id)override;

		private:
			// ==================== 私有成员变量 ====================
		std::shared_ptr<yzrilyzr_array::SampleArray> output[2]; // 输出缓冲区指针数组
		std::map<std::pair<std::string, s_midichannel_id>, std::shared_ptr<Channel>> midiChannelMap; // MIDI通道映射表
		yzrilyzr_collection::ArrayList<std::shared_ptr<Channel>> channels;                // 全部通道列表
		yzrilyzr_dsp::Limiter ** nonDrumSetLimiter=nullptr;      // 非鼓组限制器
		yzrilyzr_dsp::Limiter ** masterLimiter=nullptr;          // 主限制器
		std::recursive_mutex  channelLock;              // 通道访问锁
		std::recursive_mutex  midiChannelMapLock;              // 通道访问锁
		std::shared_ptr<yzrilyzr_dsp::DSPChain> finalEQ[2];               // 最终均衡器链

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