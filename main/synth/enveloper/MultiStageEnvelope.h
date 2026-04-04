#pragma once
#include "SimpleSynth.h"
#include "events/Note.h"
#include "events/NoteData.hpp"
#include "interface/NoteProcessor.h"
#include "synth/enveloper/Enveloper.h"
#include "interpolator/Interpolator.h"
#include <vector>
#include <memory>

namespace yzrilyzr_simplesynth{
	/**
	 * 行为：
	 *
	 * 没有循环区间：
	 * 包络值：触发后按时间走完，直至最后一个点，保持最后一个点的值
	 *
	 * 中途note off：忽略
	 *
	 * 生命周期控制：直至最后一个点且note.closed，则noMoreData
	 *
	 * 有循环区间：
	 * 包络值：触发后按时间走完，直至循环终点，跳回循环起点，无限循环
	 *
	 * 中途note off：直接跳到循环终点，循环终点（只有该点）的Y不使用原始值，而是使用currentVol（保证包络连续），走完剩余点
	 *
	 * 生命周期控制：退出循环后（此时note.closed）直至最后一个点，则noMoreData
	 */

	/**
	 * MSEPointType标记
	 * Attack 区域：
	 *     如有 DECAY：[0， DECAY]
	 *     否则不生效
	 *
	 * Decay 区域：
	 *     如有 DECAY 和 SUSTAIN_OR_LOOP_END：[DECAY, SUSTAIN_OR_LOOP_END]
	 *     如有 SUSTAIN_OR_LOOP_END：[0, SUSTAIN_OR_LOOP_END]
	 *     否则：[0, size()]
	 *
	 * Sustain 区域：
	 *     如有 DECAY 和 SUSTAIN_OR_LOOP_END：[DECAY, SUSTAIN_OR_LOOP_END]
	 *     如有 SUSTAIN_OR_LOOP_END：[0, size()]
	 *     否则不生效
	 *
	 * Release 区域：
	 *     需要 SUSTAIN_OR_LOOP_END：[SUSTAIN_OR_LOOP_END, size()]
	 *     否则不生效
	 *
	 * DECAY 可以和 LOOP_START 一起
	 * DECAY 不能在 SUSTAIN_OR_LOOP_END 之后
	 * LOOP_START 不能在 SUSTAIN_OR_LOOP_END 之后
	 *
	 * 有了 SUSTAIN_OR_LOOP_END 标志，其他标志都是非法
	 */
	enum class MSEPointType{
		DEFAULT=0b0,
		DECAY=0b1,
		LOOP_START=0b10,
		SUSTAIN_OR_LOOP_END=0b100,
	};
	enum class MSEPointMode{
		HOLD,
		SMOOTH,
		SINGLE_CURVE,
		DOUBLE_CURVE,
		HALF_SINE,
		STAGE,
		SMOOTH_STAGE,
		PULSE,
		WAVE,
	};
	struct MSEPoint{
		float x=0;
		float y=0;
		MSEPointType type;
		MSEPointMode mode;
		float modeValue=0;
		u_index index=0;//无需初始化指定
	};
	EBCLASS(MultiStageEnvelopeKeyData){
		public:
		u_sample currentVol;   // 当前包络值
		MSEPoint * start;      // 当前段的起始点
		MSEPoint * end;        // 当前段的结束点
		bool isClosed;
		u_sample loopEndVol;
		uint64_t loopCount;
	};
	ECLASS(MultiStageEnvelope, public Enveloper, NoteData<MultiStageEnvelopeKeyData>){
		public:
		MSEPoint * loopStartPoint=nullptr;
		MSEPoint * sustainOrLoopEndPoint=nullptr;
		std::vector<MSEPoint> points;
		MultiStageEnvelope();
		MultiStageEnvelope(const std::vector<MSEPoint> &points);
		bool noMoreData(const Note & note) const override;
		NoteProcPtr clone() override;
		void init(ChannelConfig & cfg) override;
		u_sample getAmp(const Note & note) override;
		yzrilyzr_lang::String toString() const override;
		MultiStageEnvelopeKeyData * init(MultiStageEnvelopeKeyData * data, const Note & note) override;
		U_CLASS_INFO_B(MultiStageEnvelope, Enveloper);


		public:
		void calcNextPoint(MultiStageEnvelopeKeyData & data, float x);
		static u_sample calcEnv(MSEPoint & start, MSEPoint & end, float x);
		static u_sample calcEnv(float xs, float xe, MSEPoint & end, float x);
		/*
		* @param x1 y1: 开始点
		* @param x2 y2: 结束点
		* @param mode: 传入结束点的模式
		* @param modeValue: 传入结束点的模式参数
		* @param x: 传入当前时间点
		*/
		static u_sample calcEnv(float xs, float ys, float xe, float ye, MSEPointMode mode, float modeValue, float x);
	};
}