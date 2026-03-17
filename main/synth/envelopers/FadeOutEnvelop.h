#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "Enveloper.h"
#include "events/Note.h"

namespace yzrilyzr_simplesynth{

/**
 * @brief 简单衰减包络，支持两种模式：
 * - OnNoteOn: 从 NoteOn 开始线性衰减到 0
 * - OnNoteOff: NoteOn 时保持 1，NoteOff 后开始线性衰减
 * 衰减速率由 fade 参数控制，fade=0 时永不衰减（始终输出 1）
 */
	ECLASS(FadeOutEnvelop, public Enveloper){
	public:

	static constexpr int OnNoteOn=0;   ///< 从 NoteOn 开始衰减
	static constexpr int OnNoteOff=1;  ///< 从 NoteOff 开始衰减


/**
 * @param fade 衰减速率 (1/秒)。例如 fade=2 表示 0.5 秒内从 1 线性降至 0。
 *             若 fade=0，则永不衰减。
 * @param mode 衰减模式
 */
	FadeOutEnvelop(float fade, int mode);
	FadeOutEnvelop()=default;
	virtual ~FadeOutEnvelop()=default;

	// NoteProcessor 接口
	u_sample getAmp(const Note & note) override;
	bool noMoreData(const Note & note) override;
	void init(ChannelConfig & cfg) override;
	u_sp<NoteProcessor> clone() override;
	yzrilyzr_lang::String toString() const override;
	void onRegisterParam()override;
	U_CLASS_INFO_B(FadeOutEnvelop, Enveloper);

	private:
	float fade;   ///< 衰减速率 (1/秒)
	int mode;    ///< 当前模式
	};

} // namespace yzrilyzr_simplesynth