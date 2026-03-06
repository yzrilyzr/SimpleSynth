#include "HardSync.h"
#include <cmath>
#include "dsp/RingBuffer.h"
#include "lang/StringFormat.hpp"

using namespace yzrilyzr_lang;
using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
namespace yzrilyzr_simplesynth{
	HardSync::HardSync() : AmpUnaryComposition(nullptr){
		static float min=0.1f, max=10.0f;
		registerParam("SlaveFreqRatio", ParamType::Float, &slaveFreqRatio, &min, &max);
	}
	HardSync::HardSync(NoteProcPtr slave, float slaveFreqRatio) : AmpUnaryComposition(slave), slaveFreqRatio(slaveFreqRatio){}

	u_sample HardSync::getAmp(const Note & note){
		HardSyncKeyData * data=getData(note);
		// 归一化主振荡器相位到[0,1)范围
		float masterPhase=RingBufferUtil::mod1(note.phaseSynth);
		// 检测主振荡器周期结束：相位发生回绕（当前值 < 上一帧值）
		bool isCycleEnd=(masterPhase < data->lastMasterPhase);
		data->lastMasterPhase=masterPhase;
		// 计算从振荡器相位
		data->phaseSynth+=note.freqSynth * slaveFreqRatio * note.cfg->deltaTime;
		// 硬同步触发：重置从振荡器相位
		if(isCycleEnd){
			data->phaseSynth=0.0f;
		}
		// 临时保存原始相位并替换
		float originalPhase=note.phaseSynth;
		auto & mut_note=const_cast<Note &>(note);
		mut_note.phaseSynth=data->phaseSynth;
		// 获取从振荡器输出
		u_sample output=a->getAmp(mut_note);
		// 恢复原始相位
		mut_note.phaseSynth=originalPhase;
		return output;
	}
	NoteProcPtr HardSync::clone(){
		return mksp<HardSync>(a->clone(), slaveFreqRatio);
	}

	HardSyncKeyData * HardSync::init(HardSyncKeyData * data, const Note & note){
		if(data == nullptr){
			data=new HardSyncKeyData();
		}
		data->lastMasterPhase=0.0;
		data->phaseSynth=0.0;
		return data;
	}
	String HardSync::toString() const{
		return StringFormat::object2string("HardSync", a, slaveFreqRatio);
	}
}