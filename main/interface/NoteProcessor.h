#pragma once
#include "SimpleSynth.h"

#include "util/ParamRegister.h"

namespace yzrilyzr_util{
	namespace ParamType{
		constexpr int NoteSrc=1000;
		constexpr int SampleData=1001;
		constexpr int PhaseSrc=1002;
		constexpr int OscSrc=1003;
	}
}
namespace yzrilyzr_dsp{
	class DSP;
}
namespace yzrilyzr_array{
	class SampleProvider;
}
namespace yzrilyzr_simplesynth{
	class Note;
	class IMixer;
	class IChannel;
	class ChannelControl;
	class PhaseSrc;
	class Osc;
	class NoteProcessor;
	class ChannelConfig;
	typedef std::shared_ptr<NoteProcessor> NoteProcPtr;
	ECLASS(NoteProcessor, public yzrilyzr_util::ParamRegister){
		public:
		virtual ~NoteProcessor()=default;
		virtual void init(ChannelConfig & cfg){}//初始化
		virtual inline u_sample getAmp(Note & note)=0;//渲染每个音符（音符生命周期由合成器管理，无需手动初始化，在这里仅填写声音是如何产生的）
		//给音符合并后的数据后处理（模拟箱体、混响、等等）
		virtual inline u_sample postProcess(u_sample output){
			return output;
		}
		//音符是否还有数据，供合成器管理生命周期
		virtual bool noMoreData(Note & note);
		virtual void cc(ChannelConfig & cfg, ChannelControl & cc){}
		virtual void noteOn(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel){}//接收NoteOn事件
		virtual void noteOff(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel){}//接收NoteOff事件
		virtual NoteProcPtr clone(){
			return NoteProcPtr(this);
		}
		void registerParamPhaseSrc(const yzrilyzr_lang::String & name, std::shared_ptr<PhaseSrc> *value);
		void registerParamSrc(const yzrilyzr_lang::String & name, NoteProcPtr * value);
		void registerParamOscSrc(const yzrilyzr_lang::String & name, std::shared_ptr<Osc> *value);
		void registerParamDSP(const yzrilyzr_lang::String & name, std::shared_ptr<yzrilyzr_dsp::DSP> *value);
		void registerParamSample(const yzrilyzr_lang::String & name, std::shared_ptr<yzrilyzr_array::SampleProvider> *value);
		yzrilyzr_lang::String toString() const override;
	};
}
