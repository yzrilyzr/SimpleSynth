#pragma once
#include "synth/generators/Osc.h"
#include "events/Note.h"
#include "array/DoubleArray.h"
#include "array/ObjectArray.hpp"
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
	std::shared_ptr<yzrilyzr_array::ObjectArray<yzrilyzr_array::DoubleArray *>> aa;
	yzrilyzr_interpolator::Interpolator * interpolator;
	public:
	~SineHarmonicWaveTable();
	SineHarmonicWaveTable(std::shared_ptr<yzrilyzr_array::ObjectArray<yzrilyzr_array::DoubleArray *>> aa);
	SineHarmonicWaveTable(std::shared_ptr<PhaseSrc> freq, std::shared_ptr<yzrilyzr_array::ObjectArray<yzrilyzr_array::DoubleArray *>> aa);
	/**
	 * 分贝转倍数
	 * line[i]为分贝数（相对于1）
	 */
	static yzrilyzr_array::DoubleArray * dBToAmp(double gain, yzrilyzr_array::DoubleArray * line);
	u_sample getAmp(Note & note) override;
	private:
	u_sample a(Note & note, double x);
	double getInterpolation(Note & note, yzrilyzr_array::DoubleArray & ampLine);
	std::string toString() const override{
		return yzrilyzr_lang::StringFormat::format("SineHarmonicWaveTable(%s)", getPhaseSource());
	}
	};
}