#pragma once
#include "SimpleSynth.h"
#include "util/ClassRegister.h"

namespace yzrilyzr_simplesynth{
	class NoteProcessor;
	class Note;
	class IMixer;
	class IChannel;
	class ChannelControl;
	class PhaseSrc;
	class Osc;
	class ChannelConfig;
	typedef u_sp<NoteProcessor> NoteProcPtr;
}
namespace yzrilyzr_dsp{
	class DSP;
	typedef u_sp<DSP> DSPPtr;
}
namespace yzrilyzr_array{
	class SampleProvider;
}
namespace yzrilyzr_util{
	namespace ParamType{
		constexpr int NoteSrc=2000;
		constexpr int SampleData=2001;
		constexpr int PhaseSrc=2002;
		constexpr int OscSrc=2003;
	}
	namespace RegisterUtil{
		U_EXPORT_API void registerParamPhaseSrc(yzrilyzr_util::ClassRegister & reg, const yzrilyzr_lang::String & name, u_sp<yzrilyzr_simplesynth::PhaseSrc> * value);
		U_EXPORT_API void registerParamSrc(yzrilyzr_util::ClassRegister & reg, const yzrilyzr_lang::String & name, yzrilyzr_simplesynth::NoteProcPtr * value);
		U_EXPORT_API void registerParamOscSrc(yzrilyzr_util::ClassRegister & reg, const yzrilyzr_lang::String & name, u_sp<yzrilyzr_simplesynth::Osc> * value);
		U_EXPORT_API void registerParamDSP(yzrilyzr_util::ClassRegister & reg, const yzrilyzr_lang::String & name, yzrilyzr_dsp::DSPPtr * value);
		U_EXPORT_API void registerParamSampleData(yzrilyzr_util::ClassRegister & reg, const yzrilyzr_lang::String & name, u_sp<yzrilyzr_array::SampleProvider> * value);
	}
}

namespace yzrilyzr_simplesynth{
	ECLASS(NoteProcessor, public yzrilyzr_util::ClassRegister){
	public:
	virtual ~NoteProcessor()=default;
	virtual void init(ChannelConfig & cfg){}//初始化
	virtual __forceinline inline u_sample getAmp(const Note & note)=0;//渲染每个音符（音符生命周期由合成器管理，无需手动初始化，在这里仅填写声音是如何产生的）
	virtual __forceinline inline void getAmpBlock(const Note * noteSnapshots, u_sample * output, u_index length);
	//给音符合并后的数据后处理（模拟箱体、混响、等等）
	virtual __forceinline inline void postProcess(u_sample * input, u_index length){ }
	//音符是否还有数据，供合成器管理生命周期
	virtual __forceinline inline bool noMoreData(const Note & note)const;
	virtual void cc(ChannelConfig & cfg, ChannelControl & cc){}
	virtual void noteOn(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel){}//接收NoteOn事件
	virtual void noteOff(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel){}//接收NoteOff事件
	virtual NoteProcPtr clone(){ return NoteProcPtr(this); }
	yzrilyzr_lang::String toString() const override;
	};
}
