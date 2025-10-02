#include "GraphEnvelop.h"
#include "events/Note.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_util;
using namespace yzrilyzr_array;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	GraphEnvelop::GraphEnvelop(int32_t sustainPointIndex,const DoubleArray * pointValues) :GraphEnvelop(sustainPointIndex, -1, -1, pointValues){}
	GraphEnvelop::GraphEnvelop(int32_t sustainPointIndex, int32_t loopStartIndex, int32_t loopEndIndex, const DoubleArray * pointValues) :
		sustainPointIndex(sustainPointIndex),
		loopStartPointIndex(loopStartIndex),
		loopEndPointIndex(loopEndIndex){
		this->points.reserve(pointValues->length / 2);
		for(uint32_t i=0, j=0;i < pointValues->length;i+=2, j++){
			this->points.emplace_back((*pointValues)[i] / 1000.0, (*pointValues)[i + 1]);
		}
	}
	GraphEnvelop::GraphEnvelop(int32_t sustainPointIndex, const std::vector<GraphPoint> & points) :
		sustainPointIndex(sustainPointIndex),
		points(points){}
	GraphEnvelop::GraphEnvelop(int32_t sustainPointIndex, int32_t loopStartIndex, int32_t loopEndIndex, const std::vector<GraphPoint> & points) :
		sustainPointIndex(sustainPointIndex),
		loopStartPointIndex(loopStartIndex),
		loopEndPointIndex(loopEndIndex),
		points(points){}
	bool GraphEnvelop::noMoreData(Note & note){
		if(note.fclosed(*note.cfg))return true;
		u_time timePassed=note.passedTime;
		GraphEnvelopKeyData * n1=getData(note);
		if(n1->isSustain) return false;
		if(n1->isLooping) return false;
		if(n1->releaseTime != -1) timePassed-=n1->releaseTime;
		return timePassed >= points[points.size() - 1].time;
	}
	u_sample GraphEnvelop::getAmp(Note & note){
		u_time timePassed=note.passedTime;
		GraphEnvelopKeyData & n1=*getData(note);
		if(n1.isSustain){
			if(!note.closed(*note.cfg) && !note.fclosed(*note.cfg)){
				return n1.sustainPoint->env;
			} else if(n1.releaseTime == -1){
				n1.isSustain=false;
				n1.releaseTime=timePassed - n1.sustainPoint->time;
			}
		} else if(n1.isLooping){
			GraphPoint & loopStartPoint=points[loopStartPointIndex];
			GraphPoint & loopEndPoint=points[loopEndPointIndex];
			u_time loopPeriod=loopEndPoint.time - loopStartPoint.time;
			u_time loopTimeInPeriod=timePassed - n1.loopEnterTime;
			loopTimeInPeriod=loopTimeInPeriod - (int32_t)(loopTimeInPeriod / loopPeriod) * loopPeriod;
			//return loopTimeInPeriod / loopPeriod;
			for(int32_t i=loopStartPointIndex;i < loopEndPointIndex;i++){
				GraphPoint & startPoint=points[i];
				GraphPoint & endPoint=points[i + 1];
				if(loopTimeInPeriod >= startPoint.time - n1.loopEnterTime && loopTimeInPeriod < endPoint.time - n1.loopEnterTime){
					// 线性插值计算当前包络值
					u_time segmentDuration=endPoint.time - startPoint.time;
					u_time segmentProgress=(loopTimeInPeriod - (startPoint.time - n1.loopEnterTime)) / segmentDuration;
					u_sample en=startPoint.env + (endPoint.env - startPoint.env) * segmentProgress;
					return en;
				}
			}
		}
		if(n1.releaseTime != -1) timePassed-=n1.releaseTime;
		// 找到当前经过时间所在的两个点
		for(uint32_t i=0;i < points.size() - 1;i++){
			GraphPoint & startPoint=points[i];
			GraphPoint & endPoint=points[i + 1];
			if(timePassed >= startPoint.time && timePassed < endPoint.time){
				// 如果到了延音点且音符未关闭，则保持延音值
				if(i == sustainPointIndex && !note.closed(*note.cfg) && !note.fclosed(*note.cfg)){
					n1.isSustain=true;
					n1.sustainPoint=&startPoint;
					return startPoint.env;
				}
				if(i == loopStartPointIndex){
					n1.isLooping=true;
					n1.loopEnterTime=startPoint.time;
				}
				// 线性插值计算当前包络值
				u_time segmentDuration=endPoint.time - startPoint.time;
				u_time segmentProgress=(timePassed - startPoint.time) / segmentDuration;
				return startPoint.env + (endPoint.env - startPoint.env) * segmentProgress;
			}
		}
		// 如果超出了最后一个点并且音符已关闭，则进入释放阶段
		if(note.closed(*note.cfg) && timePassed >= points[points.size() - 1].time){
			return 0.0; // 释放阶段结束，返回0
		}
		return points[points.size() - 1].env; // 超出范围，返回最后一个点的值
	}
	GraphEnvelopKeyData * GraphEnvelop::init(GraphEnvelopKeyData * data, Note & note){
		if(data == nullptr) data=new GraphEnvelopKeyData();
		data->isSustain=false;
		data->isLooping=false;
		data->sustainPoint=nullptr;
		data->releaseTime=-1;
		data->loopEnterTime=-1;
		return data;
	}
	String GraphEnvelop::toString()const{
		return StringFormat::object2string("GraphEnvelop", sustainPointIndex, loopStartPointIndex, loopEndPointIndex, points);
	}
}