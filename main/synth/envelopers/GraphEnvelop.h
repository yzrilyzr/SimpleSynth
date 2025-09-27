#pragma once
#include "SimpleSynth.h"
#include "interface/NoteProcessor.h"
#include "Enveloper.h"
#include "events/NoteData.hpp"
#include "array/ObjectArray.hpp"
#include "array/DoubleArray.h"

namespace yzrilyzr_simplesynth{
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
	bool isSustain=false;
	bool isLooping=false;
	u_time releaseTime=-1;
	u_time loopEnterTime=-1;
	GraphPoint * sustainPoint=nullptr;
	};
	ECLASS(GraphEnvelop, public NoteProcessor, public Enveloper, NoteData<GraphEnvelopKeyData>){
	private:
	std::vector<GraphPoint> points;
	int32_t sustainPointIndex=-1;
	int32_t loopStartPointIndex=-1;
	int32_t loopEndPointIndex=-1;
	public:
	GraphEnvelop(){
		//registerParam("InputGain", ParamType::Double, &inputGain, &gainMin, &gainMax);
	}
	GraphEnvelop(int32_t sustainPointIndex, const yzrilyzr_array::DoubleArray * pointValues);
	GraphEnvelop(int32_t sustainPointIndex, int32_t loopStartIndex, int32_t loopEndIndex, const yzrilyzr_array::DoubleArray * pointValues);
	GraphEnvelop(int32_t sustainPointIndex, const std::vector<GraphPoint> &points);
	GraphEnvelop(int32_t sustainPointIndex, int32_t loopStartIndex, int32_t loopEndIndex,const std::vector<GraphPoint> &points);
	bool noMoreData(Note & note) override;
	u_sample getAmp(Note & note) override;
	GraphEnvelopKeyData * init(GraphEnvelopKeyData * data, Note & note) override;
	std::shared_ptr <NoteProcessor> clone() override{
		return std::make_shared<GraphEnvelop>(sustainPointIndex, loopStartPointIndex, loopEndPointIndex, points);
	}
	std::string toString()const override;
	};
}