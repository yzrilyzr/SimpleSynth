#pragma once
#include "SimpleSynth.h"
#include "events/Note.h"
#include "events/NoteData.hpp"
#include "interface/NoteProcessor.h"
#include "synth/envelopers/Enveloper.h"
#include "interpolator/Interpolator.h"
#include <vector>
#include <memory>

namespace yzrilyzr_simplesynth{
	enum class MSEPointType{
		DEFAULT=0b0,
		DECAY=0b1,
		LOOP_START=0b10,
		SUSTAIN_OR_LOOP_END=0b100,
	};
	enum class MSEPointMode{
		HOLD,
		SMOOTH,
		SINGLE_CURVE,
		DOUBLE_CURVE,
		HALF_SINE,
		STAGE,
		SMOOTH_STAGE,
		PULSE,
		WAVE,
	};
	struct MSEPoint{
		float x=0;
		float y=0;
		MSEPointType type;
		MSEPointMode mode;
		float modeValue=0;
		size_t index=0;
	};
	EBCLASS(MultiStageEnvelopeKeyData){
		public:
		u_sample currentVol;
		bool isDecay;
		bool isLoop;
		bool isRelease;
		MSEPoint * start;
		MSEPoint * end;
		u_time lastTime;
		u_time releaseTime;
	};

	ECLASS(MultiStageEnvelope, public NoteProcessor, public Enveloper, NoteData<MultiStageEnvelopeKeyData>){
		public:
		MSEPoint * decayPoint=nullptr;
		MSEPoint * loopPoint=nullptr;
		MSEPoint * sustainPoint=nullptr;
		std::vector<MSEPoint> points;
		MultiStageEnvelope();
		MultiStageEnvelope(const std::vector<MSEPoint> &points);
		bool noMoreData(Note & note) override;
		NoteProcPtr clone() override;
		void init(ChannelConfig & cfg) override;
		u_sample getAmp(Note & note) override;
		std::string toString() const override;
		MultiStageEnvelopeKeyData * init(MultiStageEnvelopeKeyData * data, Note & note) override;

		private:
		u_time calcLoopTime(u_time curTime);
		void calcNextPoint(MultiStageEnvelopeKeyData & data, u_time curTime);
		public:
		static u_sample calcEnv(MSEPoint & start, MSEPoint & end, float x);
	};
}