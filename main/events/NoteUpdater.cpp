#include "Note.h"
#include "NoteUpdater.h"
#include "dsp/FastSin.h"
#include "events/ChannelConfig.h"
#include "interface/NoteTuning.h"
#include "interpolator/Interpolator.h"
#include "util/Util.h"

using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_interpolator;
namespace yzrilyzr_simplesynth{
	void NoteUpdater::preUpdateNote(Note & note, const ChannelConfig & cfg){
		// 计算最终音符ID(考虑各种调制效果)
		s_note_id finalID=note.id + static_cast<s_note_id>(cfg.NoteShift)
			+ static_cast<s_note_id>(cfg.ChannelPitchBend) * cfg.PitchBendRange
			+ cfg.CoarseTune + cfg.FineTune + note.pitchBend;

		// 调制效果计算
		double modValue=0;
		if(cfg.Modulation > 0 && note.passedTime > cfg.ModDelay){
			modValue=(double)cfg.Modulation * (double)cfg.ModDepth
				* fast_sin((note.passedTime - (double)cfg.ModDelay)*  Math::TAU * (double)cfg.ModRate, cfg.ModRate);
			finalID+=modValue;
		}

		// 滑音处理
		if(cfg.Portamento && cfg.PortamentoTime != 0){
			u_time x=Util::clamp01(note.passedTime / cfg.PortamentoTime);
			note.portamentoDeltaID=(x - 1) * (note.id - note.lastPortamentoID);
		}

		finalID+=note.portamentoDeltaID;

		// 更新音符状态
		note.passedTime=cfg.currentTime - note.startAtTime;
		note.closedPassedTime=cfg.currentTime - note.closedAtTime;
		note.freqSynth=cfg.tuning->getFrequencyByID(finalID);
		note.idSynth=finalID;
		note.velocitySynth=cfg.velocityMap->y(note.velocity) * static_cast<s_note_vel>(1 + 0.6 * modValue) * static_cast<s_note_vel>(cfg.Expression);
	}
	void NoteUpdater::postUpdateNote(Note & note,  const ChannelConfig & cfg){
		note.phaseSynth+=note.freqSynth * cfg.deltaTime;
		note.dataInvalidated=false;
	}
	void NoteUpdater::noteOn(Note & note,  ChannelConfig & cfg, s_note_id_i id, s_note_vel velocity){
		note.id=note.idSynth=static_cast<s_note_id>(id);
		note.velocity=velocity;
		note.velocitySynth=0;
		note.freqSynth=cfg.tuning->getFrequencyByID(id);
		note.phaseSynth=0;
		note.startAtTime=cfg.currentTime;
		note.passedTime=0;
		note.closedAtTime=0;
		note.forceCloseAtTime=0;
		note.closedPassedTime=0;
		note.dataInvalidated=true;
		note.noMoreData=false;
		note.lastPortamentoID=note.id;
		note.portamentoDeltaID=0;
		note.pitchBend=0;
		if(cfg.lastNote != nullptr) note.lastPortamentoID=cfg.lastNote->id;
		cfg.lastNote=&note;
		cfg.noteHoldMap[id]=true;
	}
	void NoteUpdater::noteOff(Note & note,  const ChannelConfig & cfg, s_note_vel velocity){
		note.requestClose(cfg);
	}
}