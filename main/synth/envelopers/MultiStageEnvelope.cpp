#include "MultiStageEnvelope.h"
#include "EnvUtil.h"
#include "events/ChannelConfig.h"
#include "events/ChannelEvent.h"
#include "events/Note.h"
#include "events/PNData.h"
#include "interface/NoteProcessor.h"
#include "interpolator/Interpolator.h"
#include "lang/StringFormat.hpp"
#include "util/Util.h"
#include "yzrutil.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <util/Flag.h>
#include <interpolator/InterpolateFunction.h>
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;

namespace yzrilyzr_simplesynth{
	MultiStageEnvelope::MultiStageEnvelope(const std::vector<MSEPoint> & points) : points(points){}

	MultiStageEnvelope::MultiStageEnvelope(){}
	void MultiStageEnvelope::init(ChannelConfig & cfg){
		u_index loopStartCount=0;
		u_index DecayCount=0;
		u_index SustainCount=0;
		decayPoint=nullptr;
		loopPoint=nullptr;
		sustainPoint=nullptr;
		if(points.size() < 2)throw IllegalStateException("points.size() < 2");
		u_index index=0;
		for(auto & p : points){
			bool isDecay=Flag::hasFlag(static_cast<int>(p.type), static_cast<int>(MSEPointType::DECAY));
			bool isLoop=Flag::hasFlag(static_cast<int>(p.type), static_cast<int>(MSEPointType::LOOP_START));
			bool isSustain=Flag::hasFlag(static_cast<int>(p.type), static_cast<int>(MSEPointType::SUSTAIN_OR_LOOP_END));
			if(isSustain && (isLoop || isDecay))throw IllegalStateException("Illegal Point Type");
			if(isDecay){
				decayPoint=&p;
				DecayCount++;
			}
			if(isLoop){
				loopStartCount++;
				loopPoint=&p;
			}
			if(isSustain){
				SustainCount++;
				sustainPoint=&p;
			}
			p.index=index;
			index++;
		}
		if(loopStartCount > 1)throw IllegalStateException("loopStartCount > 1");
		if(DecayCount > 1)throw IllegalStateException("DecayCount > 1");
		if(SustainCount > 1)throw IllegalStateException("SustainCount > 1");
		if(loopPoint != nullptr && sustainPoint == nullptr)sustainPoint=&points[points.size() - 1];
		if(loopPoint != nullptr && sustainPoint != nullptr){
			sustainPoint->y=loopPoint->y;
		}
	}

	bool MultiStageEnvelope::noMoreData(const Note & note){
		MultiStageEnvelopeKeyData & data=*getData(note);
		return data.end == nullptr;
	}

	NoteProcPtr MultiStageEnvelope::clone(){
		auto cloned=mksp<MultiStageEnvelope>(points);
		return cloned;
	}

	u_sample MultiStageEnvelope::getAmp(const Note & note){
		u_time currentTime=note.passedTime;
		if(currentTime < 0) return 0;
		MultiStageEnvelopeKeyData & data=*getData(note);
		bool hasSustain=sustainPoint != nullptr;
		bool noteClosed=note.closed(*note.cfg);
		//判断状态
		if(!data.isRelease && noteClosed && hasSustain){
			data.isRelease=true;
			data.end=sustainPoint;
			data.releaseTime=note.passedTime;
		}
		if(data.isRelease && hasSustain){
			u_time t=sustainPoint->x + note.passedTime - data.releaseTime;
			calcNextPoint(data, t);
			if(data.end == nullptr)return 0;
			u_sample susY=sustainPoint->y;
			u_sample env=data.currentVol * calcEnv(*data.start, *data.end, t) / susY;
		} else{
			//判断当前在哪两个点之间
			if(hasSustain && !noteClosed && data.start == sustainPoint){
				data.currentVol=sustainPoint->y;
				return data.currentVol;
			}
			if(!data.isLoop && loopPoint != nullptr && hasSustain && note.passedTime >= sustainPoint->x){
				data.isLoop=true;
			}
			u_time t=note.passedTime;
			if(data.isLoop){
				t=calcLoopTime(t);
			}
			calcNextPoint(data, t);
			data.lastTime=t;
			if(data.end == nullptr)return data.currentVol;
			if(Flag::hasFlag(static_cast<int>(data.start->type), static_cast<int>(MSEPointType::DECAY))){
				data.isDecay=true;
			}
			if(data.end == nullptr){
				return data.currentVol;
			}
			data.currentVol=calcEnv(*data.start, *data.end, t);
			return data.currentVol;
		}
		return 0;
	}

	String MultiStageEnvelope::toString() const{
		return StringFormat::object2string("MultiStageEnvelope", points);
	}

	MultiStageEnvelopeKeyData * MultiStageEnvelope::init(MultiStageEnvelopeKeyData * data, const Note & note){
		if(data == nullptr) data=new MultiStageEnvelopeKeyData();
		data->start=&points[0];
		data->end=&points[1];
		data->currentVol=0;
		data->isDecay=false;
		data->isLoop=false;
		data->isRelease=false;
		data->lastTime=0;
		data->releaseTime=0;
		return data;
	}
	u_time MultiStageEnvelope::calcLoopTime(u_time curTime){
		u_time start=loopPoint->x;
		u_time end=sustainPoint->x;
		u_time t1=curTime - start;
		u_time loopTime=end - start;
		u_time modTime=std::fmod(t1, loopTime);
		return start + modTime;
	}

	void MultiStageEnvelope::calcNextPoint(MultiStageEnvelopeKeyData & data, u_time curTime){
		if(data.end == nullptr)return;
		if(data.isLoop && curTime < data.lastTime){
			data.start=loopPoint;
		} else if(curTime >= data.end->x){
			data.start=data.end;
		}
		if(data.start->index + 1 >= points.size())data.end=nullptr;
		else data.end=&points[data.start->index + 1];
	}
	u_sample MultiStageEnvelope::calcEnv(MSEPoint & start, MSEPoint & end, float x){
		float a=end.modeValue;
		float x1=start.x;
		float x2=end.x;
		float y1=start.y;
		float y2=end.y;
		float dx=x2 - x1;
		float dy=y2 - y1;
		if(x2 == x1)return (y1 + y2) / 2.0f;
		float t=(x - x1) / dx;  // t ∈ [0, 1]
		switch(end.mode){
			case MSEPointMode::HOLD:
				return y1;
			case MSEPointMode::SMOOTH:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::smooth(t, a));
			case MSEPointMode::SINGLE_CURVE:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::pow01(t, a * 10));
			case MSEPointMode::DOUBLE_CURVE:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::doubleCurve(t, a * 10));
			case MSEPointMode::HALF_SINE:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::halfSine(t, a));
			case MSEPointMode::STAGE:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::stage(t, a));
			case MSEPointMode::SMOOTH_STAGE:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::stage(t, a));
			case MSEPointMode::PULSE:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::pulse(t, a));
			case MSEPointMode::WAVE:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::wave(t, a));
		}
		return t;
	}
} // namespace yzrilyzr_simplesynth