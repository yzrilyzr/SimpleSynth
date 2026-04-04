#include "GraphEnvelop.h"
#include "events/Note.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_util;
using namespace yzrilyzr_array;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	GraphEnvelop::GraphEnvelop(int32_t sustainPointIndex, const DoubleArray & pointValues) :GraphEnvelop(sustainPointIndex, -1, -1, pointValues){}
	GraphEnvelop::GraphEnvelop(int32_t sustainPointIndex, int32_t loopStartIndex, int32_t loopEndIndex, const DoubleArray & pointValues) :
		sustainPointIndex(sustainPointIndex),
		loopStartPointIndex(loopStartIndex),
		loopEndPointIndex(loopEndIndex){
		this->points.reserve(pointValues.length / 2);
		for(uint32_t i=0, j=0;i < pointValues.length;i+=2, j++){
			this->points.emplace_back(pointValues[i] / 1000.0, pointValues[i + 1]);
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
	bool GraphEnvelop::noMoreData(const Note & note)const{
		if(note.fclosed(*note.cfg))return true;
		GraphEnvelopKeyData * n1=getDataConst(note);
		return n1->finished;
	}
	void GraphEnvelop::init(ChannelConfig & cfg){
		if(sustainPointIndex != -1){
			if(sustainPointIndex < 0 || sustainPointIndex >= points.size()){
				throw IllegalArgumentException("Sustain range invalid");
			}
		}
		if(loopStartPointIndex != -1 || loopEndPointIndex != -1){
			if(loopEndPointIndex < loopStartPointIndex){
				throw IllegalArgumentException("Loop index invalid");
			}
			if(loopStartPointIndex < 0 || loopStartPointIndex >= points.size()){
				throw IllegalArgumentException("LoopStart range invalid");
			}
			if(loopEndPointIndex < 0 || loopEndPointIndex >= points.size()){
				throw IllegalArgumentException("LoopEnd range invalid");
			}
		}
		if(sustainPointIndex != -1 && (loopStartPointIndex != -1 || loopEndPointIndex != -1)){
			if(sustainPointIndex >= loopStartPointIndex || sustainPointIndex >= loopEndPointIndex){
				throw IllegalArgumentException("Sustain index invalid");
			}
		}
	}
	u_sample GraphEnvelop::getAmp(const Note & note){
		u_time t=note.passedTime;
		GraphEnvelopKeyData & n1=*getData(note);
		bool noteClosed=note.closed(*note.cfg);

		// 1. 处理延音保持状态
		if(n1.sustainHolding){
			if(!noteClosed){
				// 音符未关闭，继续保持在延音点
				return points[sustainPointIndex].env;
			} else{
				// 音符已关闭，退出延音保持，记录释放偏移量
				n1.sustainHolding=false;
				n1.releaseOffset=t - points[sustainPointIndex].time;
				// 继续执行，后续使用有效时间计算
			}
		}

		// 计算有效时间：若有释放偏移，则减去它以从延音点开始连续
		u_time effectiveTime=t - n1.releaseOffset;

		// 2. 检查是否应进入延音保持（仅在音符未关闭且未释放过时）
		if(!noteClosed && sustainPointIndex >= 0 && effectiveTime >= points[sustainPointIndex].time && !n1.sustainHolding && n1.releaseOffset == 0){
			n1.sustainHolding=true;
			return points[sustainPointIndex].env;
		}

		// 3. 判断是否启用循环
		bool loopEnabled=(loopStartPointIndex >= 0 && loopEndPointIndex >= 0 &&
						  loopStartPointIndex < loopEndPointIndex);

	  // 4. 如果循环启用且有效时间已进入循环区间，则进入循环模式
		if(loopEnabled && effectiveTime >= points[loopStartPointIndex].time){
			u_time loopStart=points[loopStartPointIndex].time;
			u_time loopEnd=points[loopEndPointIndex].time;
			u_time period=loopEnd - loopStart;
			u_time loopTime;

			if(period > 0){
				u_time offset=fmod(effectiveTime - loopStart, period);
				loopTime=loopStart + offset;
			} else{
				// 周期为0，始终停在循环起点
				loopTime=loopStart;
			}

			// 在循环区间内查找对应的线段
			for(int i=loopStartPointIndex; i < loopEndPointIndex; ++i){
				const GraphPoint & start=points[i];
				const GraphPoint & end=points[i + 1];
				if(loopTime >= start.time && loopTime < end.time){
					u_time segDur=end.time - start.time;
					u_sample segProgress=(loopTime - start.time) / segDur;
					return start.env + (end.env - start.env) * segProgress;
				}
			}
			// 如果 loopTime 恰好等于 loopEnd（理论上不会），则返回起点值
			return points[loopStartPointIndex].env;
		}

		// 5. 普通模式（无循环或有效时间未到循环起点）
		for(uint32_t i=0; i < points.size() - 1; ++i){
			const GraphPoint & start=points[i];
			const GraphPoint & end=points[i + 1];
			if(effectiveTime >= start.time && effectiveTime < end.time){
				u_time segDur=end.time - start.time;
				u_sample segProgress=(effectiveTime - start.time) / segDur;
				return start.env + (end.env - start.env) * segProgress;
			}
		}

		// 6. 超出最后一个点
		if(effectiveTime >= points.back().time){
			if(!loopEnabled){
				n1.finished=true;   // 仅当无循环时标记结束
			}
			return points.back().env;
		}

		// 理论上不会执行到这里，但保险起见返回最后一个点值
		return points.back().env;
	}
	GraphEnvelopKeyData * GraphEnvelop::init(GraphEnvelopKeyData * data, const Note & note){
		if(data == nullptr) data=new GraphEnvelopKeyData();
		data->sustainHolding=false;
		data->finished=false;
		data->releaseOffset=0;
		return data;
	}
	String GraphEnvelop::toString()const{
		return StringFormat::object2string("GraphEnvelop", sustainPointIndex, loopStartPointIndex, loopEndPointIndex, points);
	}
}