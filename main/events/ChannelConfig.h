#pragma once
#include "SimpleSynth.h"
#include "events/PNData.h"
#include "array/Array.hpp"

namespace yzrilyzr_interpolator{
	class Interpolator;
}
namespace yzrilyzr_dsp{
	class DSP3D;
}
namespace yzrilyzr_simplesynth{
	class NoteTuning;
	class NoteProcessor;
	class Note;
	class IMixer;
	class IChannel;
	class ChannelEvent;
	class InstrumentProvider;
	typedef std::shared_ptr<NoteProcessor> NoteProcPtr;
	EBCLASS(ChannelConfig){
		private:
		NoteProcPtr sp_noteProcessor=nullptr;
		std::shared_ptr<yzrilyzr_dsp::DSP3D> sp_dsp3d=nullptr;
		std::shared_ptr<InstrumentProvider> sp_instrument=nullptr;
		std::shared_ptr<NoteTuning> sp_tuning=nullptr;
		std::shared_ptr<yzrilyzr_interpolator::Interpolator> sp_velocityMap=nullptr;
		public:
		//基础信息（需要初始化）
		u_sample_rate sampleRate=0;
		u_time currentTime=0;
		u_time deltaTime=0;
		IMixer * mixer=nullptr;
		IChannel * channel=nullptr;
		//通道信息（需要快照）
		bool Sustain=false;              // 延音开关
		bool MonoMode=false;            // 单音模式
		bool Portamento=false;           // 滑音开关
		bool Legato=false;               // 连奏模式
		bool Sostenuto=false;            // 选择性延音
		bool SoftPedal=false;            // 弱音开关
		float PortamentoTime=0.3f;    // 滑音时间(秒)
		float Modulation=0.0f;      // 调制轮总效果[0,1]
		float ModRate=5.0f;              // 调制轮速率[0,+)
		float ModDelay=0.3f;          // 调制轮延迟[0,+)(秒)
		float ModDepth=0.5f;             // 调制深度[0,+)
		float Volume=0.7f;          // 通道音量[0,1]
		float Pan=0.0f;            // 通道声像[-1,1]，0居中
		float ChannelPitchBend=0.0f;      // 弯音轮[-1,1]
		float PitchBendRange=2.0f;    // 弯音轮范围[0,+)(默认2个id)
		float Detune=0.0f;         // 失谐效果[0,1]
		float Expression=1.0f;      // 表情效果[0,1]
		float Breath=1.0f;      // 呼吸效果[0,1]
		float Foot=1.0f;      // 脚踏效果[0,1]
		float FineTune=0.0;           // 精调音调(id)
		float CoarseTune=0.0;          // 粗调音调(id)
		float NoteShift=0.0;            // 音符偏移量
		s_bank_id Bank=0;          // 音色库值
		PNData rpn;                      // RPN参数
		PNData nrpn;                     // NRPN参数
		bool noteHoldMap[CHANNEL_MAX_NOTE_ID]={false}; // 音符是否按下映射
		bool sostenutoLock[CHANNEL_MAX_NOTE_ID]={false};         // 选择性延音锁定状态
		//
		NoteProcessor * noteProcessor=nullptr;
		Note * lastNote=nullptr;// 上次触发NoteOn的音符
		NoteTuning * tuning=nullptr;
		yzrilyzr_interpolator::Interpolator * velocityMap=nullptr;
		yzrilyzr_dsp::DSP3D * dsp3d=nullptr;
		InstrumentProvider * instrument=nullptr;
		//
		void setContext(IMixer * mixer,  IChannel * channel);
		void postInstantEvent(ChannelEvent * event);
		void setNoteProcessor(NoteProcPtr val);
		void setOnlyChannelConfig(const ChannelConfig & other);
		void sostenutoChange();
		void allNotesOff();
		void reset();
		~ChannelConfig();
		//
		public:		
		//std::shared_ptr<InstrumentProvider> getInstrumentProvider()const;
		void setInstrumentProvider(std::shared_ptr<InstrumentProvider> instr);
		//std::shared_ptr<NoteTuning> getNoteTuning()const;
		void setNoteTuning(std::shared_ptr<NoteTuning> tun);
		//std::shared_ptr<yzrilyzr_interpolator::Interpolator> getNoteVelocityMap()const;
		void setNoteVelocityMap(std::shared_ptr<yzrilyzr_interpolator::Interpolator> val);
		void set3DEffect(std::shared_ptr<yzrilyzr_dsp::DSP3D> dsp3d);
		//std::shared_ptr<yzrilyzr_dsp::DSP3D> get3DEffect();
		void set(const ChannelConfig & other);
		static std::shared_ptr<ChannelConfig> DefaultConfig();
	};
}