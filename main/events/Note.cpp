#include "Note.h"

namespace yzrilyzr_simplesynth{
	std::string Note::toString() const{
		return yzrilyzr_lang::StringFormat::format("[Note:%d Vel:%.2f]", id, velocitySynth);
	}
	void Note::set(Note & note){
			// 基础属性
		id=note.id;
		velocitySynth=note.velocitySynth;
		velocity=note.velocity;

		// 合成参数
		phaseSynth=note.phaseSynth;
		freqSynth=note.freqSynth;
		idSynth=note.idSynth;

		// 时间状态
		startAtTime=note.startAtTime;
		passedTime=note.passedTime;
		closedAtTime=note.closedAtTime;
		closedPassedTime=note.closedPassedTime;

		// 滑音状态
		lastPortamentoID=note.lastPortamentoID;
		portamentoDeltaID=note.portamentoDeltaID;
		pitchBend=note.pitchBend;

		// 标志位
		closedAtTime=note.closedAtTime;
		forceCloseAtTime=note.forceCloseAtTime;
		//closed=note.closed;
		//fclosed=note.fclosed;
		dataInvalidated=note.dataInvalidated;
		noMoreData=note.noMoreData;

		// 关联对象
		cfg=note.cfg;
	}
}