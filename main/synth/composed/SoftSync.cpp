#include "SoftSync.h"
#include "dsp/RingBuffer.h"
#include "interpolator/LineInterpolator.h"
#include "lang/StringFormat.hpp"
using namespace yzrilyzr_util;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;

namespace yzrilyzr_simplesynth{
	SoftSync::SoftSync() : AmpUnaryComposition(nullptr){
		static float min=0.1f, max=10.0f;
		registerParam("SlaveFreqRatio", ParamType::Float, &slaveFreqRatio, &min, &max);
		registerParamInterpolator("Interpolator", &syncInterp);
	}

	SoftSync::SoftSync(NoteProcPtr slave, float ratio, u_sp<Interpolator> interp=nullptr)
		: AmpUnaryComposition(slave), slaveFreqRatio(ratio){
		syncInterp=interp?interp:mksp<LineInterpolator>();
	}

	u_sample SoftSync::getAmp(Note & note){
		SoftSyncKeyData * data=getData(note);

		// 归一化主振荡器相位
		float masterPhase=RingBufferUtil::mod1(note.phaseSynth);
		bool isCycleEnd=(masterPhase < data->lastMasterPhase);
		data->lastMasterPhase=masterPhase;

		// 触发新同步周期
		if(isCycleEnd){
			data->syncStartPhase=data->slavePhase; // 记录当前相位作为插值起点
			data->syncProgress=0.0f;              // 重置进度
		}

		// 更新从振荡器相位（持续累积）
		data->slavePhase+=note.freqSynth * slaveFreqRatio * note.cfg->deltaTime;

		// 若处于同步过程中，应用插值平滑
		if(data->syncProgress < 1.0f){
			data->syncProgress+=static_cast<float>(note.cfg->deltaTime * note.freqSynth); // 进度随主频更新
			s_phase t=syncInterp->y(data->syncProgress);               // 插值曲线变换
			data->slavePhase=lerpPhase(data->syncStartPhase, 0.0f, t); // 平滑过渡到0
		}

		// 传递相位到从振荡器
		float originalPhase=note.phaseSynth;
		note.phaseSynth=RingBufferUtil::mod1(data->slavePhase);
		u_sample output=a->getAmp(note);
		note.phaseSynth=originalPhase;

		return output;
	}

	// 设置同步插值器（如Bezier、Pow等）
	void SoftSync::setSyncInterpolator(u_sp<Interpolator> interp){
		syncInterp=interp;
	}

	NoteProcPtr SoftSync::clone(){
		return mksp<SoftSync>(a->clone(), slaveFreqRatio, syncInterp);
	}

	SoftSyncKeyData * SoftSync::init(SoftSyncKeyData * data, Note & note){
		if(!data) data=new SoftSyncKeyData();
		data->lastMasterPhase=0.0f;
		data->slavePhase=0.0f;
		data->syncProgress=1.0f; // 初始无同步
		return data;
	}

	String SoftSync::toString() const{
		return StringFormat::object2string("SoftSync", a, slaveFreqRatio, syncInterp);
	}
}