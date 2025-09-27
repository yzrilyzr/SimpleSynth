#pragma once
#include "SimpleSynth.h"
#include "interface/IChannel.h"
#include "interface/NoteTuning.h"
#include "events/ChannelEvent.h"
#include "events/Note.h"
#include "events/PNData.h"
#include "interface/InstrumentProvider.h"
#include "synth/envelopers/AHDSREnvelop.h"
#include "collection/LinkedList.hpp"
#include <mutex>
#include "util/Util.h"
#include "util/Pool2.hpp"
#include "util/FixedThreadPool.h"

namespace yzrilyzr_dsp{
	class DSPChain;
	//class Chorus;
	class Limiter;
	class HRIR;
	//class Freeverb;
	//class Phaser;
}
namespace yzrilyzr_simplesynth{
	class Mixer;
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
		std::shared_ptr<yzrilyzr_array::SampleArray> output[2]; // 输出缓冲区指针数组
		u_time_stamp lastActiveTime=0;         // 最后活跃时间
		bool alwaysActive=false;        // 是否始终保持活跃状态
		NotePool workingNotesPool; // 工作音符池
		ChannelConfig channelConfig;

		// ==================== 构造与析构 ====================
		Channel();                         // 默认构造函数
		~Channel();                        // 析构函数

		// ==================== 缓冲区管理 ====================
		void setBufferSize(size_t bs);   // 设置缓冲区大小
		size_t getBufferSize()const;   // 设置缓冲区大小
		bool hasData();                    // 检查是否有数据需要处理
		void reset()override;                      // 重置通道状态
		void resetChannel();               // 完全重置通道
		void fillBuffer();                 // 填充音频缓冲区

		// ==================== 时间管理 ====================
		u_time getCurrentTime() const;     // 获取当前播放时间(秒)
		u_time_f getEventDeltaTime() const;// 获取事件处理间隔时间
		void setEventDeltaTime(u_freq Hz); // 设置事件处理频率

		// ==================== 状态查询 ====================
		size_t getPostedEventCount()const;      // 获取已发布事件数量
		size_t getCurrentProcessingNoteCount()const; // 获取当前处理的音符数量
		u_time_f getProcessTime() const;   // 获取通道处理时间
		void setAlwaysActive(bool v);

		// ==================== 通道属性 ====================
		s_midichannel_id getChannelId() const; // 获取通道ID
		void setChannelId(s_midichannel_id id); // 设置通道ID
		u_sample_rate getSampleRate()const;     // 获取采样率
		void setSampleRate(u_sample_rate sr)override; // 设置采样率
		s_note_id getNoteShift() const;       // 获取音符偏移量
		void setNoteShift(int8_t noteShift); // 设置音符偏移量
		std::shared_ptr<AHDSREnvelop> getAHDSREnv()const; // 获取AHDSR包络

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
		void setDetune(u_normal_01_f ccDetune); // 设置失谐值
		void setPhaser(u_normal_01_f ccPhaser); // 设置相位效果
		void setChorus(u_normal_01_f chorus); // 设置合唱效果
		void setReverb(u_normal_01_f reverb); // 设置混响效果
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
		void addDSPToChain(std::shared_ptr<yzrilyzr_dsp::DSP> *dsp); // 添加DSP效果到处理链
		yzrilyzr_dsp::HRIR & getHRIR();    // 获取HRIR(头部相关传输函数)
		void setHRIR(yzrilyzr_dsp::HRIR * hrir); // 设置HRIR
		yzrilyzr_dsp::Chorus & getChorus(size_t ch)const override; // 获取合唱效果器
		yzrilyzr_dsp::Phaser & getPhaser(size_t ch)const override; // 获取相位效果器
		yzrilyzr_dsp::Freeverb & getReverb(size_t ch)const override; // 获取混响效果器
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
		std::shared_ptr<yzrilyzr_dsp::DSPChain> dspChain[2]; // DSP处理链
		std::shared_ptr<yzrilyzr_dsp::DSP> choruser[2];     // 合唱效果器
		std::shared_ptr<yzrilyzr_dsp::DSP> phaser[2];     // 合唱效果器
		std::shared_ptr<yzrilyzr_dsp::DSP> reverber[2];      // 混响效果器
		std::shared_ptr<yzrilyzr_dsp::DSP> panner[2];       // 声像效果器
		std::shared_ptr<yzrilyzr_dsp::DSP> limiter[2];      // 限制器
		yzrilyzr_dsp::HRIR * hrir=nullptr;         // HRIR处理器
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
}