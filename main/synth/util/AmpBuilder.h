#pragma once
#include "array/Array.hpp"
#include "dsp/DSP.h"
#include "dsp/FilterPassType.h"
#include "dsp/IIRUtil.h"
#include "interface/NoteProcessor.h"
#include "interpolator/Interpolator.h"
#include "synth/enveloper/MultiStageEnvelope.h"
#include "synth/envfilter/BiquadEnvFilterGroup.h"

namespace yzrilyzr_simplesynth{
	// 音频振幅构建器，用于处理音频信号的振幅、调制和滤波等效果
	EBCLASS(AmpBuilder){
	private:
	NoteProcPtr _src; // 音频源处理指针

	public:
		// 构造函数
	AmpBuilder(){} // 默认构造函数
	AmpBuilder(NoteProcPtr src){ this->_src=src; } // 带初始音频源的构造函数

	// 基础设置与源操作
	AmpBuilder & src(NoteProcPtr src); // 设置音频源为NoteProcPtr类型
	AmpBuilder & src(double src); // 设置音频源为固定值
	AmpBuilder & freqSrc(double src); // 设置频率源为固定值
	AmpBuilder & freqSrc(u_sp<PhaseSrc> src); // 设置频率源为PhaseSrc对象
	AmpBuilder & cc(const yzrilyzr_array::IntArray & cc); // 设置控制变化(CC)数组
	AmpBuilder & noteShift(int8_t shift); // 音符移调
	AmpBuilder & multyKey(const yzrilyzr_array::IntArray & noteShift,
						  const yzrilyzr_array::DoubleArray & velocityMul); // 一个note可以合成多个Note

	// 振幅调整
	AmpBuilder & add(double add); // 叠加固定值（取最长时长）
	AmpBuilder & add(NoteProcPtr add); // 叠加另一个音频源（取最长时长）
	AmpBuilder & addMul(NoteProcPtr src1, double mul); // 叠加另一个音频源的倍数（取最短时长）
	AmpBuilder & mul(double mul); // 乘以固定值（取最短时长）
	AmpBuilder & mul(NoteProcPtr mul); // 乘以另一个音频源（取最短时长）
	AmpBuilder & velMix(s_note_vel ovrd, u_normal_01 mix); // 混合原始力度和指定力度
	AmpBuilder & velMix(u_normal_01 mix); // 混合原始力度和指定力度

	// 包络控制
	AmpBuilder & AR(u_time_ms attackTime, u_time_ms releaseTime,
					u_sp<yzrilyzr_interpolator::Interpolator> aCurve,
					u_sp<yzrilyzr_interpolator::Interpolator> rCurve); // AR包络
	AmpBuilder & AR(u_time_ms attackTime, u_time_ms releaseTime, u_time_ms forceReleaseTime,
					u_sp<yzrilyzr_interpolator::Interpolator> aCurve,
					u_sp<yzrilyzr_interpolator::Interpolator> rCurve); // AR包络
	AmpBuilder & ADSR(u_time_ms attackTime, u_time_ms decayTime, u_normal_01 decayToVolume,
					  bool canSustain, u_time_ms releaseTime,
					  u_sp<yzrilyzr_interpolator::Interpolator> aCurve,
					  u_sp<yzrilyzr_interpolator::Interpolator> dCurve,
					  u_sp<yzrilyzr_interpolator::Interpolator> rCurve); // ADSR包络
	AmpBuilder & ADSR(u_time_ms attackTime, u_time_ms decayTime, u_normal_01 decayToVolume,
					  bool canSustain, u_time_ms releaseTime, u_time_ms forceReleaseTime,
					  u_sp<yzrilyzr_interpolator::Interpolator> aCurve,
					  u_sp<yzrilyzr_interpolator::Interpolator> dCurve,
					  u_sp<yzrilyzr_interpolator::Interpolator> rCurve); // 带强制释放的ADSR包络
	AmpBuilder & AHDSR(u_time_ms attackTime, u_time_ms holdTime, u_time_ms decayTime,
					   u_normal_01 decayToVolume, bool canSustain, u_time_ms releaseTime,
					   u_sp<yzrilyzr_interpolator::Interpolator> aCurve,
					   u_sp<yzrilyzr_interpolator::Interpolator> dCurve,
					   u_sp<yzrilyzr_interpolator::Interpolator> rCurve); // AHDSR包络
	AmpBuilder & AHDSR(u_time_ms attackTime, u_time_ms holdTime, u_time_ms decayTime,
					   u_normal_01 decayToVolume, bool canSustain, u_time_ms releaseTime,
					   u_time_ms forceReleaseTime,
					   u_sp<yzrilyzr_interpolator::Interpolator> aCurve,
					   u_sp<yzrilyzr_interpolator::Interpolator> dCurve,
					   u_sp<yzrilyzr_interpolator::Interpolator> rCurve); // 带强制释放的AHDSR包络
	AmpBuilder & DAHDSR(u_time_ms delayTime, u_time_ms attackTime, u_time_ms holdTime,
						u_time_ms decayTime, u_normal_01 decayToVolume, bool canSustain,
						u_time_ms releaseTime,
						u_sp<yzrilyzr_interpolator::Interpolator> aCurve,
						u_sp<yzrilyzr_interpolator::Interpolator> dCurve,
						u_sp<yzrilyzr_interpolator::Interpolator> rCurve); // DAHDSR包络
	AmpBuilder & DAHDSR(u_time_ms delayTime, u_time_ms attackTime, u_time_ms holdTime,
						u_time_ms decayTime, u_normal_01 decayToVolume, bool canSustain,
						u_time_ms releaseTime, u_time_ms forceReleaseTime,
						u_sp<yzrilyzr_interpolator::Interpolator> aCurve,
						u_sp<yzrilyzr_interpolator::Interpolator> dCurve,
						u_sp<yzrilyzr_interpolator::Interpolator> rCurve); // 带强制释放的DAHDSR包络
	AmpBuilder & GraphEnv(int32_t sustainPointIndex, const yzrilyzr_array::DoubleArray & pointValues); // 图形包络
	AmpBuilder & MultiStageEnv(const std::vector<MSEPoint> &points);


	// 调制效果
	AmpBuilder & am(NoteProcPtr amSrc, double amp, u_freq Hz); // 振幅调制(AM)
	AmpBuilder & rm(NoteProcPtr amSrc, double amp, u_freq Hz); // 环形调制(RM)
	AmpBuilder & pm(NoteProcPtr pmSrc, double amp, double noteRatio); // 相位调制(PM)
	AmpBuilder & pm(NoteProcPtr pmSrc, double amp); // 相位调制(PM)
	AmpBuilder & lpm(NoteProcPtr pmSrc, double amp, u_freq lpmHz); // 低频相位调制
	AmpBuilder & lpm(NoteProcPtr lpmSrc, double amp); // 相位调制(PM)
	AmpBuilder & autoMod(u_normal_01 modFreqDepth, u_normal_01 modAmpDepth, u_freq modRate,
						 u_time modDelay, NoteProcPtr modShape=nullptr); // 自动调制
	AmpBuilder & detune(int32_t count, s_note_id offset); // 失谐效果（一次合成多个note）

	// 滤波处理
	AmpBuilder & fir(u_sample_rate sampleRate, uint16_t n, const yzrilyzr_array::DoubleArray & Wn); // FIR滤波
	AmpBuilder & iir(yzrilyzr_dsp::ButterworthParams & params); // 通用IIR滤波
	AmpBuilder & biquad(yzrilyzr_dsp::RBJParams & params); // 双二阶滤波
	AmpBuilder & biquadEnv(NoteProcPtr freqEnv, NoteProcPtr qEnv, yzrilyzr_dsp::FilterPassType type); // 包络控制双二阶滤波，freqEnv应返回0-127（noteid）
	AmpBuilder & biquadEnvID(s_note_id idOffset, double q, yzrilyzr_dsp::FilterPassType type); // 根据音符ID的双二阶滤波
	AmpBuilder & biquadEnvVel(s_note_id idMin, s_note_id idMax, double q, yzrilyzr_dsp::FilterPassType type); // 根据音符力度的双二阶滤波
	AmpBuilder & noteDSP(yzrilyzr_dsp::DSPPtr dsp); // 每个音符的DSP
	AmpBuilder & postDSP(yzrilyzr_dsp::DSPPtr dsp); // 音符合并混音后的DSP(通道DSP)
	AmpBuilder & biquadEnvGroup(int type, std::vector<BiquadEnvFilterGroupConfig> filters);//包络滤波器组

	// 失真效果
	AmpBuilder & clamp(u_sample clamp); // 硬截幅失真
	AmpBuilder & clamp(u_sample inputGain, u_sample clamp); // 硬截幅失真
	AmpBuilder & clamp(u_sample inputGain, u_sample clamp, u_sample outputGain);
	AmpBuilder & clampV(u_sample inputGain, u_sample clamp); // 硬截幅到力度的失真
	AmpBuilder & clampV(u_sample inputGain, u_sample clamp, u_sample outputGain); // 硬截幅到力度的失真
	AmpBuilder & arctanDistortion(u_sample inputGain, double alpha, u_sample outputGain); // 反正切失真
	AmpBuilder & quantization(int32_t q); // 量化失真

	// 特殊效果
	AmpBuilder & ks(u_normal_01 alpha); // Karplus-Strong合成器
	AmpBuilder & ks(u_freq freq, u_normal_01 alpha); // 固定频率的Karplus-Strong合成器
	AmpBuilder & ks(u_sp<PhaseSrc> freqSrc, u_normal_01 alpha); // 频率源控制的Karplus-Strong合成器
	AmpBuilder & drum(u_freq startFreq, u_freq endFreq, u_time duration); // 鼓点合成
	AmpBuilder & drum(u_freq startFreq, u_freq endFreq, u_time duration, int mode,
					  u_sp<yzrilyzr_interpolator::Interpolator> interpolator); // 高级鼓点合成
	AmpBuilder & mean(NoteProcPtr pEnv, double pMul); // 均值处理效果

	// 工具方法
	NoteProcPtr build(); // 构建最终的音频处理器
	NoteProcPtr cali(int samples); // 校准处理
	NoteProcPtr parse(const yzrilyzr_lang::String & str); // 解析字符串配置
	};
}
