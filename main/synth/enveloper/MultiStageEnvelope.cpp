#include "MultiStageEnvelope.h"
#include "events/ChannelConfig.h"
#include "events/ChannelEvent.h"
#include "events/Note.h"
#include "events/PNData.h"
#include "interface/NoteProcessor.h"
#include "interpolator/InterpolateFunction.h"
#include "interpolator/Interpolator.h"
#include "lang/StringFormat.hpp"
#include "synth/util/EnvUtil.h"
#include "util/Flag.h"
#include "util/Util.h"
#include "yzrutil.h"
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
		loopStartPoint=nullptr;
		sustainOrLoopEndPoint=nullptr;
		if(points.size() < 2)throw IllegalStateException("points.size() < 2");
		u_index index=0;
		for(auto & p : points){
			bool isDecay=Flag::hasFlag(static_cast<int>(p.type), static_cast<int>(MSEPointType::DECAY));
			bool isLoop=Flag::hasFlag(static_cast<int>(p.type), static_cast<int>(MSEPointType::LOOP_START));
			bool isSustain=Flag::hasFlag(static_cast<int>(p.type), static_cast<int>(MSEPointType::SUSTAIN_OR_LOOP_END));
			if(isSustain && isLoop)throw IllegalStateException("Illegal Point Type");
			if(isDecay){
				DecayCount++;
			}
			if(isLoop){
				loopStartCount++;
				loopStartPoint=&p;
			}
			if(isSustain){
				SustainCount++;
				sustainOrLoopEndPoint=&p;
			}
			p.index=index;
			index++;
		}
		if(loopStartCount > 1)throw IllegalStateException("loopStartCount > 1");
		if(DecayCount > 1)throw IllegalStateException("DecayCount > 1");
		if(SustainCount > 1)throw IllegalStateException("SustainCount > 1");
		if(loopStartPoint != nullptr && sustainOrLoopEndPoint == nullptr){
			sustainOrLoopEndPoint=&points[points.size() - 1];
		}
		if(loopStartPoint != nullptr && sustainOrLoopEndPoint != nullptr){
			sustainOrLoopEndPoint->y=loopStartPoint->y;
		}
	}

	bool MultiStageEnvelope::noMoreData(const Note & note)const{
		MultiStageEnvelopeKeyData & data=*getDataConst(note);
		// 包络结束条件：已无下一个点 且 音符已关闭
		return data.end == nullptr || note.fclosed(*note.cfg);
	}

	NoteProcPtr MultiStageEnvelope::clone(){
		auto cloned=mksp<MultiStageEnvelope>(points);
		return cloned;
	}

	u_sample MultiStageEnvelope::getAmp(const Note & note){
		u_time currentTime=note.passedTime;
		if(currentTime < 0) return 0;
		MultiStageEnvelopeKeyData & data=*getData(note);
		bool hasLoopSection=loopStartPoint != nullptr && sustainOrLoopEndPoint != nullptr;
		bool noteClosed=note.closed(*note.cfg);
		//走完
		if(data.end == nullptr){
			if(noteClosed)return 0;
			return data.start->y;
		}
		if(hasLoopSection){
			//有循环时关闭
			if(noteClosed){
				if(!data.isClosed){
					data.start=sustainOrLoopEndPoint;
					data.end=&points[sustainOrLoopEndPoint->index + 1];
					data.loopEndVol=data.currentVol;
					data.isClosed=true;
				}
				float x=note.closedPassedTime + sustainOrLoopEndPoint->x;
				if(data.start == sustainOrLoopEndPoint){
					data.currentVol=calcEnv(sustainOrLoopEndPoint->x, data.loopEndVol, *data.end, x);
				} else{
					data.currentVol=calcEnv(*data.start, *data.end, x);
				}
				calcNextPoint(data, x);
			} else{
				float x=note.passedTime - data.loopCount * (sustainOrLoopEndPoint->x - loopStartPoint->x);
				data.currentVol=calcEnv(*data.start, *data.end, x);
				calcNextPoint(data, x);
				if(data.start == sustainOrLoopEndPoint){
					data.loopCount++;
					data.start=loopStartPoint;
					data.end=&points[loopStartPoint->index + 1];
				}
			}
		} else{
			data.currentVol=calcEnv(*data.start, *data.end, note.passedTime);
			calcNextPoint(data, note.passedTime);
		}
		return data.currentVol;
	}

	String MultiStageEnvelope::toString() const{
		return StringFormat::object2string("MultiStageEnvelope", points);
	}

	MultiStageEnvelopeKeyData * MultiStageEnvelope::init(MultiStageEnvelopeKeyData * data, const Note & note){
		if(data == nullptr) data=new MultiStageEnvelopeKeyData();
		data->start=&points[0];
		data->end=&points[1];
		data->currentVol=0;
		data->isClosed=false;
		data->loopCount=0;
		data->loopEndVol=0;
		return data;
	}

	void MultiStageEnvelope::calcNextPoint(MultiStageEnvelopeKeyData & data, float x){
		if(data.end == nullptr)return;
		if(x >= data.end->x){
			data.start=data.end;
			if(data.start->index + 1 >= points.size()){
				data.end=nullptr;
			} else{
				data.end=&points[data.start->index + 1];
			}
		}
	}
	u_sample MultiStageEnvelope::calcEnv(MSEPoint & start, MSEPoint & end, float x){
		return calcEnv(start.x, start.y, end.x, end.y, end.mode, end.modeValue, x);
	}
	u_sample MultiStageEnvelope::calcEnv(float xs, float ys, MSEPoint & end, float x){
		return calcEnv(xs, ys, end.x, end.y, end.mode, end.modeValue, x);
	}
	u_sample MultiStageEnvelope::calcEnv(float x1, float y1, float x2, float y2, MSEPointMode mode, float modeValue, float x){
		float dx=x2 - x1;
		float dy=y2 - y1;
		if(x2 == x1)return (y1 + y2) / 2.0f;
		float t=(x - x1) / dx;  // t ∈ [0, 1]
		switch(mode){
			case MSEPointMode::HOLD:
				return y1;
			case MSEPointMode::SMOOTH:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::smooth(t, modeValue));
			case MSEPointMode::SINGLE_CURVE:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::pow01(t, modeValue * 10));
			case MSEPointMode::DOUBLE_CURVE:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::doubleCurve(t, modeValue * 10));
			case MSEPointMode::HALF_SINE:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::halfSine(t, modeValue));
			case MSEPointMode::STAGE:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::stage(t, modeValue));
			case MSEPointMode::SMOOTH_STAGE:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::stage(t, modeValue));
			case MSEPointMode::PULSE:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::pulse(t, modeValue));
			case MSEPointMode::WAVE:
				return InterpolateFunction::linear(y1, y2, InterpolateFunction::wave(t, modeValue));
		}
		return t;
	}
} // namespace yzrilyzr_simplesynth