#pragma once
#include "SimpleSynth.h"
#include "Enveloper.h"
#include "interface/NoteProcessor.h"
#include "events/Note.h"
#include "util/Util.h"
#include "interpolator/Interpolator.h"

namespace yzrilyzr_simplesynth{
	ECLASS(TimeEnvelop, public NoteProcessor), public Enveloper{
	private:
	u_sp<yzrilyzr_interpolator::Interpolator> curve;
	u_time duration;
	bool inv;
	/**
	 * @param curve
	 * @param duration (ms)
	 */
	public:
	TimeEnvelop() : TimeEnvelop(0, nullptr, false){}
	TimeEnvelop(u_time_ms duration, u_sp<yzrilyzr_interpolator::Interpolator> curve) : TimeEnvelop(duration, curve, false){}
	TimeEnvelop(u_time_ms duration, u_sp<yzrilyzr_interpolator::Interpolator> curve, bool inv){
		this->curve=curve;
		this->duration=duration / 1000.0;
		this->inv=inv;
	}
	u_sample getAmp(const Note & note)override{
		u_time x=note.passedTime / duration;
		x=yzrilyzr_util::Util::clamp01(x);
		if(inv) x=1 - x;
		return curve->y(x);
	}
	void onRegisterParam() override;
	};
}