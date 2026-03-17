#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "Enveloper.h"
#include "events/NoteData.hpp"
#include "array/Array.hpp"
#include "array/Array.hpp"

namespace yzrilyzr_simplesynth{
	/**
	* 0. 延音和循环都不启用：一直往后走到点结束，结束后返回最后一个点的包络值，且noMoreData true
	*
	* 1. 延音点开启：一直往后走到达延音点，直至noteoff保持延音点这个值。noteoff后继续往后走到点结束，结束后返回最后一个点的包络值，且noMoreData true。
	*
	* 2. 循环开启：一直往后走到达循环结束点，跳回循环开始点，在区间无限循环（忽略循环后的点），noMoreData始终返回false。
	*
	* 3. 两个都开启，noMoreData始终返回false:
	* 3.1 延音点在循环开始之前：一直往后走到达延音点，直至noteoff保持延音点这个值。noteoff后继续往后，走到循环区间无限循环
	* 3.2 延音点在循环内：一直往后走到达延音点，直至noteoff保持延音点这个值。off后继续往后，走到循环结束点，跳回循环开始点，在循环区间无限循环
	* 3.3 延音点在循环结束之后：忽略延音点配置
	*/
	struct GraphPoint{
		u_time time; // 时间，以秒为单位
		u_normal_01 env;  // 包络值，范围为0到1
		GraphPoint(u_time time, u_normal_01 env){
			this->time=time;
			this->env=env;
		}
	};
	EBCLASS(GraphEnvelopKeyData){
		public:
		bool sustainHolding=false;   // 是否正在延音保持
		bool finished=false;         // 包络是否已结束（仅用于无循环情况）
		u_time releaseOffset=0;   // 从延音点开始的释放偏移量
	};
	ECLASS(GraphEnvelop, public Enveloper, NoteData<GraphEnvelopKeyData>){
		public:
		std::vector<GraphPoint> points;
		int32_t sustainPointIndex=-1;
		int32_t loopStartPointIndex=-1;
		int32_t loopEndPointIndex=-1;
		GraphEnvelop()=default;
		GraphEnvelop(int32_t sustainPointIndex, const yzrilyzr_array::DoubleArray & pointValues);
		GraphEnvelop(int32_t sustainPointIndex, int32_t loopStartIndex, int32_t loopEndIndex, const yzrilyzr_array::DoubleArray & pointValues);
		GraphEnvelop(int32_t sustainPointIndex, const std::vector<GraphPoint> &points);
		GraphEnvelop(int32_t sustainPointIndex, int32_t loopStartIndex, int32_t loopEndIndex, const std::vector<GraphPoint> &points);
		bool noMoreData(const Note & note) override;
		u_sample getAmp(const Note & note) override;
		void init(ChannelConfig & cfg)override;
		GraphEnvelopKeyData * init(GraphEnvelopKeyData * data, const Note & note) override;
		u_sp <NoteProcessor> clone() override{
			return mksp<GraphEnvelop>(sustainPointIndex, loopStartPointIndex, loopEndPointIndex, points);
		}
		yzrilyzr_lang::String toString()const override;
		U_CLASS_INFO_B(GraphEnvelop, Enveloper);

	};
}