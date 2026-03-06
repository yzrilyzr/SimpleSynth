#pragma once
#include "synth/generators/Osc.h"
#include "events/Note.h"
#include "array/Array.hpp"
#include "array/Array.hpp"
#include "interpolator/Interpolator.h"

namespace yzrilyzr_simplesynth{
	/**
	 * n次谐波表
	 * [谐波次数][谐波振幅vs频率曲线(插值点)]
	 * 谐波振幅vs频率曲线(插值点)：
	 * length==1, index 0为该谐波的振幅
	 * length>=5, index 0为起始相位, 1起始频率, 2终止频率, 3~length-1为曲线(至少两个点)
	 */
	ECLASS(SineHarmonicWaveTable, public Osc){
	private:
	yzrilyzr_array::Array<yzrilyzr_array::DoubleArray> aa;
	u_sp < yzrilyzr_interpolator::Interpolator> interpolator;
	public:
	~SineHarmonicWaveTable();
	SineHarmonicWaveTable(const yzrilyzr_array::Array<yzrilyzr_array::DoubleArray> &aa);
	SineHarmonicWaveTable(u_sp<PhaseSrc> freq, const yzrilyzr_array::Array<yzrilyzr_array::DoubleArray> &aa);
	/**
	 * 分贝转倍数
	 * line[i]为分贝数（相对于1）
	 */
	static yzrilyzr_array::DoubleArray dBToAmp(double gain,const yzrilyzr_array::DoubleArray &line);
	u_sample getAmp(const Note & note) override;
	private:
	u_sample a(const Note & note, double x);
	double getInterpolation(const Note & note,const yzrilyzr_array::DoubleArray & ampLine);
	yzrilyzr_lang::String toString() const override{
		return yzrilyzr_lang::StringFormat::format("SineHarmonicWaveTable(%s)", getPhaseSource());
	}
	};
}